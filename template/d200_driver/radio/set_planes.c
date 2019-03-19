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
#include "set_planes.h"
/* plane types */
#define PLANE_V100     (100)
#define PLANE_V200     (200)
#define PLANE_V300     (300)
/* linker settings */
static unsigned char set_planes_settings;
/*--------------------------*/
struct file * file_log,*check;
/* fs inode system register */
FS_INODE_REGISTER("/set_planes.o",setplane,setplanes_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(set_planes_settings);
/* heap init */
int setplanes_heap_init(void)
{
  /* full of zero */
	memset(&setplane,0,sizeof(setplane));
	/* shell base */
	setplane.shell_i = shell_sched_getfiles();
	/* driver config */
	setplane.config = setplanes_default_config;
	/* file interface */
	setplane.flip.f_inode = &setplane;
	setplane.flip.f_path = "/set_planes.o";
	/* setting init */
	FS_SHELL_INIT(set_planes_settings,set_planes_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	set_planes_settings = 0;
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
int setplanes_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/set_planes_%s.txt",p_path);
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
	if( shell_create_dynamic("setplanes_thread",setplanes_thread,5) != FS_OK )
	{
		printf_f("setplanes_thread create FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("setplanes_thread create OK\r\n");
	printf_f("running...\r\n");
	/* return */
	return FS_OK;
}

/* thread */
void setplanes_thread(void)
{	
	unsigned short type_plane = PLANE_V300;
	/* erase key */
	if( fs_ioctl(check,14,2,&type_plane) == FS_OK )
	{
		if( type_plane == fs_ioctl(check,15,0,0) )
		{
			char buffer[32];
			/* transfer to string */
			sprintf(buffer,"Set up the plane to V%d successfully\r\n",type_plane);
			/* write into file */
			printf_f(buffer);
		}
		else
		{
      printf_f("set up error 1\r\n");			
		}
	}
	else
	{
    printf_f("set up error\r\n");		
	}
	/* erase the head msg */
	unsigned int id_def[3] = {0xF1F2F3F4,0xE1E2E3E4,0xD1D2D3D4};
	/*--------------------*/
	memcpy((void *)(0x20408000+20 * 1024),id_def,sizeof(id_def));
	/* reset the system */
	shell_execute("system_reset",0);	
}



















