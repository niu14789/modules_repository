/** 
* @file     sbus.c 
* @brief    sbus驱动. 
* @details  接收sbus数据并解析. 
* @author   zbb 
* @date      2016-7-28
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
//#include "FreeRTOS.h"
//#include <stdbool.h>
//#include <stdio.h>
//#include <string.h>
//#include "cmsis_os.h"
#include "sbus.h"
#include "interprocess_comm.h"
#include "uartd.h"
#include "board.h"
#include "cmsis_os.h"
#include "main.h"

static SbusState_t SbusState={false,false,0};
static uint32_t SbusChannelData[SBUS_MAX_CHANNEL];
SbusFrame_t SbusFrame;
static uint8_t  SbusFramePosition = 0;
static uint32_t SbusFrameStartAt = 0;
extern uint32_t HAL_GetTick(void);

/**
 * funcation SbusDataReceive
 * @brief 接收SBUS数据
 * @param c 串口数据
 * @return 无
 */
void SbusDataReceive(uint8_t c)
{
  uint32_t now = HAL_GetTick();
	uint32_t sbusFrameTime = now - SbusFrameStartAt;
  if (sbusFrameTime > SBUS_TIME)			
	{
    SbusFramePosition = 0;
  }
	if (SbusFramePosition == 0) 
	{
		//SbusFrameStartAt = now;
		if (c != SBUS_FRAME_BEGIN_BYTE) 
		{
			uart_sbus_flush();
			//printf("failed\n");
      return;
    }
    SbusFrameStartAt = now;
  }
	if (SbusFramePosition < SBUS_FRAME_SIZE) 
	{
    SbusFrame.bytes[SbusFramePosition++] = (uint8_t)c;
    if (SbusFramePosition == SBUS_FRAME_SIZE) 
		{
      SbusState.SbusReceiveDone = true;
    } 
		else 
		{
      SbusState.SbusReceiveDone = false;
    }
  }
}
/**
 * funcation sbusReadRawRC
 * @brief 获取sbus数据
 * @param chan 通道号
 * @return sbus数据
 */
uint16_t sbusReadRawRC(uint8_t chan)
{
  return (0.625f * SbusChannelData[chan]) + 880;
}

/**
 * function debug_sbus
 * @brief 	test sbus for debug
 * @param 	null
 * @return  null
 */
void debug_sbus(void)
{
	uint32_t i = 0;
		
	for(i = 0; i<SBUS_MAX_CHANNEL; i++)
	{
		printf("%d ", sbusReadRawRC(i));
	}
	
	printf("\r\n");
}

/**
 * function test_1
 * @brief 测试任务
 * @param 任务参数
 * @return 无
 */
bool _new_input_;
void task_radio_in( void *argument)
{
	uint8_t c = 0;
	uart_sbus_initialization(100000);
	SbusFrame_t* temp;
	_new_input_=false;
	//uint32_t starttime = 0;
	//uint32_t bytesToRead = 0;
	while(1)
	{
		//starttime = HAL_GetTick();
		uart_sbus_tick();
		//bytesToRead = uxQueueMessagesWaiting(queue_sbusrx);
		//printf("%d,",bytesToRead);
		while(xQueueReceive(queue_sbusrx, &c, 0) == pdPASS)
		{
			//printf("%x,",c);
			SbusDataReceive(c);
			if (SbusState.SbusReceiveDone) 
			{
				break;
			}
		}
		if (SbusState.SbusReceiveDone) 
		{
			SbusState.SbusReceiveDone = false;
			SbusChannelData[0] = SbusFrame.frame.chan0;
			SbusChannelData[1] = SbusFrame.frame.chan1;
			SbusChannelData[2] = SbusFrame.frame.chan2;
			SbusChannelData[3] = SbusFrame.frame.chan3;
			SbusChannelData[4] = SbusFrame.frame.chan4;
			SbusChannelData[5] = SbusFrame.frame.chan5;
			SbusChannelData[6] = SbusFrame.frame.chan6;
			SbusChannelData[7] = SbusFrame.frame.chan7;
			SbusChannelData[8] = SbusFrame.frame.chan8;
			SbusChannelData[9] = SbusFrame.frame.chan9;
			SbusChannelData[10] = SbusFrame.frame.chan10;
			SbusChannelData[11] = SbusFrame.frame.chan11;
			SbusChannelData[12] = SbusFrame.frame.chan12;
			SbusChannelData[13] = SbusFrame.frame.chan13;
			SbusChannelData[14] = SbusFrame.frame.chan14;
			SbusChannelData[15] = SbusFrame.frame.chan15;

			if (SbusFrame.frame.flags & SBUS_FLAG_CHANNEL_17) 
			{
        SbusChannelData[16] = SBUS_DIGITAL_CHANNEL_MAX;
			} 
			else 
			{
        SbusChannelData[16] = SBUS_DIGITAL_CHANNEL_MIN;
			}

			if (SbusFrame.frame.flags & SBUS_FLAG_CHANNEL_18) 
			{
        SbusChannelData[17] = SBUS_DIGITAL_CHANNEL_MAX;
			} 
			else 
			{
        SbusChannelData[17] = SBUS_DIGITAL_CHANNEL_MIN;
			}
			
			if(uxQueueSpacesAvailable(queue_radio_in) == 0)
			{
				_new_input_=true;
				xQueueReceive(queue_radio_in, (void*)&temp, 1);
			}
			xQueueSendToBack(queue_radio_in, (void*)&SbusFrame, 1);
//			printf("%d,",HAL_GetTick() - starttime);
//			printf("%d\n",SbusChannelData[2]);
    }
		osDelay(1);
	}
}

