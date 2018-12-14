/*
 * erase.c
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
#include "erase_key.h"
/* linker settings */
static unsigned char erase_settings;
/*--------------------------*/
struct file * file_log,*check;
void pic_thread(void);
/* fs inode system register */
FS_INODE_REGISTER("/erase_key.o",erase,erase_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(erase_settings);
/* heap init */
int erase_heap_init(void)
{
  /* full of zero */
	memset(&erase,0,sizeof(erase));
	/* shell base */
	erase.shell_i = shell_sched_getfiles();
	/* driver config */
	erase.config = erase_default_config;
	/* file interface */
	erase.flip.f_inode = &erase;
	erase.flip.f_path = "/erase_key.o";
	/* setting init */
	FS_SHELL_INIT(erase_settings,erase_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	erase_settings = 0;
	file_log = 0;
	/* add your own code here */
  erase.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
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
int erase_default_config(void)
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
	check = open("/system_check.d",__FS_OPEN_ALWAYS);
	/* judging */
	if( check == 0 )
	{
		printf_f("check open FAIL.break\r\n");
		return FS_ERR;		
	}
	printf_f("check open OK\r\n");
	/*------------------------*/
	if( shell_create_dynamic("erase_thread",erase_thread,5) != FS_OK )
	{
		printf_f("erase_thread create FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("erase_thread create OK\r\n");
	printf_f("erase...\r\n");
	/* return */
	return FS_OK;
}

/* thread */
void erase_thread(void)
{	
	unsigned char buffer[40];
	
	memset(buffer,0xff,sizeof(buffer));
	/* erase key */
	if( fs_ioctl(check,3,40,buffer) == FS_OK )
	{
		printf_f("erase ok\r\n");
	}else
	{
    printf_f("erase error\r\n");		
	}
	/* erase the head msg */
	unsigned int id_def[3] = {0xF1F2F3F4,0xE1E2E3E4,0xD1D2D3D4};
	/*--------------------*/
	memcpy((void *)(0x20408000+20 * 1024),id_def,sizeof(id_def));
	/* reset the system */
	shell_execute("system_reset",0);	
}



















