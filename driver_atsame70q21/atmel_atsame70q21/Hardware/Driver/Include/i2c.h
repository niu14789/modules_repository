/** 
* @file      i2c.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/
#ifndef _I2C_H_
#define _I2C_H_
#include "board.h"

#ifndef G_I2C_EXTERN
#define G_I2C_EXTERN extern
#else
#define G_I2C_EXTERN 	
#endif
#define PMC_ID_TWI0   ID_TWIHS0 ///<TWI0 peripheral ID for EEPROM device
#define I2C_BASE_DEV0 TWIHS0
#define IIC_ENABLE ///<the macro enable iic

#define TWCK            400000 ///<TWI clock frequency in Hz.
G_I2C_EXTERN void i2c_send(void);
G_I2C_EXTERN void i2c_init(uint8_t dev_num);
G_I2C_EXTERN void iic_read_write_test(void);
G_I2C_EXTERN void task_iic(const void *arg);
#endif



