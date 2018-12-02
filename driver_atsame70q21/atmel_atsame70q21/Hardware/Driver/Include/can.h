/** 
* @file      can.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
#ifndef _CAN_H_
#define _CAN_H_

#include "board.h"

#define MSG_ID_0           0x19       ///< standard ID
#define MSG_ID_0_DATA1     0x81        ///< 1 byte message
#define MSG_ID_1           0x11111111  ///< extended ID
#define MSG_ID_1_DATA1_4   0x55555555  ///< 8 byte message
#define MSG_ID_1_DATA5_8   0x00FF00FF
#define MSG_ID_2           0x01       ///< standard ID
#define MSG_ID_2_MASK      0x7FC       ///< bits 0 & 1 are don't care
#define TX_BUFFER_0        0
#define TX_BUFFER_1        1
#define RX_BUFFER_0        0
#define RX_BUFFER_1        1
#define FILTER_0           0
#define FILTER_1           1

#define BUTTON_NOT_PUSHED  1
#define BUTTON_PUSHED      0

#define CAN_STANDARD       0
#define CAN_FD             1
void can_dev_init(void);
uint8_t can_data_send(uint8_t *buff,uint8_t len,uint32_t id);
uint8_t can_cmd_send(uint8_t *buff,uint8_t len,uint32_t id);
uint8_t get_data_can_data(uint8_t *buff,uint32_t * id);
uint8_t get_cmd_can_data(uint8_t *buff,uint32_t * id);
#endif





