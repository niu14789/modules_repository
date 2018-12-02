/** 
* @file  hid.c 
* @brief   usb hid模式驱动. 
* @details  usb hid模式初始化，数据收发接口. 
* @author   zbb 
* @date      2016-7-28
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
#include "hid.h"
#include <string.h>
#include "cmsis_os.h"
uint8_t usb_hid_rxbuff[USB_HID_RXSIZE]={0};
uint8_t usb_hid_txbuff[USB_HID_TXSIZE]={0};
struct usb_buff hid_txbuff;
struct usb_buff hid_rxbuff;
//osMutexId mutex_hid_rx;
//osMutexId mutex_hid_tx;
/**
 * funcation _ConfigureUotghs
 * @brief Configure USB settings for USB device，UTMI parallel mode, High/Full/Low Speed
 * @param 无
 * @return 无
 */
static void _ConfigureUotghs(void)
{
	PMC->PMC_SCDR = PMC_SCDR_USBCLK; ///<UUSBCK not used in this configuration (High Speed)
	PMC->PMC_USB = PMC_USB_USBS;///<USB clock register: USB Clock Input is UTMI PLL
	PMC_EnablePeripheral(ID_USBHS); ///<Enable peripheral clock for USBHS 
	USBHS->USBHS_CTRL = USBHS_CTRL_UIMOD_DEVICE;
	PMC->CKGR_UCKR = CKGR_UCKR_UPLLEN | CKGR_UCKR_UPLLCOUNT(0xF);///<Enable PLL 480 MHz
	while (!(PMC->PMC_SR & PMC_SR_LOCKU));///<Wait that PLL is considered locked by the PMC


	NVIC_EnableIRQ(USBHS_IRQn);///<enable IRQ
}
/**
 * funcation usb_hid_init
 * @brief usb hid模式初始化
 * @param 无
 * @return 无
 */
void usb_hid_init(void)
{
	osMutexDef_t mutex_dummy;
	_ConfigureUotghs();
	HIDDTransferDriver_Initialize(&hiddTransferDriverDescriptors);///<HID driver initialization
	NVIC_SetPriority(USBHS_IRQn ,5);
	USBD_Connect();///<connect if needed
	hid_txbuff.buff=usb_hid_txbuff;
	hid_txbuff.head=0;
	hid_txbuff.tail=0;
	hid_txbuff.size=USB_HID_TXSIZE;
	
	hid_rxbuff.buff=usb_hid_rxbuff;
	hid_rxbuff.head=0;
	hid_rxbuff.tail=0;
	hid_rxbuff.size=USB_HID_RXSIZE;
	
	mutex_hid_rx = osMutexCreate(&mutex_dummy);
	mutex_hid_tx = osMutexCreate(&mutex_dummy);
}
/**
 * funcation get_hid_buff_avaliable
 * @brief 获取usb hid buff 有效数据
 * @param hid_buff: 指针 ：rx/tx
 * @return avaliable: 有效数据字节
 */
static uint16_t get_hid_buff_avaliable(struct usb_buff * hid_buff)
{
	 uint16_t avaliable=0;
	if(hid_buff->head == hid_buff->tail)
	{
		avaliable=0;
	}
    else if(hid_buff->head > hid_buff->tail)
	{
		avaliable=hid_buff->size-(hid_buff->head - hid_buff->tail);
	}
	else if(hid_buff->head < hid_buff->tail)
	{
		avaliable=hid_buff->tail - hid_buff->head;
	}
	else
	{
	
	}
	return avaliable;
}
/**
 * funcation get_hid_buff_space
 * @brief 获取usb hid buff 剩余空间
 * @param hid_buff: 指针 ：rx/tx
 * @return space: 剩余空间字节
 */
static uint16_t get_hid_buff_space(struct usb_buff * hid_buff)
{
	 uint16_t space=0;
	if(hid_buff->head == hid_buff->tail)
	{
		space=hid_buff->size;
	}
    else if(hid_buff->head > hid_buff->tail)
	{
		space=hid_buff->head - hid_buff->tail -1;
	}
	else if(hid_buff->head < hid_buff->tail)
	{
		space=hid_buff->size - (hid_buff->tail - hid_buff->head)-1;
	}
	else
	{}
	return space;
}
/**
 * funcation update_hid_buff
 * @brief 维护hid buff指针
 * @param 无
 * @return 无
 */
static void update_hid_buff(struct usb_buff * hid_buff)
{
	hid_buff->head = hid_buff->head % hid_buff->size;
	hid_buff->tail = hid_buff->tail % hid_buff->size;
}
/**
 * funcation send_hid_data_to_buff
 * @brief 发送数据到hid buff
 * @param buff:需要发送数据指针
 * @param len:发送长度
 * @return 实际发送长度
 */
