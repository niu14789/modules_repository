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
/* linker settings */
static unsigned char read_sn_key_settings;
/*--------------------------*/
/* declares */
int read_sn_key_default_config(void);
int read_sn_key_heap_init(void);
/*--------------------------*/
struct file * file_log,*check;
/* fs inode system register */
FS_INODE_REGISTER("/read_sn_key.o",setplane,read_sn_key_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(read_sn_key_settings);
/* heap init */
int read_sn_key_heap_init(void)
{
  /* full of zero */
	memset(&setplane,0,sizeof(setplane));
	/* shell base */
	setplane.shell_i = shell_sched_getfiles();
	/* driver config */
	setplane.config = read_sn_key_default_config;
	/* file interface */
	setplane.flip.f_inode = &setplane;
	setplane.flip.f_path = "/read_sn_key.o";
	/* setting init */
	FS_SHELL_INIT(read_sn_key_settings,read_sn_key_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	read_sn_key_settings = 0;
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
int read_sn_key_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/read_sn_key_%s.txt",p_path);
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
	/* copy data */
	memset(buffer,0,sizeof(buffer));
	/*------------------------*/
	if( fs_ioctl(check,10,32,buffer) != FS_OK )
	{
		printf_f("read sn error\r\n");
		return FS_ERR;
	}
	/*------------------------*/
	printf_f("SN : ");
	printf_f(buffer);
	printf_f("\r\n");
	/* copy data */
	memset(buffer,0,sizeof(buffer));
	/*------------------------*/
	if( fs_ioctl(check,2,40,buffer) != FS_OK )
	{
		printf_f("read key error\r\n");
		return FS_ERR;
	}
	/* read */
	char cbuf[16];
	/* read head */
	printf_f("key : ");
	/* transfer */
	for( unsigned int i = 0 ; i < 40 ; i ++ )
	{	
		/* copy data */
		memset(cbuf,0,sizeof(cbuf));
		/* merge path */
	  sprintf(cbuf,"%02X",buffer[i]);
		/* write */
		printf_f(cbuf);
		/*--------*/
	}
	/* ok */
	printf_f("\r\n");
	/**/
	/* return */
	return FS_OK;
}



















