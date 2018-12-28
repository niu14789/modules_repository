/*
 * version.c
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
#include "version.h"

/* linker settings */
static unsigned char version_settings;
/*--------------------------*/
struct file * file_log,*bl;
/* define some param */
static unsigned short freq_ctrl = 0;
static char version_buffer[512];
modules_status *m;
/* fs inode system register */
FS_INODE_REGISTER("/version.o",version,version_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(version_settings);
/* heap init */
int version_heap_init(void)
{
  /* full of zero */
	memset(&version,0,sizeof(version));
	/* shell base */
	version.shell_i = shell_sched_getfiles();
	/* driver config */
	version.config = version_default_config;
	/* file interface */
	version.flip.f_inode = &version;
	version.flip.f_path = "/version.o";
	/* setting init */
	FS_SHELL_INIT(version_settings,version_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	version_settings = 0;
	freq_ctrl = 0;
	memset(version_buffer,0,sizeof(version_buffer));
	/* add your own code here */
  version.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
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
int version_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/version_%s.txt",p_path);	
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
	bl = open("/bootloader.o",__FS_OPEN_ALWAYS);
	/* judging */
	if( bl == 0 )
	{
		printf_f("bl open FAIL.break\r\n");
		return FS_ERR;		
	}
	printf_f("bl open OK\r\n");
	/*------------------------*/	
	unsigned short version = fs_ioctl(bl,3,0,0);
	/* create version message */
	sprintf(buffer,"FC version is %d\r\n",version);
	/* write into file */
	printf_f(buffer);	
  /* find moudles */
	 /* open */
	struct shell_cmd * p = shell_find("modules_status_s");
	/* OK ? */
	if( p == NULL )
	{
	  printf_f("find modules status fail.break\r\n");
		return FS_ERR;
	}
	printf_f("find modules status OK\r\n");
	/* ok */
	m = (modules_status *)p->enter;	
	/* create */
	if( shell_create_dynamic("version_thread",version_thread,3) != FS_OK )	//100ms
	{
		printf_f("version_thread create FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("version_thread create OK\r\n");
	printf_f("...\r\n");
	/* return */
	return FS_OK;
}
/* thread */
void version_thread(void)
{	
	if( freq_ctrl++ < 10*60 )//60S
	{
		return;
	}
	/* after 1 mins */
	for( int i = 0 ; i < 24 ; i ++ )
	{
		int tl = strlen(version_buffer);
		/* create version buffer */
		sprintf(version_buffer+tl,"%s:%d ID:0x%X\r\n",m->id,m->version_m,m->dev_id);
		/* incremeter */
		m++;
	}
	/* write into files */
	printf_f(version_buffer);
	/* delete the task and reset */
	shell_delete_dynamic("version_thread",0xff);
	/* reset for notice */
	shell_execute("system_reset",0);	
}



















