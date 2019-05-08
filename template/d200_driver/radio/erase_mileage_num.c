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
/* declare */
int erase_mn_heap_init(void);
int erase_mn_default_config(void);
/* private Macros */
/* flash backup addr */
#define FLASH_BACKUP     ( 0xE0000 )
#define KEY_ECP_SIZE     ( 42 )          
#define FLASH_BACKUP_SN  ( FLASH_BACKUP + KEY_ECP_SIZE )
#define SN_NUM_SIZE      ( 34 )
#define FLASH_MILEAGE    ( FLASH_BACKUP_SN + SN_NUM_SIZE )
#define MILEAGE_SIZE     ( 6 )
#define FLASH_FLIGHT_NUM ( FLASH_MILEAGE + MILEAGE_SIZE )
#define FLIGHT_SIZE      ( 6 )
#define FLASH_CALIBRATE  ( FLASH_FLIGHT_NUM + FLIGHT_SIZE )
#define FLASH_CALI_SIZE  ( 16 )
#define FLASH_PLANE_TYPE ( FLASH_CALIBRATE + FLASH_CALI_SIZE )
#define FLASH_PLANE_SIZE ( 4 )
#define FLASH_OPS_BASE   ( FLASH_BACKUP + KEY_ECP_SIZE + SN_NUM_SIZE + 420 )
/* linker settings */
static unsigned char erase_mn_settings;
/*--------------------------*/
struct file * file_log,*check;
/* fs inode system register */
FS_INODE_REGISTER("/set_planes.o",setplane,erase_mn_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(erase_mn_settings);
/* heap init */
int erase_mn_heap_init(void)
{
  /* full of zero */
	memset(&setplane,0,sizeof(setplane));
	/* shell base */
	setplane.shell_i = shell_sched_getfiles();
	/* driver config */
	setplane.config = erase_mn_default_config;
	/* file interface */
	setplane.flip.f_inode = &setplane;
	setplane.flip.f_path = "/set_planes.o";
	/* setting init */
	FS_SHELL_INIT(erase_mn_settings,erase_mn_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	erase_mn_settings = 0;
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
int erase_mn_default_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/erase_mileage_num_%s.txt",p_path);
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
	unsigned char dat[4] = { 0 , 0 , 0 ,0 };
	unsigned int buf[2] = { (unsigned int)dat , 4 };
	//do something
	if( fs_ioctl(check,1,FLASH_MILEAGE,buf) == FS_OK )
	{
		printf_f("Reset mileage OK\r\n");
	}
	else
	{
		printf_f("Reset mileage FAIL\r\n");
	}
	/* flight counter */
	if( fs_ioctl(check,1,FLASH_FLIGHT_NUM,buf) == FS_OK )
	{
		printf_f("Reset flight counter OK\r\n");
	}	
	else
	{
		printf_f("Reset flight counter FAIL\r\n");
	}
	/* return */
	return FS_OK;
}



















