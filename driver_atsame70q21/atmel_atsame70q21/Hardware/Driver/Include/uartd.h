/** 
* @file      uartd.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/
#ifndef _UARTD_H_
#define _UARTD_H_

#include "board.h"

#define TX_BUFF_SIZE         512
#define RX_BUFF_SIZE         512

#define TX2_BUFF_SIZE         512
#define RX2_BUFF_SIZE         256//bd930 nav
#define TX1_BUFF_SIZE         64
#define RX1_BUFF_SIZE         8192//rt27
#define RX3_BUFF_SIZE         25//sbus
#define RX4_BUFF_SIZE         42//sp25
#define TX5_BUFF_SIZE         512
#define RX5_BUFF_SIZE         10//link
#define TX7_BUFF_SIZE         256
#define RX7_BUFF_SIZE         2048//ublox

#define UART_DEV_0  UART0         
#define UART_DEV_1	UART1         
#define UART_DEV_2	UART2         
#define UART_DEV_3	UART3         
#define UART_DEV_4  UART4         
#define UART_DEV_5  USART0         
#define UART_DEV_6	USART1        
#define UART_DEV_7	USART2       

#define LLI_SIZE_STATIC              (2)

enum uart_id
{
	UART_0=0,
	UART_GPS_COM1=1,
	UART_GPS_COM2=2,
	UART_SBUS=3,
	UART_4=4,
	USART_LINK=5,
	USART_6=6,
	USART_GPS_UBLOX=7
};
struct uart_buff
{
	uint8_t *buff;
	uint16_t head;
	uint16_t tail;
	uint16_t size;
};


void uart_link_initialization(uint32_t baudrate);

extern void dma_init(void);

int u_s_art_heap_init(void);
int uart_default_config(void);
int u_s_art_config(void * p_arg , int argc);
int u_s_art_callback(int type,void * data,int len);
int dma(int type,void * data,int len);
void u_s_art_dma_tx_cb(char id,uint8_t ichannel, void * data);
void u_s_art_dma_rx_cb(char id,uint8_t ichannel, void * data);
#endif


