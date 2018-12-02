/*
 * pwm.c
 *
 *  Created on: 2017年5月17日
 *      Author: niu
 */
#include "board.h"
#include "fs.h"
#include "fs_config.h"
#include "f_shell.h"
#include "string.h"
#include "pwm.h"
#include "f_drv.h"
/* default pins */
static const Pin pwm_pins[] = {         ///<Pins to configure for the application.
	PIN_PWMC0_PWMH0,
	PIN_PWMC0_PWMH1,
	PIN_PWMC0_PWMH2,
	PIN_PWMC0_PWMH3,
	PIN_PWMC1_PWMH0
};
/* declaration */
struct file * pwm_dev_open(FAR struct file * filp);
/* fs inode system register */
FS_INODE_REGISTER("/pwm.o",pwm,pwm_heap_init,0);
/* heap init */
int pwm_heap_init(void)
{
  /* full of zero */
	memset(&pwm,0,sizeof(pwm));
	/* shell base */
	pwm.shell_i = shell_sched_getfiles();
	/* file opearation */
	pwm.ops.open = pwm_dev_open;
	/* driver config */
	pwm.config = pwm_default_config;
	pwm.drv_ops.config = pwm_config;
	pwm.drv_ops.write = pwm_wtire;
	pwm.drv_ops.enable = pwm_enable;
	pwm.drv_ops.disable = pwm_disable;
	/* file interface */
	pwm.flip.f_inode = &pwm;
	pwm.flip.f_path = "/pwm.o";
	/* heap */

	/* add your own code here */
  pwm.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
/* default config */
int pwm_default_config(void)
{
	pwm_config_msg msg;
	msg.div = 4;
	msg.period = 11718*2;
	msg.dutycycle = 11718*2;
	/* config */
	pwm_config(&msg,sizeof(msg));
	/* return */
	return FS_OK;
}

int pwm_config(void * p_arg , int argc)
{
	/* pointer */
	pwm_config_msg * msg;
	/* if the length not squal */	
	if(sizeof(pwm_config_msg) != argc)
	{
		return FS_ERR;// not supply this format
	}
	/* force */
	msg = ( pwm_config_msg *)p_arg;
	/* start config */
	PMC_EnablePeripheral(ID_PWM0);
  PMC_EnablePeripheral(ID_PWM1);
	PIO_Configure(pwm_pins, PIO_LISTSIZE(pwm_pins));///<IO config
	/* channel 0 */
	PWMC_ConfigureChannel(PWM0,0,msg->div,0,0);
	PWMC_SetPeriod(PWM0,0,msg->period);
	PWMC_SetDutyCycle(PWM0,0,msg->dutycycle);
	/* channel 1 */
	PWMC_ConfigureChannel(PWM0,1,msg->div,0,0);
	PWMC_SetPeriod(PWM0,1,msg->period);
	PWMC_SetDutyCycle(PWM0,1,msg->dutycycle);
	/* channel 2 */
	PWMC_ConfigureChannel(PWM0,2,msg->div,0,0);
	PWMC_SetPeriod(PWM0,2,msg->period);
	PWMC_SetDutyCycle(PWM0,2,msg->dutycycle);
	/* channel 3 */
	PWMC_ConfigureChannel(PWM0,3,msg->div,0,0);
	PWMC_SetPeriod(PWM0,3,msg->period);
	PWMC_SetDutyCycle(PWM0,3,msg->dutycycle);
	/* pwm1 channel 0 */
	PWMC_ConfigureChannel(PWM1,0,msg->div,0,0);
	PWMC_SetPeriod(PWM1,0,msg->period);
	PWMC_SetDutyCycle(PWM1,0,msg->dutycycle);
	/* enable all channel */
	PWMC_EnableChannel(PWM0,0);
	PWMC_EnableChannel(PWM0,1);
	PWMC_EnableChannel(PWM0,2);
	PWMC_EnableChannel(PWM0,3);
	PWMC_EnableChannel(PWM1,0);
	/* return */
	return FS_OK;
}

/* file opearation */
struct file * pwm_dev_open(FAR struct file * filp)
{
	pwm.flip.f_oflags = __FS_IS_INODE_OK;
	return &pwm.flip;
}
/* driver interface */
int pwm_wtire(int type,void *buffer,int width,unsigned int size)
{
	unsigned short * pwm_value;
	
	if(size != 8 || width != 2)
	{
		/* can not supply this format */
		return FS_ERR;
	}
	/* force */
	pwm_value = (unsigned short *)buffer;
	/* whitch one ? */
	if( type == 4 )
	{
		PWMC_SetDutyCycle(PWM1,0,pwm_value[0]);
		return FS_OK;
	}
	/* equle */
	PWMC_SetDutyCycle(PWM0,0,pwm_value[0]);
	PWMC_SetDutyCycle(PWM0,1,pwm_value[1]);
	PWMC_SetDutyCycle(PWM0,2,pwm_value[2]);
	PWMC_SetDutyCycle(PWM0,3,pwm_value[3]);
	/* return */
	return FS_OK;
}
/* enable */
int pwm_enable(int mode)
{
	if( mode < 4 )
	{
		/* enable all channel */
		PWMC_EnableChannel(PWM0,0);
		PWMC_EnableChannel(PWM0,1);
		PWMC_EnableChannel(PWM0,2);
		PWMC_EnableChannel(PWM0,3);
  }else
	{
		PWMC_EnableChannel(PWM1,0);
	}
	return FS_OK;
}
/* disable */
int pwm_disable(int mode)
{
	if( mode < 4 )
	{	
		/* enable all channel */
		PWMC_DisableChannel(PWM0,0);
		PWMC_DisableChannel(PWM0,1);
		PWMC_DisableChannel(PWM0,2);
		PWMC_DisableChannel(PWM0,3);
	}else
	{
		PWMC_DisableChannel(PWM1,0);
	}
	return FS_OK;
}











































