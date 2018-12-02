/** 
* @file      can.c 
* @brief     can驱动. 
* @details  can驱动，包括can初始化，can数据收发接口. 
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
#include "can.h"
#include <string.h>
#include "cmsis_os.h"
#define CAN_QUEUE_LEN  20
#define CAN_ITEM_SIZE  20
uint8_t        txData[8];
uint32_t       txdCntr;
uint32_t       rxdCntr;
uint32_t       rxFifoCntr;
uint32_t     * txMailbox0;
uint32_t     * txMailbox1;
Mailbox8Type   rxMailbox0;
Mailbox8Type   rxMailbox1;
Mailbox8Type   rxMailbox2;
QueueHandle_t data_receive_queue;
QueueHandle_t cmd_receive_queue;
static portBASE_TYPE can_higherprioritytaskwoken = pdFALSE;
/**
 * funcation Can_Dev_init
 * @brief can初始化
 * @param  dev_num：can设备编号 can1/can2
 * @return 无
 */
void can_dev_init(void)
{
	MATRIX->CCFG_SYSIO |= (1u << 12);
	
	
	MCAN_Init(&mcan1Config);
	MCAN_InitTxQueue(&mcan1Config);
//	MCAN_InitLoopback(&mcan1Config); 
	MCAN_Enable(&mcan1Config);
	MCAN_IEnableMessageStoredToRxDedBuffer(&mcan1Config, CAN_INTR_LINE_1);
//	MCAN_ConfigRxClassicFilter(&mcan1Config, CAN_FIFO_0, FILTER_0, MSG_ID_2,CAN_STD_ID, 0xff);	
	NVIC_SetPriority(MCAN1_LINE1_IRQn ,5);

	
	MCAN_Init(&mcan0Config);
	MCAN_InitTxQueue(&mcan0Config);
//	MCAN_InitLoopback(&mcan0Config); 
	MCAN_Enable(&mcan0Config);
	MCAN_IEnableMessageStoredToRxDedBuffer(&mcan0Config, CAN_INTR_LINE_1);
	MCAN_ConfigRxClassicFilter(&mcan0Config, CAN_FIFO_0, FILTER_0, MSG_ID_2,CAN_STD_ID, 0x7f0);	
	NVIC_SetPriority(MCAN0_LINE1_IRQn ,5);

	
	data_receive_queue=xQueueCreate(CAN_QUEUE_LEN,CAN_ITEM_SIZE);
	cmd_receive_queue=xQueueCreate(CAN_QUEUE_LEN,CAN_ITEM_SIZE);
}
/**
 * funcation can_data_send
 * @brief 数据can发送函数
 * @param  buff:发送数据指针 
 * @param  len:发送数据长度
 * @param  id:can数据帧标识符
 * @return len：成功发送长度
 */
uint8_t can_data_send(uint8_t *buff,uint8_t len,uint32_t id)
{

    uint8_t ret;
	if(len>8)
	{
	   ret=0;
	}
    else
	{

		MCAN_AddToTxFifoQ(&mcan1Config, id, CAN_STD_ID,(MCan_DlcType)len, buff);

		while(!(MCAN_IsTxComplete(&mcan1Config)))
		{
		}
		while (!(MCAN_IsBufferTxd(&mcan1Config, TX_BUFFER_0)))	
		{	
		}
		MCAN_ClearTxComplete(&mcan1Config);
		ret=len;
	}
	return ret;
}
/**
 * funcation can_cmd_send
 * @brief 控制can发送函数
 * @param  buff:发送数据指针 
 * @param  len:发送数据长度
 * @param  id:can数据帧标识符
 * @return len：成功发送长度
 */
uint8_t can_cmd_send(uint8_t *buff,uint8_t len,uint32_t id)
{
    uint8_t ret;
	if(len>8)
	{
	   ret=0;
	}
    else
	{
		MCAN_AddToTxFifoQ(&mcan0Config, id, CAN_STD_ID,(MCan_DlcType)len, buff);

		while(!(MCAN_IsTxComplete(&mcan0Config)))
		{
		}
		while (!(MCAN_IsBufferTxd(&mcan0Config, TX_BUFFER_0)))	
		{	
		}
		MCAN_ClearTxComplete(&mcan0Config);
		ret=len;
	}  
	return ret;
}
/**
 * funcation MCAN1_Handler
 * @brief can1中断处理
 * @param 无
 * @return  无
 */
void MCAN1_Line1_Handler(void)
{
	if(MCAN_IsMessageStoredToRxFifo0(&mcan1Config))
	{
		MCAN_ClearMessageStoredToRxFifo0(&mcan1Config);
		SCB_InvalidateDCache_by_Addr((uint32_t *)mcan1Config.msgRam.pRxFifo0,mcan1Config.rxFifo0ElmtSize & 0x1F);
		MCAN_GetRxFifoBuffer(&mcan1Config, CAN_FIFO_0,(Mailbox64Type *) &rxMailbox1);
		printf("rxMailbox1:%u\n",rxMailbox1.data[0]);
		xQueueSendFromISR(data_receive_queue,&rxMailbox1,&can_higherprioritytaskwoken);
	}
}
/**
 * funcation MCAN0_Handler
 * @brief can0中断处理
 * @param 无
 * @return  无
 */
void MCAN0_Line1_Handler(void)
{
	if(MCAN_IsMessageStoredToRxFifo0(&mcan0Config))
	{
		MCAN_ClearMessageStoredToRxFifo0(&mcan0Config);
		SCB_InvalidateDCache_by_Addr((uint32_t *)mcan0Config.msgRam.pRxFifo0,mcan0Config.rxFifo0ElmtSize & 0x1F);
		MCAN_GetRxFifoBuffer(&mcan0Config, CAN_FIFO_0,(Mailbox64Type *) &rxMailbox0);
		printf("rxMailbox0:%u %u %u %u\n",rxMailbox0.data[0],rxMailbox0.data[1],rxMailbox0.data[2],rxMailbox0.data[3]);
		xQueueSendFromISR(cmd_receive_queue,&rxMailbox0,&can_higherprioritytaskwoken);
		
	}
}
/**
 * funcation get_can_data
 * @brief 接收数据can
 * @param buff:发送数据指针 
 * @param id:can数据id
 * @return 发送长度
 */
uint8_t get_data_can_data(uint8_t *buff,uint32_t * id)
{
	
	Mailbox8Type data;
	if(xQueueReceive( data_receive_queue, &data, ( TickType_t ) 0 )>0)
	{
		*id=data.info.id;
		memcpy(buff,&data.data[0],data.info.length);	
		return  data.info.length;
	}
	return 0;
}
/**
 * funcation get_can_cmd
 * @brief 接收控制can数据
 * @param buff:发送数据指针 
 * @param id:can数据id
 * @return 发送长度
 */
uint8_t get_cmd_can_data(uint8_t *buff,uint32_t * id)
{
	
	Mailbox8Type data;
	if(xQueueReceive( cmd_receive_queue, &data, ( TickType_t ) 0 )>0)
	{
		*id=data.info.id;
		memcpy(buff,&data.data[0],data.info.length);	
		return  data.info.length;
	}
	return 0;
}








