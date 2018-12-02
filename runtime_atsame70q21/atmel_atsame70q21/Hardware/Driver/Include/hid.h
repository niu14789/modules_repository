/** 
* @file      hid.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/
#ifndef _HID_H_
#define _HID_H_
#include "board.h"
#include <HIDDTransferDriver.h>
#define USB_HID_RXSIZE 4096
#define USB_HID_TXSIZE 4096
struct usb_buff{
	uint8_t *buff;
	uint16_t head;
	uint16_t tail;
	uint16_t size;
};
extern USBDDriverDescriptors hiddTransferDriverDescriptors;
void usb_hid_init(void);
void hid_timer_tick(void);
int16_t get_hid_data_from_buff(void);
extern uint16_t send_hid_data_to_buff(const uint8_t *buff,uint16_t len);
#endif


