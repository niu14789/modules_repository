/*
 * task.c
 *
 *  Created on: 2017-5-19
 *      Author: YJ-User17
 */
#include "fs.h"
#include "f_shell.h"
#include "tc.h"
#include "runtime.h"
#include "string.h"
#include "fs_config.h"
/* declares */
int runtime_dev_ioctl(FAR struct file *filp, int cmd, unsigned long arg,void *pri_data);
int system_shell_init(struct shell_cmd * p_shell_cmd,unsigned int max);
/* const static period */
const unsigned short system_task_period[3] = { TASK_PERIOD0 , TASK_PERIOD1 , TASK_PERIOD2 };//1ms 4ms 10ms
const unsigned short user_task_period[3] = { TASK_PERIOD3 , TASK_PERIOD4 , TASK_PERIOD5 };//100ms 200ms 500ms
/* task init */
const isr_handler_def __FS_t0[4] = { 0xADCF3568 ,39,task0_handler, 0x887F651D };
const isr_handler_def __FS_t1[4] = { 0xADCF3568 ,40,task1_handler, 0x887F651D };
const isr_handler_def __FS_t2[4] = { 0xADCF3568 ,41,task2_handler, 0x887F651D };
const isr_handler_def __FS_t3[4] = { 0xADCF3568 ,42,task3_handler, 0x887F651D };
const isr_handler_def __FS_t4[4] = { 0xADCF3568 ,43,task4_handler, 0x887F651D };
const isr_handler_def __FS_t5[4] = { 0xADCF3568 ,44,task5_handler, 0x887F651D };
/* fs inode system register */
FS_INODE_REGISTER("/runtime.o",runtime,task_heap_init,202);
/* fs shell register for start system */
FS_SHELL_REGISTER(system_start);
/* disable all it */
FS_SHELL_REGISTER(disable_all_it);
/* heap init */
int task_heap_init(void)
{
	 /* full of zero */
	memset(&runtime,0,sizeof(runtime));
	/* fs runs init */
	fs_runs_init();
	/* shell base */
	runtime.shell_i = shell_sched_getfiles();
	/* driver config */
	runtime.config = task_default_config;
	runtime.drv_ops.config = task_config;
	runtime.ops.ioctl = runtime_dev_ioctl;
	/* file interface */
	runtime.flip.f_inode = &runtime;
	runtime.flip.f_path = "/runtime.o";
	/* shell init */
	FS_SHELL_INIT(system_start,system_start,4,_CB_EXE_);
	FS_SHELL_INIT(disable_all_it,disable_all_it,4,_CB_EXE_);
	/* heap */
	
	/* add your own code here */
  runtime.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
///* task default config */
int task_default_config(void)
{
	int i;
	/* config the clock source */
	PMC_ConfigurePCK6(1, (11)<<4);//1:Main Clock 12MHz  12·ÖÆµ PCK6 1MHz
	/* enable the peripheral */
	PMC_EnablePeripheral(ID_TC0);	
	PMC_EnablePeripheral(ID_TC1);
	PMC_EnablePeripheral(ID_TC2);
	PMC_EnablePeripheral(ID_TC3);
	PMC_EnablePeripheral(ID_TC4);
	PMC_EnablePeripheral(ID_TC5);
	/* enable and config interrup  t */
	/* This is for math0 */
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn ,0);
//	NVIC_EnableIRQ(TC0_IRQn);

	NVIC_ClearPendingIRQ(TC1_IRQn);
	NVIC_SetPriority(TC1_IRQn ,1);
//	NVIC_EnableIRQ(TC1_IRQn);

	NVIC_ClearPendingIRQ(TC2_IRQn);
	NVIC_SetPriority(TC2_IRQn ,2);
//	NVIC_EnableIRQ(TC2_IRQn);

	NVIC_ClearPendingIRQ(TC3_IRQn);
	NVIC_SetPriority(TC3_IRQn ,4);
//	NVIC_EnableIRQ(TC3_IRQn);

	NVIC_ClearPendingIRQ(TC4_IRQn);
	NVIC_SetPriority(TC4_IRQn ,4);
//	NVIC_EnableIRQ(TC4_IRQn);

	NVIC_ClearPendingIRQ(TC5_IRQn);
	NVIC_SetPriority(TC5_IRQn ,4);
//	NVIC_EnableIRQ(TC5_IRQn);

	NVIC_ClearPendingIRQ(TC6_IRQn);
	NVIC_SetPriority(TC6_IRQn ,4);
//	NVIC_EnableIRQ(TC6_IRQn);
	/* return */
	/* config the channel for system task */
	for( int i = 0 ; i < 3 ; i ++ )
	{
		TC0->TC_CHANNEL[i].TC_CCR = (1<<1); //disable the counter
		TC0->TC_CHANNEL[i].TC_CMR = TC_CMR_CPCTRG | 0;  //compare enable
		TC0->TC_CHANNEL[i].TC_IER = TC_IER_CPCS;//0x01
		TC0->TC_CHANNEL[i].TC_RC = system_task_period[i];
		TC0->TC_CHANNEL[i].TC_CCR = 0;
		TC0->TC_CHANNEL[i].TC_CCR |= 0x5;
	}
	/* config the channel for system app */
	for( i = 0 ; i < 3 ; i ++ )
	{
		TC1->TC_CHANNEL[i].TC_CCR = (1<<1); //disable the counter
		TC1->TC_CHANNEL[i].TC_CMR = TC_CMR_CPCTRG | 4;  //compare enable , clock from 
		TC1->TC_CHANNEL[i].TC_IER = TC_IER_CPCS;//0x01
		TC1->TC_CHANNEL[i].TC_RC = user_task_period[i];//20ms
		TC1->TC_CHANNEL[i].TC_CCR = 0;
		TC1->TC_CHANNEL[i].TC_CCR = 0x5;  // enable logic task
	}
  /* return */
	return FS_OK;
}
/* tack config */
int task_config(void * p_arg , int argc)
{
	return FS_OK;
}
/* start system */
int system_start(int argc)
{
	/* table for IRQn CMSIS */
	unsigned char irqn[10] = { TC0_IRQn , TC1_IRQn , TC2_IRQn , TC3_IRQn , TC4_IRQn , 
	                           TC5_IRQn , TC6_IRQn , USBHS_IRQn , MCAN0_LINE1_IRQn , MCAN1_LINE1_IRQn};
	/* typedef */
	unsigned short argc_wide = argc & 0x7fff;
	/*-----------*/
	if( argc & 0x8000 )
	{
		if( (argc&0x7f) >= 10 )
		{
			 return FS_ERR;//not supply this format
		}
    /*----------------*/
    if( argc & 0x80 )
		{
			/* enable */
			NVIC_EnableIRQ((IRQn_Type)irqn[ argc & 0x7f ]);
		}else
    {
			/* disable */
			NVIC_DisableIRQ((IRQn_Type)irqn[ argc & 0x7f ]);
		}			
		/* return for */
		return FS_OK;
	}
	/* each day pl */
	for( int i = 0 ; i < 10 ; i ++ )
  { 
		 if( argc_wide & ( 1 << i ) )
		 {
			  /* enable the irq */
			  NVIC_EnableIRQ((IRQn_Type)irqn[i]);
		 }else
		 {
        NVIC_DisableIRQ((IRQn_Type)irqn[i]);
		 }
  }	
	/* return */
	return FS_OK;
}
/* disable all it */
int disable_all_it(int argc)
{
	for( int i = 0 ; i < 65 ; i ++ )
	{
		NVIC_DisableIRQ((IRQn_Type)i);
	}
	return FS_OK;
}

