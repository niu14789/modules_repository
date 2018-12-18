/*
 * statistical.c
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
#include "statistical.h"

#define DIRECTOR   0 // 0 is SEND . 1 is RECEIVE

/* linker settings */
static unsigned char statistical_settings;
/*--------------------------*/
struct file * file_log,*gs,*bl;
void pic_thread(void);
/* define some param */
static unsigned int  peroid_ms[256];
static unsigned char save_buffer[128];
static unsigned int  pos = 0;
static unsigned int  rate = 0;
static unsigned short freq_ctrl = 0;
static unsigned int rate_temp = 0;
/* static stamps */
static unsigned int timestamps_sys_ms = 0;
/* fs inode system register */
FS_INODE_REGISTER("/statistical.o",statistical,statistical_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(statistical_settings);
/* heap init */
int statistical_heap_init(void)
{
  /* full of zero */
	memset(&statistical,0,sizeof(statistical));
	/* shell base */
	statistical.shell_i = shell_sched_getfiles();
	/* driver config */
	statistical.config = statistical_default_config;
	/* file interface */
	statistical.flip.f_inode = &statistical;
	statistical.flip.f_path = "/statistical.o";
	/* setting init */
	FS_SHELL_INIT(statistical_settings,statistical_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	statistical_settings = 0;
	timestamps_sys_ms = 0;
	rate = 0;
	rate_temp = 0;
	/* -- */
	memset(peroid_ms,0,sizeof(peroid_ms));
	memset(save_buffer,0,sizeof(save_buffer));
	pos = 0;
	/* add your own code here */
  statistical.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
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
int statistical_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
#if DIRECTOR == 0
	sprintf(buffer,"/storage/emmc/shell/statis_send_%s.txt",p_path);
#else
	sprintf(buffer,"/storage/emmc/shell/statis_rec_%s.txt",p_path);
#endif	
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
	/* open gs */
	gs = open("/ground.d",__FS_OPEN_ALWAYS);
	/* judging */
	if( gs == 0 )
	{
		printf_f("gs open FAIL.break\r\n");
		return FS_ERR;		
	}
	printf_f("gs open OK\r\n");
	/*------------------------*/
	fs_ioctl(gs,2,0,0);//ignore the key
	/* tips */
	printf_f("skip key\r\n");
#if 0
	/* set radio start work to 1 */
	fs_ioctl(gs,10,1,0);
	/* test mode */
	printf_f("start work to OK\r\n");
#endif
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
	/* insert call back */
	unsigned int fun = (unsigned int)gs_callback;
	/* register the callback */
#if DIRECTOR == 0	
	if( shell_write("__FS_gs_callback",&fun,4,0*4) != FS_OK )
#else
	if( shell_write("__FS_gs_callback",&fun,4,3*4) != FS_OK )
#endif	
	{
		printf_f("__FS_gs_callback inset FAIL. break\r\n");
		return FS_ERR;
		/*------------------------*/			
	}		
	/* OK */
	printf_f("__FS_gs_callback inset OK\r\n");	
	/* create time thread */
	if( shell_create_dynamic("time_stamp_ms",time_stamp_ms,0) != FS_OK )
	{
		printf_f("time_stamp_ms create FAIL . break\r\n");
		return FS_ERR;			
	}
	printf_f("time_stamp_ms create ok\r\n");
	/*------------------*/
	if( shell_create_dynamic("statistical_thread",statistical_thread,3) != FS_OK )
	{
		printf_f("statistical_thread create FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("statistical_thread create OK\r\n");
	printf_f("statisting...\r\n");
	/* return */
	return FS_OK;
}
/* create the time stamp with 1ms */
void time_stamp_ms(void)
{
	timestamps_sys_ms++;//ms
}
/* gs callback */
int gs_callback(int type,void * data,int len)
{
#if DIRECTOR == 0		
	unsigned char * p = data;
	/* get ID and len and save period */
	unsigned char ID = p[5];
#else
	unsigned char ID = type;
#endif	
	/* get period */
	if( peroid_ms[ID] != 0 )
	{
		unsigned int t = timestamps_sys_ms - peroid_ms[ID];
		/* create buffer */
		char buffer[128];
		memset(buffer,0,sizeof(buffer));
#if DIRECTOR == 0			
		len = len;
#else
		len += 8;
#endif
		sprintf(buffer,"ID:% 3d LEN:% 3d period:% 6dms RATE:%dByte/s\r\n",ID,len,t,rate);
		/* save into buffer */
		if( pos == 0 )
		{
			memset(save_buffer,0,sizeof(save_buffer));
			memcpy(save_buffer,buffer,strlen(buffer));
			pos = 1;
		}
	}
	/* transfer */
	peroid_ms[ID] = timestamps_sys_ms;	
	/* statis the send data */
#if DIRECTOR == 0	
	rate_temp += len;
#else
	rate_temp += len + 8;
#endif	
	return FS_OK;
}
/* thread */
void statistical_thread(void)
{	
	if( freq_ctrl++ >= 10 )
	{
		freq_ctrl = 0;
		rate = rate_temp;
		rate_temp = 0;
	}
	/*--------------------*/
	if( pos == 1 )
	{
		printf_f((const char *)save_buffer);
		pos = 0;
	}
}



















