/** 
* @file      sbus.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/
#ifndef _SBUS_H
#define _SBUS_H
#include "board.h"

#define SBUS_MAX_CHANNEL 18
#define SBUS_FRAME_SIZE 25

#define SBUS_FRAME_BEGIN_BYTE 0x0F

#define SBUS_BAUDRATE 100000

#define SBUS_TIME 4
#define SBUS_FLAG_CHANNEL_17        (1 << 0)
#define SBUS_FLAG_CHANNEL_18        (1 << 1)
#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3)
#define SBUS_DIGITAL_CHANNEL_MIN 173
#define SBUS_DIGITAL_CHANNEL_MAX 1812
typedef struct _SbusState
{
	bool SbusPortInit;
	bool SbusReceiveDone;
	uint16_t SbusStateFlag;
}SbusState_t;
struct _SbusFrame {
    uint8_t syncByte;  ///< 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
    unsigned int chan0 : 11;
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
    unsigned int chan12 : 11;
    unsigned int chan13 : 11;
    unsigned int chan14 : 11;
    unsigned int chan15 : 11;
    uint8_t flags;
    uint8_t endByte;
} __attribute__ ((__packed__));
typedef union {
    uint8_t bytes[SBUS_FRAME_SIZE];
    struct _SbusFrame frame;
} SbusFrame_t;

extern void task_radio_in( void *argument);
extern void SbusDataReceive(uint8_t c);

#endif






