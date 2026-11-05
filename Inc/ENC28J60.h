/*
 * ENC28J60.h
 *
 *  Created on: 12 Eki 2026
 *      Author: Mansu
 */

#ifndef INC_ENC28J60_H_
#define INC_ENC28J60_H_

#include "main.h"



typedef struct
{
	SPI_HandleTypeDef *hspi;

	uint8_t test_rx[2];
	uint8_t test_tx[2];

	uint8_t receive_data[2];
	uint8_t rx_header[6];
	uint16_t next_pkt_ptr;
	uint8_t rx_buffer[1536];
	uint16_t rx_len;

	uint8_t clock_Ready;

}enc_t;


void ENC_init(enc_t *enc);

void clock_control(enc_t *enc);

void mac_init(enc_t *enc);

void tx_rx_config(enc_t *enc);

void transmit_message(enc_t *enc , uint8_t *str , uint16_t len);

void receive_message(enc_t *enc);

void transmit_frame(enc_t *enc, uint8_t *frame, uint16_t len);

#endif /* INC_ENC28J60_H_ */
