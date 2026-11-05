# STM32F407 ile Ethernet Üzerinden Yönelim Takibi

Bu projede BNO055 sensöründen aldığımız hareket verilerini STM32F407VG üzerinde işleyip Ethernet üzerinden bilgisayara aktarıyoruz. Kartı elinizde eğdiğinizde veya döndürdüğünüzde tarayıcıdaki 3B uçak modeli de aynı hareketi takip ediyor.

Projenin iki ayrı arayüzü var. **STM32’nin sunduğu web paneli** sensör açılarını izlemeye ve manyetometre kalibrasyonunu başlatmaya yarıyor. **Bilgisayardaki Python uygulaması** ise karttan gelen yönelim verisini alıp 3B uçak modeli üzerinde gösteriyor.

![BNO055 verileriyle yönlendirilen 3B uçak modeli ve quaternion değerleri](./simülation.png)

## Sistem nasıl çalışıyor?

```text
BNO055 ── I²C ──► STM32F407VG ── SPI ──► ENC28J60 ── Ethernet ──► Bilgisayar
                  Ölçüm ve filtreleme                              │
                  Madgwick + lwIP                                  ├─ Web paneli
                                                                   └─ Python → 3B uçak
```

1. STM32, **AMG modunda** çalışan BNO055’in ivmeölçer, manyetometre ve jiroskop verilerini yaklaşık **20 ms** aralıklarla okur.
2. İvme ve manyetik alan ölçümleri filtrelenir. Kart sabitken alınan jiroskop ölçümleriyle eksenlerdeki kayma tahmin edilir.
3. STM32 üzerinde iki hesap yapılır. Eğim ve pusula hesabından **roll, pitch ve yaw** değerleri elde edilir. **Madgwick filtresi** ise aynı sensör verilerinden kartın üç boyutlu yönelimini `w, x, y, z` sırasıyla dört sayıyla ifade eder. Bu gösterime *quaternion* denir.
4. Web paneli açıları kartın HTTP sunucusundan alır. Python uygulaması ise ayrı bir TCP bağlantısı üzerinden quaternion değerlerini okur ve uçağın duruşunu günceller.

## Donanım ve pin bağlantıları

![STM32F407VG için CubeMX pin bağlantıları](./baglantilar.png)

| Modül | Modül pini | STM32F407VG pini | CubeMX ayarı |
|---|---|---|---|
| BNO055 | SCL | PB8 | `I2C1_SCL` |
| BNO055 | SDA | PB7 | `I2C1_SDA` |
| BNO055 | RST | PC7 | GPIO çıkışı, `BNO_Reset` |
| ENC28J60 | SCK | PC10 | `SPI3_SCK` |
| ENC28J60 | SO / MISO | PC11 | `SPI3_MISO` |
| ENC28J60 | SI / MOSI | PC12 | `SPI3_MOSI` |
| ENC28J60 | CS | PC9 | GPIO çıkışı, `CS` |
| ENC28J60 | RESET | PC8 | GPIO çıkışı, `Reset` |

STM32 ile iki modülün GND hatları ortaklanmalıdır. Ethernet kablosu ENC28J60 modülünün RJ45 girişine takılır. CubeMX görselinde mikrodenetleyicinin dahili Ethernet birimine ait `ETH_*` pinleri de görülür. Bu uygulamada harici ENC28J60 ile veri alışverişi **SPI3** üzerinden yapılır.

## BNO055 sürücüsü ve yönelim hesabı

`Src/BNO055.c`, sensörün başlatılmasını, ölçümlerin okunmasını ve kalibrasyon işlemlerini yönetir. BNO055’i **AMG modunda** kullanmamızın amacı, üç sensörün ölçümlerine erişip yönelim hesabını STM32 üzerinde yapmaktır.