//		if( bin_data[i] == 0xADCF3568 && bin_data[i+3] == 0x887F651D )
//		{
//			vector = bin_data[i+1];
//			entire_addr = bin_data[i+2];
//			/*------------------------*/
/* ints */
void task0_handler(void)
{
	uint32_t dummy;
	dummy = TC0->TC_CHANNEL[0].TC_SR;
	(void)dummy;
	shell_timer_thread(0);
}
void task1_handler(void)
{
	uint32_t dummy;
	dummy = TC0->TC_CHANNEL[1].TC_SR;
	(void)dummy;
	shell_timer_thread(1);
}
void task2_handler(void)
{
	uint32_t dummy;
	dummy = TC0->TC_CHANNEL[2].TC_SR;
	(void)dummy;
	shell_timer_thread(2);
}
void task3_handler(void)
{
	uint32_t dummy;
	dummy = TC1->TC_CHANNEL[0].TC_SR;
	(void)dummy;
	shell_timer_thread(3);
}
void task4_handler(void)
{
	uint32_t dummy;
	dummy = TC1->TC_CHANNEL[1].TC_SR;
	(void)dummy;
	shell_timer_thread(4);
}
void task5_handler(void)
{
	uint32_t dummy;
	dummy = TC1->TC_CHANNEL[2].TC_SR;
	(void)dummy;
	shell_timer_thread(5);
}


























