/*
 * TCP_Talha.h
 *
 *  Created on: 14 Eki 2026
 *      Author: Mansu
 */

#ifndef INC_TCP_TALHA_H_
#define INC_TCP_TALHA_H_

#include "main.h"
#include "lwip.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"



void tcp_server_init(void);

err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

extern struct tcp_pcb *active_client_pcb; // Aktif bağlantıyı dışarı açıyoruz
void tcp_send_sensor_data(struct tcp_pcb *tpcb, float r, float p, float y);

void tcp_send_quaternion_data(struct tcp_pcb *tpcb, float q0, float q1, float q2, float q3);
#endif /* INC_TCP_TALHA_H_ */
