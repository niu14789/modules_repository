/** 
* @file      spi.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       ���ڷ�������˿Ƽ����޹�˾��������
*/
#ifndef __SPI_H__
#define __SPI_H__

#define SPI0_CS0  0
#define SPI0_CS1  1
#define SPI0_CS2  2
#define SPI0_CS3  3

#define SPI1_CS0  0
#define SPI1_CS1  1
#define SPI1_CS2  2
#define SPI1_CS3  3

int spi_heap_init(void);
int spi_default_config(void);
int spi_config(void * p_arg , int argc);
int spi_wtire(int type,void *buffer,int width,unsigned int size);
int spi_read(int type,void *buffer,int width,unsigned int size);
int spi_selectchip(int type,char status);
int spi_handler(int type,void * data,int len);
#endif