- **Jiroskop kayması:** Kart sabitken bile jiroskop küçük bir açısal hız ölçebilir. Bu değer zamanla toplandığında açı yavaşça değişir. Sürücü, kartın sabit kaldığı ölçüm aralıklarında her eksenin ofsetini tahmin eder ve sonraki okumalardan çıkarır.
- **Manyetometre kalibrasyonu:** Sabit manyetik etkiler ölçümlerin merkezini kaydırabilir (*hard iron*). Kartın çevresindeki malzemeler de eksenlerin ölçüm aralıklarını farklı oranlarda değiştirebilir (*soft iron*). Kalibrasyonda kart farklı yönlere çevrilerek her eksenin alt ve üst değerleri toplanır. Bunlardan eksen başına ofset ve ölçek düzeltmesi hesaplanır.
- **Eğim telafili pusula:** Kart eğildiğinde manyetik alanın X ve Y eksenlerindeki bileşenleri değişir. Yazılım, yatış ve yunuslama açılarını kullanarak alanı yatay düzleme yansıtır ve pusula yönünü hesaplar.
- **Madgwick filtresi:** Düzeltilmiş jiroskop verisini, filtrelenmiş ivme ve manyetik alan ölçümleriyle birleştirir. `Src/MadgwickAHRS.c` içindeki bu hesap, Python uygulamasına gönderilen dört quaternion bileşenini üretir.

## ENC28J60, Ethernet ve lwIP

**ENC28J60**, STM32’nin SPI üzerinden kontrol ettiği Ethernet denetleyicisidir. Ethernet kablosundan gelen çerçeveleri kendi alım belleğinde tutar. STM32 gönderim yapacağında da çerçeveyi SPI üzerinden modülün iletim belleğine yazar.

**lwIP**, STM32 üzerinde çalışan TCP/IP yazılımıdır. IP adresleme, TCP bağlantıları ve HTTP istekleri bu katmanda işlenir. ENC28J60 sürücüsü Ethernet çerçevelerini taşır. `ethernetif.c` ise sürücü ile lwIP’nin kullandığı veri yapıları arasında bağlantı kurar.

| Dosya | Görevi |
|---|---|
| `Src/ENC28J60.c` ve `Inc/ENC28J60.h` | ENC28J60’ı başlatır, MAC ve tampon ayarlarını yapar. `receive_message()` gelen Ethernet çerçevesini okur, `transmit_frame()` giden çerçeveyi modüle aktarır. |
| `LWIP/Target/ethernetif.c` | `low_level_input()` ile alınan çerçeveyi lwIP’nin `pbuf` tamponuna taşır. `low_level_output()` ile lwIP’nin hazırladığı veriyi ENC28J60 sürücüsüne verir. |
| `LWIP/App/lwip.c` | Kartın IP adresini ve ağ maskesini tanımlar. `netif_add()` ile ağ arayüzünü kaydeder. `MX_LWIP_Process()` ile gelen paketleri ve ağ zamanlayıcılarını işler. |
| `LWIP/Target/lwipopts.h` | lwIP yapılandırmasını içerir. Gömülü HTTP sunucusu, SSI ve CGI desteği burada etkinleştirilir. |
| `Src/TCP_Talha.c` | `5000` numaralı TCP portunu dinler. Bağlanan istemciye quaternion değerlerini virgülle ayrılmış ve satır sonuyla biten metin olarak gönderir. |
| `Src/http_ssi.c` | Sensör ve kalibrasyon değerlerini web sayfasına yerleştirir. Tarayıcıdan gelen kalibrasyon isteğini karşılar. |

## Kendi STM32CubeIDE projenize entegrasyon

Depodaki uygulama kaynakları `Src/` ve `Inc/` altında bulunur. Aşağıdaki adımlarda bu dosyalar CubeIDE projesinin `Core/Src/` ve `Core/Inc/` klasörlerine eklenir. Başlatma sırası için depodaki `Src/main.c` dosyası referans alınabilir.

### 1. Çevre birimlerini yapılandırın

CubeMX’te **I2C1** birimini etkinleştirip saat hızını örnekteki gibi **50 kHz** yapın. **SPI3** birimini master olarak yapılandırın. Pinleri bağlantı tablosuna göre seçin. `CS`, `Reset` ve `BNO_Reset` etiketlerini sürücüdeki adlarla eşleştirin.

### 2. Sensör ve Ethernet sürücülerini ekleyin

`BNO055.c`, `MadgwickAHRS.c` ve `ENC28J60.c` dosyalarını `Core/Src/` altına, aynı adlı başlık dosyalarını `Core/Inc/` altına kopyalayın. SPI ve I²C tanıtıcılarını örnekteki gibi bağlayın:

```c
enc.hspi = &hspi3;
sensor_info.i2c = &hi2c1;
```

