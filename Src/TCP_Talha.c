/*
 * TCP_Talha.c
 *
 *  Created on: 14 Eki 2026
 *      Author: Mansu
 */

#include "TCP_Talha.h"
extern uint8_t message[5];
void tcp_send_message(struct tcp_pcb * 	pcb, const void * 	arg, u16_t len, u8_t apiflags );

void deneme()
{

}
void tcp_server_init(void)
{
    /* YENİ BİR TCP OLURSTUR BU PCB = PROGRAM CONTROL BAND SANIRIM */
    struct tcp_pcb *tpcb;
    tpcb = tcp_new();

    err_t err;

    /* 2.ADIM ADRESS AYARLADIK */
    ip_addr_t myIPADDR;

    IP_ADDR4(&myIPADDR, 192, 168, 1, 20);

    /* PORT AYARLADIK */
    err = tcp_bind(tpcb, &myIPADDR, 5000);

    if (err == ERR_OK)
    {
        /* 3. HATTI DİNLEMEYE BAŞLADIK */
        tpcb = tcp_listen(tpcb);

        /* 4. CALLBACK GELİYOR BİTİNCE */
        tcp_accept(tpcb, tcp_server_accept);
    }
    else
    {
       // bellek sızıntısını önlemek için PCB'yi serbest bırakmamız gerekir.
    	tcp_close(tpcb);
    }
}

struct tcp_pcb *active_client_pcb = NULL;

err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{


	if(err != ERR_OK)
		return ERR_RST; // iletisimi durdur

	tcp_recv(newpcb, tcp_server_recv);

	tcp_send_message(newpcb, message, 5, TCP_WRITE_FLAG_COPY);

	active_client_pcb = newpcb;

	return ERR_OK;


}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if(p == NULL) // mesaj gelmediyse iletişim bitmiştir
	{
		tcp_close(tpcb);
		return ERR_OK;

	}
	tcp_recved(tpcb, p->tot_len);

	//p->payload veri kısmı gelen mesaj
	//tcp_send_message(tpcb, p->payload, p->tot_len, TCP_WRITE_FLAG_COPY);

	pbuf_free(p); // eski veriyi sildik

	return ERR_OK;

}

void tcp_send_message(struct tcp_pcb * 	pcb, const void * 	arg, u16_t len, u8_t apiflags )
{

	//flag yerine TCP_WRITE_FLAG_COPY yazabilirsin
	tcp_write(pcb, arg, len, apiflags);


	tcp_output(pcb);

}

// Anlık Roll, Pitch, Yaw verisini bağlı olan TCP istemcisine saniyede onlarca kez basmak için:
void tcp_send_sensor_data(struct tcp_pcb *tpcb, float r, float p, float y) {
    if (tpcb != NULL && tpcb->state == ESTABLISHED) {
        char packet[64];
        int len = sprintf(packet, "%.2f,%.2f,%.2f\n", r, p, y);
        tcp_write(tpcb, packet, len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
    }
}

// Anlık q0, q1, q2, q3 kuaterniyon verisini basmak için:
void tcp_send_quaternion_data(struct tcp_pcb *tpcb, float q0, float q1, float q2, float q3) {
    if (tpcb != NULL && tpcb->state == ESTABLISHED) {
        char packet[64];
        int len = sprintf(packet, "%.4f,%.4f,%.4f,%.4f\n", q0, q1, q2, q3);
        tcp_write(tpcb, packet, len, TCP_WRITE_FLAG_COPY);

        tcp_output(tpcb);
    }
}