uint16_t send_hid_data_to_buff(const uint8_t *buff,uint16_t len)
{
    uint16_t space=0;
	uint16_t ret=0;
	osMutexWait(mutex_hid_tx,5);
	
	space=get_hid_buff_space(&hid_txbuff);
    if(space<len)
	{
		ret=0;
	}
	else if(space >=len)
	{
		if(hid_txbuff.head > hid_txbuff.tail)
		{
			memcpy(&hid_txbuff.buff[hid_txbuff.tail],buff,len);
			hid_txbuff.tail +=len;
			update_hid_buff(&hid_txbuff);
		}
		else if((hid_txbuff.size - hid_txbuff.tail) >= len)
		{
			memcpy(&hid_txbuff.buff[hid_txbuff.tail],buff,len);
			hid_txbuff.tail +=len;
			update_hid_buff(&hid_txbuff);
		}
		else 
		{
			ret=hid_txbuff.size - hid_txbuff.tail;
			memcpy(&hid_txbuff.buff[hid_txbuff.tail],buff,ret);
			hid_txbuff.tail +=ret;
			update_hid_buff(&hid_txbuff);
			buff +=ret;
			memcpy(&hid_txbuff.buff[hid_txbuff.tail],buff,len - ret);
		}
		ret=len;
	}
	else
	{}
	osMutexRelease(mutex_hid_tx);
	return ret;
}
/**
 * funcation get_hid_data_from_buff
 * @brief 从hid rxbuff获取数据
 * @param 无
 * @return 获取到的数据
 */
int16_t get_hid_data_from_buff(void)
{
	uint16_t avaliable=0;
	int16_t c= -1;

	osMutexWait(mutex_hid_rx,5);

	avaliable=get_hid_buff_avaliable(&hid_rxbuff);
	if(avaliable >0)
	{
		c=hid_rxbuff.buff[hid_rxbuff.head];
		hid_rxbuff.head += 1;
		update_hid_buff(&hid_rxbuff);
	}
	osMutexRelease(mutex_hid_rx);
	return c;
}
/**
 * funcation hid_timer_tick
 * @brief 定时从usb 发送接收数据
 * @param 无
 * @return 无
 */
void hid_timer_tick(void)
{
	uint16_t avaliable=0;
	uint16_t space=0;
	uint16_t num=0;
    uint8_t buff[512]={0};
	uint16_t len=0;
	uint16_t ret=0;
	osMutexWait(mutex_hid_tx,5);
	avaliable=get_hid_buff_avaliable(&hid_txbuff);
	if(avaliable>0)
	{	
		num=hid_txbuff.size - hid_txbuff.head;
		if(num > avaliable)
		{
			if(USBD_GetState() >= USBD_STATE_CONFIGURED)	
			{			
				HIDDTransferDriver_Write(&hid_txbuff.buff[hid_txbuff.head], avaliable, 0, 0);
				hid_txbuff.head +=avaliable;
				update_hid_buff(&hid_txbuff);
			}
		}
		else
		{
			if(USBD_GetState() >= USBD_STATE_CONFIGURED)	
			{			
				HIDDTransferDriver_Write(&hid_txbuff.buff[hid_txbuff.head], num, 0, 0);
				hid_txbuff.head +=num;
				update_hid_buff(&hid_txbuff);
			}
			if(USBD_GetState() >= USBD_STATE_CONFIGURED)	
			{
				HIDDTransferDriver_Write(&hid_txbuff.buff[hid_txbuff.head], avaliable-num, 0, 0);
				hid_txbuff.head +=(avaliable-num);
				update_hid_buff(&hid_txbuff);
			}

		}
	}
	osMutexRelease(mutex_hid_tx);
	
	
	osMutexWait(mutex_hid_rx,5);
	
	len=HIDDTransferDriver_Read(buff,512);
if(len >0)
{	
	space=get_hid_buff_space(&hid_rxbuff);
	if(space >len)
	{
		if(hid_rxbuff.head > hid_rxbuff.tail)
		{
			memcpy(&hid_rxbuff.buff[hid_rxbuff.tail],buff,len);
			hid_rxbuff.tail +=len;
			update_hid_buff(&hid_rxbuff);
		}
		else if((hid_rxbuff.size - hid_rxbuff.tail) >= len)
		{
			memcpy(&hid_rxbuff.buff[hid_rxbuff.tail],buff,len);
			hid_rxbuff.tail +=len;
			update_hid_buff(&hid_rxbuff);
		}
		else 
		{
			ret=hid_rxbuff.size - hid_rxbuff.tail;
			memcpy(&hid_rxbuff.buff[hid_rxbuff.tail],buff,ret);
			hid_rxbuff.tail +=ret;
			update_hid_buff(&hid_rxbuff);
			//buff +=ret;
			memcpy(&hid_rxbuff.buff[hid_rxbuff.tail],&buff[ret],len - ret);
			//此处欠一顿饭 zhangbb@feimarobotics.com
            hid_rxbuff.tail +=(len - ret);
			update_hid_buff(&hid_rxbuff);
		}
	}
}
	osMutexRelease(mutex_hid_rx);
}


