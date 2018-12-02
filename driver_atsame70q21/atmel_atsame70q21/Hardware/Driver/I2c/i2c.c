/** 
* @file     i2c.c 
* @brief    i2c驱动. 
* @details  i2c模式初始化，数据收发接口. 
* @author   tcf 
* @date      2016-7-28
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
#include "i2c.h"
#include "global.h"

#include "main.h"
#include "board.h"
#include "cmsis_os.h"

#define TWCK            400000 ///<TWI clock frequency in Hz.

#define AT24MAC_ADDRESS         0x57 ///<Slave address of twi_eeprom example
#define AT24MAC_SERIAL_NUM_ADD  0x5F 

#define PAGE_SIZE       16 ///<Page size of an AT24MAC402 chip (in bytes)

#define EEPROM_PAGES    16 ///<Page numbers of an AT24MAC402 chip

#define BOARD_PINS_TWI0_PORT PINS_TWI0 ///<Eeprom Pins definition
#define BOARD_ID_TWI_EEPROM  ID_TWIHS0 ///<TWI0 peripheral ID for eeprom device
#define BOARD_BASE_TWI_SLAVE TWIHS0 ///<TWI0 base address for eeprom device

static const Pin pins[] = BOARD_PINS_TWI0_PORT; ///<Pio pins to configure.
COMPILER_ALIGNED(32) static uint8_t pData[PAGE_SIZE] = "this iic test\r\n"; ///<Page buffer.

static Twid twid;///<TWI driver instance
/**
 * funcation i2c_init
 * @brief i2c初始化
 * @param dev_num i2c设备号
 * @return 无
 */
void i2c_init(uint8_t dev_num) 
{
	PIO_Configure(pins, PIO_LISTSIZE(pins));///<Configure TWI pins. PA3 PA4
	PMC_EnablePeripheral(PMC_ID_TWI0);///<Power clock

	TWI_ConfigureMaster(BOARD_BASE_TWI_SLAVE, TWCK, BOARD_MCK);	///<Configure TWI
	TWID_Initialize(&twid, BOARD_BASE_TWI_SLAVE);

	NVIC_ClearPendingIRQ(TWIHS0_IRQn);///<Configure TWI interrupts 
	NVIC_SetPriority(SPI0_IRQn ,1);
	NVIC_EnableIRQ(TWIHS0_IRQn);
	
}

/**
 * funcation i2c_send
 * @brief iic send test
 * @param 无
 * @return 无
 */
void i2c_send(void)
{
	uint8_t pData[]={1,2,3,4};
	TWID_Write(&twid, 0x58, 0, 0, pData, 4, 0);
	//TWID_DmaRead(&twi_dma, 0x58, 0, 0, pData, 4, 0);
}


/**
 * funcation TWIHS0_Handler
 * @brief TWI interrupt handler. Forwards the interrupt to the TWI driver handler.
 * @param 无
 * @return 无
 */
void TWIHS0_Handler(void)
{
	TWID_Handler(&twid);
}


uint32_t CallBackFired = 0;
/**
 * funcation TestCallback
 * @brief transfer complete to callback interffice
 * @param 无
 * @return 无
 */
static void TestCallback(void)
{
	CallBackFired++;
}



/**
 * funcation task_iic
 * @brief the task for iic
 * @param 无
 * @return 无
 */
void task_iic(const void *arg)
{

	Async async;
	
	memset(&async, 0, sizeof(async));
	async.callback = (void *) TestCallback;
	
	
	TWID_Write(&twid, AT24MAC_ADDRESS, 0, 1, pData, PAGE_SIZE, &async);
	//while (!ASYNC_IsFinished(&async));
	osDelay(5);
	if(ASYNC_IsFinished(&async) == 1)
	{
		///<send complete callback;
		;
	}
	
	memset(pData, 0, PAGE_SIZE);
	
	TWID_Read(&twid, AT24MAC_ADDRESS, 0, 1, pData, PAGE_SIZE, &async);
	//while (!ASYNC_IsFinished(&async));
	osDelay(5);
	if(ASYNC_IsFinished(&async) == 1)
	{
		///<send complete callback;
		;
	}
	
	while(1)
	{
		//printf((const char *)pData);
		memset(pData, 0, PAGE_SIZE);
		TWID_Read(&twid, AT24MAC_ADDRESS, 0, 1, pData, PAGE_SIZE, &async);
		osDelay(5000);
		spi_send_data(NULL);
	}
}

















