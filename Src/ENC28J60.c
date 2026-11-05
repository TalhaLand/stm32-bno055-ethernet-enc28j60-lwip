/*
 * ENC28J60.c
 *
 *  Created on: 12 Eki 2026
 *      Author: Mansu
 */
#include "ENC28J60.h"
#include "stm32f407xx.h"


void ENC_init(enc_t *enc)
{

	HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
	HAL_Delay(150);

	clock_control(enc);
	if(enc->clock_Ready != 1)
		return;

	mac_init(enc);

	tx_rx_config(enc);

	uint8_t receive_start[2] = {0x5F, 0x04};

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, receive_start, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	enc->next_pkt_ptr = 0x1000;

}


void clock_control(enc_t *enc)
{
    uint8_t cmd = 0x1D;
    uint8_t status = 0;
    uint32_t timeout = 0;

    do {
        HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(enc->hspi, &cmd, 1, 100);
        HAL_SPI_Receive(enc->hspi, &status, 1, 100);
        HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

        timeout++;
        if (timeout > 5000) { // 1000 denemede saat sinyali gelmezse çık
            enc->clock_Ready = 0;
            return;
        }
    } while (!(status & 0x01));

    enc->clock_Ready = 1;
}


void mac_init(enc_t *enc)
{
	uint8_t data = 0x0D;
	uint8_t bank2 = 0x02;
	uint8_t gecici = 0x5f;

	/******************************************/
	//kod yazacaksan her zaman ilk 3 bit 010 olmalı sonra adreslere bakıgında 0x ya da 1x yazar ona göre ayarlarsın
	//örnegin 1f adresine veri yazacaksın 010 1 1111 oluyor bu da 0x5f yapıyor


	//Control register’a normal yazma yapacaksam komutun ilk üç biti 010 olmalı

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, &gecici, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &bank2, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	uint8_t macon1 = 0x40;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x42;
	data = 0x33;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	macon1 = 0x43;
	data = 0x40;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x44;
	data = 0x15;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x46;
	data = 0x12;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x47;
	data = 0x0C;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x4A;
	data = 0xEE;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	macon1 = 0x4B;
	data = 0x05;
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);


	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	uint8_t bank3 = 0x03;

	macon1 = 0x54;
	data = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	macon1 = 0x56;
	data = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	macon1 = 0x57;
	data = 0x01;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, &macon1, 1, 100);
	HAL_SPI_Transmit(enc->hspi, &data, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	HAL_Delay(1);

	uint8_t maddr = 0x44;
	uint8_t mac_Address_ = 0x02;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, &gecici, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &bank3, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	mac_Address_ = 0x01;
	 maddr = 0x45;

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	 maddr = 0x42;

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	 maddr = 0x43;

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	 maddr = 0x40;

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	 maddr = 0x41;

	HAL_SPI_Transmit(enc->hspi, &maddr, 1, 100);

	HAL_SPI_Transmit(enc->hspi, &mac_Address_, 1, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

}


void tx_rx_config(enc_t *enc)
{
	uint8_t data_code[2] = {0x5f , 0x04};

	/******************************/
	//bank 0 geçtik
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/***********************************/
	// rx baslangııc adress low bit
	data_code[0] = 0x48;
	data_code[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/***********************************/
	// rx baslangic adress high bit
	data_code[0] = 0x49;
	data_code[1] = 0x10;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	/***********************************/
	// rx BİTİS adress high bit
	data_code[0] = 0x4A;
	data_code[1] = 0xFF;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	// rx BİTİS adress high bit
	data_code[0] = 0x4B;
	data_code[1] = 0x1F;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	// rx isaretici adress high bit
	data_code[0] = 0x4C;
	data_code[1] = 0xFF; //0x00 değiştirk denemek için

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	// rx isaretici adress high bit
	data_code[0] = 0x4d;
	data_code[1] = 0x1F; // 0x10

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);



	uint8_t data[2] = {0x5f , 0x01};
	/******************************************************/
	//bank 1gectik

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, data, 2, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*******************************************************/
	//filtre kaldırdık

		data[0] = 0x58;
		data[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, data, 2, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
		//bank 0 geçtik
		HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
		HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


}

void transmit_message(enc_t *enc , uint8_t *str , uint16_t len)
{
	uint8_t data_code[2] = {0x5f , 0x04 };

	/******************************/
	//bank 0 geçtik
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


	/************************************/
	//EWRPT ayarladık bu kalemi satır basına koymak gibi bir şey

	data_code[0] = 0x42;
	data_code[1] = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


	data_code[0] = 0x43;
	data_code[1] = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
		//ETXST low bit ayarladık

	data_code[0] = 0x44;
	data_code[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
	//ETXST high bit ayarladık
	data_code[0] = 0x45;
	data_code[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
	//ram tarafı

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	data_code[0] = 0x7A;
	data_code[1] = 0x00;
	HAL_SPI_Transmit(enc->hspi, data_code, 2, 100);

	//B0-25-AA-57-CA-D9benim bilgisayarın mac adresi
	uint8_t my_pc_mac_address[6] 	= {0xB0 , 0x25 , 0xAA , 0x57 , 0xCA , 0xD9};
	uint8_t stm_mac[6] 				= {0x02 , 0x01 , 0x01 , 0x01 , 0x01 , 0x01};

	data_code[0] = 0x08;
	data_code[1] = 0x00;

	HAL_SPI_Transmit(enc->hspi, my_pc_mac_address, sizeof(my_pc_mac_address), 100);
	HAL_SPI_Transmit(enc->hspi, stm_mac, sizeof(stm_mac), 100);
	HAL_SPI_Transmit(enc->hspi, data_code, 2, 100);

	HAL_SPI_Transmit(enc->hspi, str, len, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*****************************************************/
	// send message kısmı ram'a bildiriyoruz mesaj bitti gönder diye
	//Paketin Sonu bildirdik
	data_code[0] = 0x46;
	data_code[1] = (len) & 0xFF;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	data_code[0] = 0x47;
	data_code[1] = ((len) >> 8);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*****************************************************/
	// iletim tetiğini verdik

	data_code[0] = 0x9F;
	data_code[1] = 0x08;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
// ileride buraya ECON1.TXRTS pini 0 olana kadar beklemelik while koyucaz sonra cs pini high uapıcaz
	HAL_Delay(3);





}




void receive_message(enc_t *enc)
{
	enc->rx_len = 0;
	uint8_t data[2] = {0x5f , 0x05};
	/******************************************************/
	//bank 1gectik

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, data, 2, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


	/**************************************************/
	// EPKTCNT okuduk

	data[0] = (0x00 | 0x19);
	data[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(enc->hspi, &data[0], enc->receive_data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	if(enc->receive_data[1] == 0)
		return;

	/********************************************************/
	// bank 0 döndük
	data[0] = 0x5F;
	data[1] = 0x04;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(enc->hspi, data, 2, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	/**********************************************/
	// rx pointer ayarlıyoruz

	data[0] = 0x40;
	data[1] = enc->next_pkt_ptr & 0xFF;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	data[0] = 0x41;
	data[1] =(enc->next_pkt_ptr >> 8) & 0xFF;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*******************************************************/
	// RAMDAN VERİ OKİCAZ

	data[0] = 0x3A;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, &data[0], 1, 100);
	HAL_SPI_Receive(enc->hspi, enc->rx_header , 6 , 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	//enc->rx_header[0] ve enc->rx_header[1]: Next Packet Pointer (Sonraki paketin başlama adresi)

	//enc->rx_header[2] ve enc->rx_header[3]: Paket Uzunluğu (len)

	enc->next_pkt_ptr = enc->rx_header[0] | (enc->rx_header[1] << 8);

	uint16_t len = enc->rx_header[2] | (enc->rx_header[3] << 8);
	len -= 4;


	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, &data[0], 1, 100);

	HAL_SPI_Receive(enc->hspi, enc->rx_buffer , len , 100);
	enc->rx_len = len;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
/*


	data[0] = 0x4C;
	data[1] = enc->rx_header[0];

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	data[0] = 0x4D;
	data[1] = enc->rx_header[1];



	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	data[0] = 0x9E;
	data[1] = 0x40;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);*/

	///*****///
	// asagıyı silip yorum satırını ac
	uint16_t rx_read_ptr;

	if (enc->next_pkt_ptr == 0x1000)
	    rx_read_ptr = 0x1FFF;
	else
	    rx_read_ptr = enc->next_pkt_ptr - 1;

	// Düşük byte
	data[0] = 0x4C;
	data[1] = rx_read_ptr & 0xFF;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	// Yüksek byte
	data[0] = 0x4D;
	data[1] = (rx_read_ptr >> 8) & 0xFF;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	// Paket sayacını azalt
	data[0] = 0x9E;
	data[1] = 0x40;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data, 2, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);



}



void transmit_frame(enc_t *enc, uint8_t *frame, uint16_t len)
{
	uint8_t data_code[2] = {0x5f , 0x04 };

	/******************************/
	//bank 0 geçtik
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


	/************************************/
	//EWRPT ayarladık bu kalemi satır basına koymak gibi bir şey

	data_code[0] = 0x42;
	data_code[1] = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);


	data_code[0] = 0x43;
	data_code[1] = 0x00;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
		//ETXST low bit ayarladık

	data_code[0] = 0x44;
	data_code[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/******************************/
	//ETXST high bit ayarladık
	data_code[0] = 0x45;
	data_code[1] = 0x00;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	data_code[0] = 0x7A;
	data_code[1] = 0x00;
	HAL_SPI_Transmit(enc->hspi, data_code, 2, 100);

	HAL_SPI_Transmit(enc->hspi, frame, len, 100);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*****************************************************/
	// send message kısmı ram'a bildiriyoruz mesaj bitti gönder diye
	//Paketin Sonu bildirdik
	data_code[0] = 0x46;
	data_code[1] = (len) & 0xFF;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	data_code[0] = 0x47;
	data_code[1] = ((len) >> 8);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	/*****************************************************/
	// iletim tetiğini verdik

	data_code[0] = 0x9F;
	data_code[1] = 0x08;

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(enc->hspi, data_code, sizeof(data_code), 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
// ileride buraya ECON1.TXRTS pini 0 olana kadar beklemelik while koyucaz sonra cs pini high uapıcaz
	HAL_Delay(3);





}

