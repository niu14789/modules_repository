/*
 * radio.c
 *
 *  Created on: 2017?5?17?
 *      Author: niu
 */
#include "fs.h"
#include "fs_config.h"
#include "f_shell.h"
#include "string.h"
#include "f_drv.h"
#include "f_ops.h"
#include "stdio.h"
#include "radio.h"
/* linker settings */
static unsigned char radio_settings;
/*--------------------------*/
struct file * file_log,*gim;
static unsigned int cnt = 0;
static unsigned int delay_time = 0;
static unsigned int pic_cnt = 0;
void pic_thread(void);
/* fs inode system register */
FS_INODE_REGISTER("/pic_test.o",radio,radio_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(radio_settings);
/* heap init */
int radio_heap_init(void)
{
  /* full of zero */
	memset(&radio,0,sizeof(radio));
	/* shell base */
	radio.shell_i = shell_sched_getfiles();
	/* driver config */
	radio.config = radio_default_config;
	/* file interface */
	radio.flip.f_inode = &radio;
	radio.flip.f_path = "/pic_test.o";
	/* setting init */
	FS_SHELL_INIT(radio_settings,radio_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	radio_settings = 0;
	file_log = 0;
	cnt = 0;
	gim = 0;
	delay_time = 240;
	pic_cnt = 0;
	/* add your own code here */
  radio.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
/*- printf to file */
void printf_f(const char * p)
{
	if( file_log != 0 )
	{
		fs_write(file_log,p,strlen(p));
	}
}
/* default config */
int radio_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/take_pictures_%s.txt",p_path);
	/* open new file */
	file_log = open(buffer,__FS_CREATE_ALWAYS|__FS_WRITE);	
	/* create */
	if( file_log == 0 )
	{
		printf_f("file create FAIL\r\n");
		return FS_ERR;
	}
	/* create ok */
	printf_f("file create OK \r\n");
	/* open gimbal */
	gim = open("/gimbal.d",__FS_OPEN_ALWAYS);
	/* judging */
	if( gim == 0 )
	{
		printf_f("gimbal open FAIL.break\r\n");
		return FS_ERR;		
	}
	printf_f("gimbal open OK\r\n");
	/*------------------------*/
	if( shell_create_dynamic("pic_thread",pic_thread,5) != FS_OK )
	{
		printf_f("pic_thread create FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("pic_thread create OK\r\n");
	printf_f("taking...\r\n");
	/* return */
	return FS_OK;
}

/* thread */
void pic_thread(void)
{
	/*---------*/
	if( delay_time > 0 )
	{
		delay_time -- ;
		return;
	}
	/*------------*/
	if( cnt >= 4 )//2s
	{
		cnt = 0;
		/* take pic */
		fs_ioctl(gim,2,1,0);
		/* inc */
		pic_cnt ++;
		/*-----*/
		char buffer[32];
		/* clear */
		memset(buffer,0,sizeof(buffer));
		/*----------------------------*/
		sprintf(buffer,"PIC:%d\r\n",pic_cnt++);
		/* das */
		printf_f(buffer);		
	}
	/* incremeters */
	cnt++;
}



