GPIO ve SPI hazır olduktan sonra `ENC_init(&enc)` çağrılır. I²C hazır olduğunda `BNO055_Init()` sensörü yapılandırır. BNO055’in varsayılan 7 bit adresi `0x28`’dir. Modülünüz `0x29` adresindeyse `BNO055.h` içindeki `BNO055_I2C_address` tanımını `(0x29 << 1)` olarak güncelleyin.

### 3. lwIP’yi ENC28J60’a bağlayın

Hedef projeye lwIP ve HTTP sunucusu kaynaklarını dahil edin. Ağ arayüzünü taşırken depodaki **`LWIP/App/` ve `LWIP/Target/` klasörlerini** esas alın. Kök dizindeki aynı adlı dosyaları ikinci kez derlemeye eklemek yinelenen fonksiyon tanımlarına yol açar.

`MX_LWIP_Init()` içinde ENC28J60 nesnesi `&enc`, `netif_add()` çağrısına geçirilir. Böylece `ethernetif.c`, gönderme ve alma sırasında hangi sürücüyle çalışacağını bilir. `TCP_Talha.c/.h` ve `http_ssi.c/.h` dosyalarını da ekleyip ağ hazır olduktan sonra `tcp_server_init()` ve `http_server_init()` fonksiyonlarını çağırın.

Ana döngüde **`MX_LWIP_Process()` her turda**, sensör okuma ve filtreleme ise **20 ms aralıklarla** çalışır. Ağ işlemini sensör zamanlayıcısının dışında tutmak, iki ölçüm arasında gelen TCP ve HTTP isteklerinin de işlenmesini sağlar.

## IP adresi nasıl ayarlanır?

Örnek yapılandırmada kartın sabit IP adresi **`192.168.1.20`**, ağ maskesi **`255.255.255.0`** olarak ayarlanmıştır. Bilgisayarın Ethernet arayüzü aynı alt ağda ve farklı bir adreste olmalıdır. Doğrudan kablo bağlantısında, adres boşsa bilgisayar için `192.168.1.10` kullanılabilir.

Kartın IP adresini değiştirirken şu üç ayarı birlikte güncelleyin:

| Dosya | Güncellenecek alan |
|---|---|
| `LWIP/App/lwip.c` | `MX_LWIP_Init()` içindeki `IP_ADDRESS`. Ağınıza göre `NETMASK_ADDRESS` ve `GATEWAY_ADDRESS` değerlerini de ayarlayın. |
| `Src/TCP_Talha.c` | `tcp_server_init()` içindeki `IP_ADDR4()` çağrısı. TCP sunucusu burada verilen yerel adrese bağlanır. |
| `python baslat.py` | `TCP_HOST`. Python istemcisi kartı bu adreste arar. |

TCP portu değiştirilirse `tcp_bind()` içindeki port ile Python’daki `TCP_PORT` aynı olmalıdır. Python dosyasındaki `HTTP_HOST = "127.0.0.1"` ise bilgisayardaki 3B görüntüleyiciye aittir.

## Gömülü web paneli ve kalibrasyon

![STM32 web panelindeki açı alanları, dört LED göstergesi ve kalibrasyon düğmesi](./site.png)

STM32, ana sayfayı kendi flash belleğinden sunar. Tarayıcı, roll, pitch ve yaw alanlarını güncellemek için `/data.shtml` kaynağını okur. **SSI**, sayfadaki alanların firmware içindeki güncel değerlerle doldurulmasını sağlar. **CGI** ise tarayıcıdan gelen bir isteği kart üzerinde bir işleme bağlar.

Örneğin **Soft / Hard Iron Kalibrasyonunu Başlat** düğmesi `/calibrate.cgi?start=1` isteğini gönderir. Firmware kalibrasyon moduna geçer ve 1500 örnek toplar. Bu sırada sensörü farklı yönlere çevirmek, üç eksenin ölçüm aralığını belirlemek için gereklidir. Kalibrasyon tamamlandığında kart yeni ofsetleri kullanarak yönelim hesabına devam eder.

### PWM ve LED göstergeleri

Panel tasarımındaki dört renkli gösterge, CubeMX görselindeki **TIM4** kanallarıyla eşleşir:

| Kanal | Pin | LED |
|---|---|---|
| TIM4_CH1 | PD12 | Yeşil |
| TIM4_CH2 | PD13 | Turuncu |
| TIM4_CH3 | PD14 | Kırmızı |
| TIM4_CH4 | PD15 | Mavi |

**PWM**, bir çıkışı sabit periyotlarla açıp kapatarak sinyalin aktif kaldığı süreyi ayarlar. Örneğin aktif-yüksek bağlı bir LED’de %25 görev oranı, çıkışın her periyodun dörtte birinde aktif kalması anlamına gelir. Görev oranı yükseldikçe LED’in ortalama parlaklığı artar.

Ekran görüntüsünde panel veri beklediği için göstergelerde `--` görülüyor. Paylaşılan `Src/main.c` sürümünde fiziksel PWM çıkışlarını başlatan çağrılar bulunmuyor. Bu kanalları kart üzerinde kullanmak için CubeMX’te **TIM4 → PWM Generation CH1–CH4** seçilir. Timer başlatıldıktan sonra her kanal `HAL_TIM_PWM_Start()` ile etkinleştirilir ve parlaklık ilgili kanalın karşılaştırma değeriyle ayarlanır.

## `fsdata.c` nedir, nasıl oluşturulur?

STM32 üzerindeki HTTP sunucusunun HTML dosyalarına erişebilmesi için sayfalar firmware’in içine gömülür. **`http/fs/`**, düzenlenebilir web dosyalarının bulunduğu klasördür. **`http/fsdata.c`** ise bu klasörün içeriğinden üretilen C dosyasıdır. Sayfa yollarını ve dosya içeriklerini flash bellekte tutulabilecek veri dizileri olarak içerir.

Örneğin tarayıcı `/index.html` istediğinde HTTP sunucusu bu yolu gömülü dosya tablosunda bulur ve karşılık gelen baytları Ethernet üzerinden gönderir. `.shtml` sayfalarındaki dinamik alanlar da istek işlendiği sırada SSI tarafından doldurulur.

**Web sayfasını değiştirdikten sonra:**

1. Ana sayfa için `http/fs/index.html`, veri alanları için `http/fs/data.shtml`, kalibrasyon ekranı için `http/fs/calibration.shtml` dosyasını düzenleyin.
2. Windows’ta terminali **`http/` klasöründe** açın. Bu dizinde hem `makeFSdata.exe` hem de onun okuyacağı `fs/` klasörü bulunur.
3. Üretim aracını çalıştırın:

   ```powershell
   .\makeFSdata.exe
   ```

4. Üretilen `fsdata.c` dosyasını hedef projenizin lwIP HTTP sunucusunun kullandığı dizine alın. CubeIDE projelerinde bu genellikle `Middlewares/Third_Party/LwIP/src/apps/http/` yoludur. `fs.c`, üretilen içeriği `HTTPD_FSDATA_FILE` üzerinden dahil eder.
5. Firmware’i yeniden derleyip karta yükleyin. Böylece kart, flash belleğe eklenen yeni sayfayı sunar.

## Python ile 3B uçak görüntüleyicisi

`python baslat.py`, kartın TCP sunucusuna bağlanır ve her satırda gelen `w,x,y,z` değerlerini okur. Geçerli quaternion değerlerini normalize ederek tarayıcıdaki Three.js uygulamasına aktarır. Uçak modeli iki ölçüm arasındaki dönüşü yumuşatarak gösterir.

Görüntüleyiciyi kullanmak için kart ağdayken, Python dosyasının bulunduğu klasörde:

```bash
python "python baslat.py"
```

| Adres | Ne için kullanılır? |
|---|---|
| `http://192.168.1.20/` | STM32’nin Ethernet üzerinden sunduğu sensör ve kalibrasyon paneli |
| `192.168.1.20:5000` | Python’un karttan quaternion verisi aldığı TCP bağlantısı |
| `http://127.0.0.1:5000/` | Python betiği çalıştığı sürece bilgisayarda sunulan 3B uçak ekranı |

Model güncel veri aldığında panelde **Bağlı · quaternion akışı** yazar. **Mevcut Duruşu Sıfırla** düğmesi, o anki yönelimi görsel referans olarak kaydeder. Python tarafı standart kütüphaneyi kullanır. Tarayıcı, Three.js kütüphanesini internet üzerinden yükler.
