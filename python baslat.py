from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import math
import socket
import threading
import time
import webbrowser

TCP_HOST = "192.168.1.20"
TCP_PORT = 5000

HTTP_HOST = "127.0.0.1"
HTTP_PORT = 5000

# Quaternion sırası: w, x, y, z
durum = {
    "q": (1.0, 0.0, 0.0, 0.0),
    "bagli": False,
    "son_veri": None,
}
durum_kilidi = threading.Lock()


def tcp_dinleyici_thread():
    while True:
        try:
            print(f"STM32 bağlantısı deneniyor: {TCP_HOST}:{TCP_PORT}")

            with socket.create_connection(
                (TCP_HOST, TCP_PORT), timeout=3.0
            ) as s:
                s.settimeout(3.0)

                with durum_kilidi:
                    durum["bagli"] = True
                    durum["son_veri"] = None

                print("STM32 TCP bağlantısı kuruldu.")

                buffer = b""

                while True:
                    data = s.recv(4096)

                    if not data:
                        raise ConnectionError("STM32 bağlantıyı kapattı.")

                    buffer += data

                    while b"\n" in buffer:
                        line, buffer = buffer.split(b"\n", 1)

                        try:
                            text = line.decode("ascii").strip()

                            # Mevcut STM32 kodu bağlantı başında
                            # satır sonu olmadan "talha" gönderiyor.
                            if text.startswith("talha"):
                                text = text[5:].strip()

                            alanlar = text.split(",")

                            # Eski roll,pitch,yaw satırlarını kabul etme.
                            if len(alanlar) != 4:
                                continue

                            q = tuple(float(x) for x in alanlar)

                            if not all(math.isfinite(x) for x in q):
                                continue

                            norm = math.hypot(*q)

                            if norm < 1e-9:
                                continue

                            q = tuple(x / norm for x in q)

                        except (ValueError, UnicodeError):
                            continue

                        # Dört bileşeni birlikte güncelle.
                        with durum_kilidi:
                            durum["q"] = q
                            durum["son_veri"] = time.monotonic()

                    if len(buffer) > 8192:
                        raise ValueError(
                            "TCP verisinde satır sonu bulunamadı."
                        )

        except (OSError, ConnectionError, ValueError) as hata:
            print(f"TCP: {hata}")

        finally:
            with durum_kilidi:
                durum["bagli"] = False

        time.sleep(1.0)


