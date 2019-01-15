/*
 * remove.c
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
#include "remove_check.h"
/* linker settings */
static unsigned char remove_check_settings;
/*--------------------------*/
struct file * file_log,*check;
void pic_thread(void);
/* fs inode system register */
FS_INODE_REGISTER("/remove_check.o",remove_check,remove_check_init,0);
/* fs shell def */
FS_SHELL_REGISTER(remove_check_settings);
/* heap init */
int remove_check_init(void)
{
  /* full of zero */
	memset(&remove_check,0,sizeof(remove_check));
	/* shell base */
	remove_check.shell_i = shell_sched_getfiles();
	/* driver config */
	remove_check.config = remove_check_config;
	/* file interface */
	remove_check.flip.f_inode = &remove_check;
	remove_check.flip.f_path = "/remove_check.o";
	/* setting init */
	FS_SHELL_INIT(remove_check_settings,remove_check_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	remove_check_settings = 0;
	/* add your own code here */
  remove_check.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
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
int remove_check_config(void)
{	
	char buffer[64];
	/*-----------------------------*/
	char * p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* merge path */
	sprintf(buffer,"/storage/emmc/shell/remove_check_%s.txt",p_path);
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
	/* delete check thread */
	/*------------------------*/
	if( shell_delete_dynamic("check_process",0xff) != FS_OK )
	{
		printf_f("delete check_process FAIL\r\n");
		return FS_ERR;		
	}
	/*-----------*/
	printf_f("delete check_process OK\r\n");
	/* return */
	return FS_OK;
}


















