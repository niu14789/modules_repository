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
/* plane types */
#define PLANE_PROPELLER_OLD     (0x3514)
#define PLANE_PROPELLER_NEW     (0x7825)
/* linker settings */
static unsigned char set_propeller_settings;
/*--------------------------*/
/* declares */
int set_propeller_default_config(void);
int set_propeller_heap_init(void);
/*--------------------------*/
struct file * file_log,*check;
/* fs inode system register */
FS_INODE_REGISTER("/set_propeller.o",setplane,set_propeller_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(set_propeller_settings);
/* heap init */
int set_propeller_heap_init(void)
{
  /* full of zero */
	memset(&setplane,0,sizeof(setplane));
	/* shell base */
	setplane.shell_i = shell_sched_getfiles();
	/* driver config */
	setplane.config = set_propeller_default_config;
	/* file interface */
	setplane.flip.f_inode = &setplane;
	setplane.flip.f_path = "/set_propeller.o";
	/* setting init */
	FS_SHELL_INIT(set_propeller_settings,set_propeller_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	set_propeller_settings = 0;
	file_log = 0;
	/* add your own code here */
  setplane.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
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
int set_propeller_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/set_propeller_%s.txt",p_path);
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
	unsigned short type = PLANE_PROPELLER_NEW;
	/*------------------------*/
	if( fs_ioctl(check,16,2,&type) == FS_OK )
	{
		/* double */
		if( fs_ioctl(check,17,0,0) == type )
		{
		    printf_f("set 3 propellers OK\r\n");
			  /* break */
			  return FS_OK;
		}
	}
	/* set 2 propelloers error */
	printf_f("set 3 propelloers error\r\n");
	/* return */
	return FS_OK;
}



















