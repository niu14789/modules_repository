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
struct file * file_log;
static unsigned int cnt = 0;
static unsigned int flag = 0;
/* fs inode system register */
FS_INODE_REGISTER("/test.o",radio,radio_heap_init,0);
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
	radio.flip.f_path = "/test.o";
	/* setting init */
	FS_SHELL_INIT(radio_settings,radio_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	radio_settings = 0;
	file_log = 0;
	cnt = 0;
	flag = 0;
	/* add your own code here */
  radio.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
/* default config */
int radio_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/template_%s.txt",p_path);
	/* open new file */
	file_log = open(buffer,__FS_CREATE_ALWAYS|__FS_WRITE);	
	/* create */
	if( file_log == 0 )
	{
     /* return */
		return FS_ERR;
	}
	/* write ok */
	fs_write(file_log,"file create ok\r\n",strlen("file create ok\r\n"));
	/* insert callback */
  unsigned int fun = (unsigned int)call_back;
  /* register the callback */	//vision_cb
  if( shell_write("__FS_gps_callback",&fun,4,1*4) != FS_OK )//can1
	{
		fs_write(file_log,"__FS_gps_callback 1 insert fail\r\n",strlen("__FS_gps_callback 1 insert fail\r\n"));
		return FS_ERR;
	}
	fs_write(file_log,"__FS_gps_callback 1 insert ok\r\n",strlen("__FS_gps_callback 1 insert ok\r\n"));
	/*---------------*/	
	if( shell_create_dynamic("template_thread",template_thread,3) != FS_OK )
	{
		fs_write(file_log,"template_thread create fail\r\n",strlen("template_thread create fail\r\n"));
		return FS_ERR;		
	}
	fs_write(file_log,"template_thread create ok\r\n",strlen("template_thread create ok\r\n"));
	/* return */
	return FS_OK;
}
/* call back */
int call_back(int type,void * data,int len)
{
	 /*--------------*/
	 flag = 1;
	 cnt = type;
	 /*--------------*/
	 return FS_OK;
}
/* thread */
void template_thread(void)
{
	if( flag == 1 )
	{
		flag = 0;
		
		char buffer[32];
		/* clear */
		memset(buffer,0,sizeof(buffer));
		/*----------------------------*/
		sprintf(buffer,"time:%d\r\n",cnt);
		/* das */
		fs_write(file_log,buffer,strlen(buffer));
	}
}



