HTML_EKRANI = r"""
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>BNO055 Quaternion Uçak</title>

    <style>
        body {
            margin: 0;
            background: #0b0f19;
            color: #fff;
            font-family: monospace;
            overflow: hidden;
        }

        canvas {
            display: block;
        }

        #bilgi {
            position: absolute;
            top: 16px;
            left: 16px;
            background: rgba(15, 23, 42, 0.9);
            padding: 16px 20px;
            border-radius: 10px;
            font-size: 14px;
            line-height: 1.6;
            border: 1px solid #38bdf8;
            z-index: 10;
            min-width: 245px;
        }

        #baglanti {
            color: #fbbf24;
            margin-bottom: 8px;
        }

        .btn {
            margin-top: 10px;
            background: #38bdf8;
            border: none;
            padding: 8px 14px;
            border-radius: 6px;
            cursor: pointer;
            font-weight: bold;
            color: #0b0f19;
        }

        .btn:hover {
            background: #0284c7;
            color: #fff;
        }

        .btn:disabled {
            opacity: 0.4;
            cursor: default;
        }

        #not {
            max-width: 260px;
            margin-top: 8px;
            font-size: 12px;
            color: #94a3b8;
        }
    </style>
</head>

<body>
    <div id="bilgi">
        <b>BNO055 · MADGWICK</b>
        <div id="baglanti">Bağlantı bekleniyor...</div>

        <div>Q0 (W): <span id="qw">—</span></div>
        <div>Q1 (X): <span id="qx">—</span></div>
        <div>Q2 (Y): <span id="qy">—</span></div>
        <div>Q3 (Z): <span id="qz">—</span></div>

        <button id="sifirla" class="btn" disabled>
            Mevcut Duruşu Sıfırla
        </button>

        <div id="not">
            Sıfırlama, mevcut eğim ve yönü referans alır.
        </div>
    </div>

    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>

    <script>
        const baglanti = document.getElementById("baglanti");
        const sifirlaButonu = document.getElementById("sifirla");

        if (typeof THREE === "undefined") {
            baglanti.textContent =
                "Three.js yüklenemedi. İnternet bağlantısını kontrol et.";
            throw new Error("Three.js yüklenemedi.");
        }

        const scene = new THREE.Scene();
        scene.background = new THREE.Color(0x0b0f19);

        const camera = new THREE.PerspectiveCamera(
            55,
            window.innerWidth / window.innerHeight,
            0.1,
            1000
        );

        const renderer = new THREE.WebGLRenderer({
            antialias: true
        });

        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        renderer.setSize(window.innerWidth, window.innerHeight);
        document.body.appendChild(renderer.domElement);

        const grid = new THREE.GridHelper(
            30, 30, 0x38bdf8, 0x1e293b
        );
        grid.position.y = -3;
        scene.add(grid);

        scene.add(new THREE.AmbientLight(0xffffff, 0.8));

        const dirLight = new THREE.DirectionalLight(0xffffff, 1.2);
        dirLight.position.set(10, 15, 10);
        scene.add(dirLight);

        // Uçak modeli: burun -Z, üst +Y, sağ +X.
        const ucak = new THREE.Group();

        const matGovde = new THREE.MeshStandardMaterial({
            color: 0xdbeafe,
            roughness: 0.3,
            metalness: 0.2
        });

        const matKanat = new THREE.MeshStandardMaterial({
            color: 0x00d2ff,
            roughness: 0.4
        });

        const matKokpit = new THREE.MeshStandardMaterial({
            color: 0x0f172a,
            roughness: 0.1,
            metalness: 0.9
        });

        const matKuyruk = new THREE.MeshStandardMaterial({
            color: 0xef4444
        });

        const govde = new THREE.Mesh(
            new THREE.CylinderGeometry(0.3, 0.3, 4.0, 32),
            matGovde
        );
        govde.rotation.x = Math.PI / 2;
        ucak.add(govde);

        const burun = new THREE.Mesh(
            new THREE.ConeGeometry(0.3, 1.4, 32),
            matGovde
        );
        burun.rotation.x = -Math.PI / 2;
        burun.position.z = -2.7;
        ucak.add(burun);

        const kokpitGeo = new THREE.SphereGeometry(0.24, 16, 16);
        kokpitGeo.scale(0.8, 0.9, 2.2);

        const kokpit = new THREE.Mesh(kokpitGeo, matKokpit);
        kokpit.position.set(0, 0.26, -0.8);
        ucak.add(kokpit);

        const anaKanat = new THREE.Mesh(
            new THREE.BoxGeometry(5.0, 0.06, 1.3),
            matKanat
        );
        anaKanat.position.set(0, 0, -0.2);
        ucak.add(anaKanat);

        const wingletGeo = new THREE.BoxGeometry(0.06, 0.5, 0.7);

        const solWinglet = new THREE.Mesh(wingletGeo, matKuyruk);
        solWinglet.position.set(-2.5, 0.25, -0.2);

        const sagWinglet = new THREE.Mesh(wingletGeo, matKuyruk);
        sagWinglet.position.set(2.5, 0.25, -0.2);

        ucak.add(solWinglet, sagWinglet);

        const arkaKanat = new THREE.Mesh(
            new THREE.BoxGeometry(1.8, 0.05, 0.6),
            matKanat
        );
        arkaKanat.position.set(0, 0.05, 1.6);
        ucak.add(arkaKanat);

        const dikeyKuyruk = new THREE.Mesh(
            new THREE.BoxGeometry(0.06, 0.9, 0.7),
            matKuyruk
        );
        dikeyKuyruk.position.set(0, 0.55, 1.6);
        ucak.add(dikeyKuyruk);

        scene.add(ucak);

        camera.position.set(0, 2.8, 6.5);
        camera.lookAt(0, 0, 0);

        const hamQuat = new THREE.Quaternion();
        const hedefQuat = new THREE.Quaternion();
        const cariQuat = new THREE.Quaternion();
        const sifirQuat = new THREE.Quaternion();

        let ilkVeriAlindi = false;
        let veriGecerli = false;

        function hedefiHesapla() {
            hedefQuat
                .multiplyQuaternions(sifirQuat, hamQuat)
                .normalize();
        }

        sifirlaButonu.addEventListener("click", () => {
            if (!veriGecerli) return;

            // Her seferinde sıfırlanmamış yönelimi referans al.
            sifirQuat.copy(hamQuat).invert();
            hedefiHesapla();

            cariQuat.copy(hedefQuat);
            ucak.quaternion.copy(cariQuat);
        });

        async function veriGuncelle() {
            const controller = new AbortController();
            const timeout = setTimeout(() => controller.abort(), 1500);

            try {
                const res = await fetch("/veri", {
                    cache: "no-store",
                    signal: controller.signal
                });

                if (!res.ok) throw new Error("HTTP hatası");

                const d = await res.json();

                veriGecerli = false;
                sifirlaButonu.disabled = true;

                if (!d.ok) {
                    baglanti.textContent = d.connected
                        ? "TCP bağlı; güncel quaternion bekleniyor..."
                        : "STM32 bağlantısı bekleniyor...";

                    baglanti.style.color = "#fbbf24";
                    return;
                }

                const values = [d.qw, d.qx, d.qy, d.qz];

                if (!values.every(Number.isFinite)) {
                    throw new Error("Geçersiz quaternion");
                }

                /*
                 * Önceki kodundaki eksen eşlemesi korunuyor.
                 * Gerçek kart/model uyumu hareketle doğrulanmalı.
                 */
                hamQuat.set(-d.qy, d.qz, -d.qx, d.qw);

                if (hamQuat.lengthSq() < 1e-12) {
                    throw new Error("Sıfır quaternion");
                }

                hamQuat.normalize();
                hedefiHesapla();

                if (!ilkVeriAlindi) {
                    cariQuat.copy(hedefQuat);
                    ilkVeriAlindi = true;
                }

                veriGecerli = true;
                sifirlaButonu.disabled = false;

                baglanti.textContent = "Bağlı · quaternion akışı";
                baglanti.style.color = "#4ade80";

                document.getElementById("qw").textContent =
                    d.qw.toFixed(4);
                document.getElementById("qx").textContent =
                    d.qx.toFixed(4);
                document.getElementById("qy").textContent =
                    d.qy.toFixed(4);
                document.getElementById("qz").textContent =
                    d.qz.toFixed(4);

            } catch (e) {
                veriGecerli = false;
                sifirlaButonu.disabled = true;

                baglanti.textContent =
                    "Veri alınamıyor; son duruş korunuyor.";
                baglanti.style.color = "#f87171";

            } finally {
                clearTimeout(timeout);

                // Önceki istek bitmeden yeni istek başlatma.
                setTimeout(veriGuncelle, 20);
            }
        }

        veriGuncelle();

        let oncekiKare = performance.now();

        function animate(simdi) {
            requestAnimationFrame(animate);

            const dt = Math.min((simdi - oncekiKare) / 1000, 0.1);
            oncekiKare = simdi;

            if (ilkVeriAlindi) {
                // Kare hızından bağımsız yumuşatma.
                const agirlik = 1 - Math.exp(-15 * dt);

                cariQuat.slerp(hedefQuat, agirlik).normalize();
                ucak.quaternion.copy(cariQuat);
            }

            renderer.render(scene, camera);
        }

        requestAnimationFrame(animate);

        window.addEventListener("resize", () => {
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();

            renderer.setSize(window.innerWidth, window.innerHeight);
        });
    </script>
</body>
</html>
"""


class Sunucu(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass

    def cevap_gonder(self, kod, icerik_turu, veri):
        self.send_response(kod)
        self.send_header("Content-Type", icerik_turu)
        self.send_header("Content-Length", str(len(veri)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()

        try:
            self.wfile.write(veri)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_GET(self):
        yol = self.path.split("?", 1)[0]

        if yol == "/":
            self.cevap_gonder(
                200,
                "text/html; charset=utf-8",
                HTML_EKRANI.encode("utf-8"),
            )

        elif yol == "/veri":
            with durum_kilidi:
                qw, qx, qy, qz = durum["q"]
                bagli = durum["bagli"]
                son_veri = durum["son_veri"]

            yas = (
                time.monotonic() - son_veri
                if son_veri is not None
                else None
            )

            cevap = {
                "ok": bagli and yas is not None and yas < 2.0,
                "connected": bagli,
                "qw": qw,
                "qx": qx,
                "qy": qy,
                "qz": qz,
            }

            self.cevap_gonder(
                200,
                "application/json; charset=utf-8",
                json.dumps(cevap, allow_nan=False).encode("utf-8"),
            )

        else:
            self.cevap_gonder(
                404,
                "text/plain; charset=utf-8",
                "Bulunamadı".encode("utf-8"),
            )


if __name__ == "__main__":
    sunucu = ThreadingHTTPServer(
        (HTTP_HOST, HTTP_PORT), Sunucu
    )
    sunucu.daemon_threads = True

    threading.Thread(
        target=tcp_dinleyici_thread,
        daemon=True,
    ).start()

    adres = f"http://{HTTP_HOST}:{HTTP_PORT}"
    print(f"Web sunucusu: {adres}")

    # Sunucu hizmet vermeye başladıktan kısa süre sonra aç.
    acilis = threading.Timer(0.5, lambda: webbrowser.open(adres))
    acilis.daemon = True
    acilis.start()

    try:
        sunucu.serve_forever()
    except KeyboardInterrupt:
        print("\nSunucu kapatıldı.")
    finally:
        sunucu.server_close()