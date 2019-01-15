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
/* decalreat */
struct shell_cmd * dynamic_find(const char * shell_cmd , unsigned char * index);
/* linker settings */
static unsigned char radio_settings;
/* fs inode system register */
FS_INODE_REGISTER("/radio.o",radio,radio_heap_init,0);
/* fs shell def */
FS_SHELL_REGISTER(radio_settings);
/*run cmd*/
static unsigned int send_read_cmd;
/*time cnt*/
static unsigned int TimeCnt;
/* radio version */
static unsigned char * radio_ver_head;
/*---------------*/
static unsigned short radio_ver;
/* radio firmware version */
static unsigned short * radio_firmware_ver;
/* firmware postion and size */
static unsigned int fw_size,fw_offset;
/* uart_config */
static unsigned char * uart_config;
/* close cnt */
static unsigned short task_close_cnt;
/*- interface -*/
static struct file * radio_uart5 , * file_log;
/*---- log name ----*/
static char * p_path;
/*---- default path ----*/
static char ** fu_path;
/*----------------------*/
int (*fm_creater)(unsigned char ,unsigned char ,unsigned char ,unsigned char *);
/*----------------------*/
void (*can_data_pop_task)(void);
/*----------------------*/
static unsigned char step_head2,payload_len,msg_id,msg;
/* read buffer */
unsigned char fw_read_buffer[2048];
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
	radio.flip.f_path = "/radio.o";
	/* setting init */
	FS_SHELL_INIT(radio_settings,radio_settings,0x010000 + 1 , _CB_VAR_);
	/* clear */
	radio_settings = 0;
	radio_ver_head = 0;
	radio_firmware_ver = 0;
	uart_config = 0;
	radio_ver = 0;
	fw_size = 0;
	fw_offset = 0;
	task_close_cnt = 0;
	radio_uart5 = 0;
	file_log = 0;
	p_path = 0;
	fu_path = 0;
	fm_creater = 0;
	can_data_pop_task = 0;
	step_head2 = 0;
	payload_len = 0;
	msg_id = 0;
	send_read_cmd = 1;
	TimeCnt = 0;
	msg = 0xff;
	/* add your own code here */
  radio.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
/* default config */
int radio_default_config(void)
{
	struct shell_cmd * p_shell,*pp_shell;
	/* find the radio firmware version */
	p_shell = shell_find("modules_status_s");
	/*------------------------*/
	pp_shell = shell_find("fu_path");
	/*------------------------*/
	fu_path = pp_shell->enter;
	/*------------------------*/
	if( pp_shell != NULL && p_shell != NULL && (int)(*fu_path) != FS_ERR )
	{
		/*--------------------------------*/
		radio_firmware_ver = (unsigned short *)((unsigned int)p_shell->enter + 28*21 + 6 );	
		/*--------------------------------*/
		if( *radio_firmware_ver > 200 )
		{
			fw_size = radio_firmware_ver[6] << 16 | radio_firmware_ver[5];
			fw_offset = radio_firmware_ver[10] << 16 | radio_firmware_ver[9];
			/*---------------------------*/
		}
	}
	/* create always */
	shell_create_dynamic("radio_thread",radio_thread,5);
	/*---------------------------*/	
	/* find the radio version */
	p_shell = shell_find("radio_param");
	/*------------------------*/
	if( p_shell != NULL )
	{
	  radio_ver_head = p_shell->enter;
	}	
	/* open the uart config page */
	p_shell = shell_find("uart_config");
	/*-----ok?------*/
	if( p_shell != NULL )
	{
	  uart_config = p_shell->enter;
	}	
	/* find creater name */
	p_shell = shell_find("ageement_combination_direct");
	/*-----ok?------*/
	if( p_shell != NULL )
	{
		fm_creater = (int (*)(unsigned char ,unsigned char ,unsigned char ,unsigned char *))p_shell->enter;
	}		
	/* find dynamic exe */
	unsigned char index;
	/* find dynamic exe */
	p_shell = dynamic_find("can_data_pop_task",&index);
	/*-----ok?------*/
	if( p_shell != NULL )
	{
		can_data_pop_task = (void (*)(void))p_shell->enter;
	}		
	/*-----------------------------*/
	radio_uart5 = open("/UART/5",__FS_OPEN_ALWAYS);
	/*-----------------------------*/
	p_path = (char *)shell_find("log_num_exprot")->enter;
	/*-----------------------------*/
	/* return */
	return FS_OK;
}
/* thread */
void radio_thread(void)
{
	/*--------------------*/
	if( radio_ver_head[26] != 0 && radio_ver_head[28] != 0 && radio_ver_head[30] != 0 )
	{
		radio_ver = ( radio_ver_head[26] - '0' ) * 100 + ( radio_ver_head[28] - '0' ) * 10 + ( radio_ver_head[30] - '0' );
		#if 0
		/* one or two */
		if( radio_ver >= 200 )
		{
			radio_ver = 205;
		}
		#endif
	}
	/* need update ? */
	if( radio_ver != 0 )
	{
		/*-------------------------------*/
		char buffer[64];
		char rbp[64];
		/* full with 0 */
		memset(buffer,0,sizeof(buffer));
		memset(rbp,0,sizeof(rbp));
		/* open new file */
		file_log = open("/storage/emmc/system/radio.rv",__FS_CREATE_ALWAYS|__FS_WRITE);		
		/* get and transfer data */
		memcpy(buffer,radio_ver_head,16);
		memcpy(buffer+32,radio_ver_head+16,16);
		/* create buffer */
		sprintf(rbp,"devId:%s\r\nFWver:%s\r\n",buffer,buffer+32);
		/*-------------------------------------*/
		fs_write(file_log,rbp,strlen(rbp));			
		/*-------------------------------------*/		
		/* not need */
		shell_delete_dynamic("radio_thread",0xff);
		/*---------------------------------------*/
		if( fw_size && (( radio_ver == 200 ) || ( radio_ver > 200 && radio_ver != *radio_firmware_ver )))
		{
      /*-------------------------------*/
			radio_ready();
			/*-------------------------------*/
		}else/*not update ==*/
		{
			uart_config[5] &=~ 0x40;
			/*start radio*/
			fs_write(radio_uart5,(const char *)RADIO_START_WORK,sizeof(RADIO_START_WORK));
			fs_write(radio_uart5,(const char *)RADIO_START_WORK,sizeof(RADIO_START_WORK));
			fs_write(radio_uart5,(const char *)RADIO_START_WORK,sizeof(RADIO_START_WORK));
			fs_write(radio_uart5,(const char *)RADIO_START_WORK,sizeof(RADIO_START_WORK));
			fs_write(radio_uart5,(const char *)RADIO_START_WORK,sizeof(RADIO_START_WORK));
			/* change radio to dma mode */
			uart_config[5] |= 0x40;
		}
		/*---------------------------------*/
	}else
	{
		task_close_cnt++;
		/* timeout */
		if( task_close_cnt >= 80 )//about 80s
		{
			task_close_cnt = 0;
			/* close thread */
			shell_delete_dynamic("radio_thread",0xff);
		}
	}
}

/*-----------------------*/
static void radio_ready(void)
{
	shell_delete_dynamic("usb_check",0xff);
	/* display all task */
	shell_execute("system_start",1<<5);
	/* bootloader app or need update */
	uart_config[5] &=~ 0x40;//single mode
	/*-------------------------------*/
	char buffer[64];
	/* merge path */
	sprintf(buffer,"/storage/emmc/system/radio_%s.txt",p_path);
	/* open new file */
	file_log = open(buffer,__FS_CREATE_ALWAYS|__FS_WRITE);		
	/*---------------------*/
	sprintf(buffer,"radio from %d to %d start...\r\n",radio_ver,*radio_firmware_ver);
	/*-------------------------------------*/
	fs_write(file_log,buffer,strlen(buffer));			
	/*-------------------------------------*/
	fs_write(radio_uart5,(const char *)RADIO_READY,sizeof(RADIO_READY));
	/* notify to ground and led control */
	shell_execute("bl_led_status",0x30);//green fast
	//create log file and start 
	shell_create_dynamic("radio_upgrade",radio_upgrade,5);	
	/*-------------------------------------*/
}

void timeout_process(void)
{	
	if(send_read_cmd == 1)
	{
		if(TimeCnt >= 100)
		{
			fs_write(file_log,"timeout 50s,send fw\r\n",22);
			if( radio_firmware_send() != FS_OK )
			{
				fs_write(file_log,"timeout 50s,send fw failed\r\n",28);
				/* reset system */
				shell_execute("system_reset",0x00);	
			}
			else
			{
				/*send firmware ok,start timeout*/
				TimeCnt = 0;
				send_read_cmd = 2;
			}
		}
		else
		{
			TimeCnt ++;
		}
	}else if(send_read_cmd == 2 )
	{
		/*timeout 5s*/
		if(TimeCnt >= 10)
		{
			fs_write(file_log,"timeout 5s\r\n",13);
			/*reset system*/
			shell_execute("system_reset",0x00);	
		}else
		{
			TimeCnt ++;
		}
	}
}
/*-------------------*/
void radio_upgrade(void)
{
	unsigned char buffer[128];
	/* read data */
	int len = fs_read(radio_uart5,(char *)buffer,sizeof(buffer));
	/* process */
	radio_process(buffer,len);
	/*timeout process*/
	timeout_process();
	/*----------------*/
	can_data_pop_task();
}
/*--------------------*/
static void radio_process(unsigned char * buffer , unsigned int len )
{
	for( int i = 0 ; i < len ; i ++ )
	{
		frame_prase(buffer[i]);
	}
}
/* case : simple type */
static void frame_prase(unsigned char c)
{
	/* case */
	switch(step_head2)
	{
		case 0:
			 if(c == 0x46)
			 {
				 step_head2 = 1;
			 }
			 else
			 {
				 step_head2 = 0;
			 }
			 break;
	  case 1:
			 if(c == 0x4d)
			 {
				 step_head2 = 2;
			 }
			 else
			 {
				 if(c == 0x46)
				 {
					 step_head2 = 1;
				 }else
				 {
					 step_head2 = 0;
				 }
			 }
		   break;
		case 2:
			 if((c == 0xFF)||(c == 0x7E)) // rd -> ap
			 {
				 step_head2 = 3;
			 }
			 else
			 {
				 if(c == 0x46)
				 {
					 step_head2 = 1;
				 }else
				 {
					 step_head2 = 0;
				 }
			 }
			 break;
		case 3:
			 payload_len = c;
			 step_head2 = 4;//payload length
			 break;
		case 4:
			 if( c == 0x01 )
			 {
				 step_head2 = 5;
			 }else
			 {
					if(c == 0x46)
				  {
					  step_head2 = 1;
				  }else
				  {
					  step_head2 = 0;
				  }			 
			 }
			 break;
		case 5:
			 msg_id = c;
			 step_head2 = 6;//msg id
			 break;
		case 6:
			 if( payload_len == 0 )
			 {
				 if( c == 0xAA )
				 {
					 step_head2 = 7;
					 /* ---------- */
				 }else
				 {
					 step_head2 = 0;
				 }
			 }else
			 {
				 msg = c;
				 payload_len --;
			 }
			 break;
		case 7:
			 if( c == 0x55 )
			 {
				 /* get data */
				 radio_result(msg_id,msg);
				 /*----------*/
			 }
			 /*----------*/
			 step_head2 = 0;
			 /*----------*/
		default :
			 step_head2 = 0;
			 break;
	}
}
/* radio receive process*/
void radio_result(unsigned char id,unsigned char msg)
{
	/* send */
	switch(id)
	{
		case 0x11://has already ok.send firmware
			fs_write(file_log,"radio ready ok,sending firmware\r\n",33);		
		  /* send ok ? */
			if( radio_firmware_send() != FS_OK )
			{
				/* reset system */
				shell_execute("system_reset",0);
				/*---------------------------------*/			
			}
			TimeCnt = 0;
			send_read_cmd = 2;
		break;
		case 0x10://update result
			if( msg == 0 )
			{
				/* ok */
				fs_write(file_log,"radio update ok\r\n",17);			
			}else
			{
				fs_write(file_log,"radio update fail\r\n",19);	
			}
			/* reset system */
			shell_execute("system_reset",0);
			/*---------------------------------*/
		break;
	}
}
/* radio firmware send */
static int radio_firmware_send(void)
{
	int len;
	/* open firmware */
	struct file * radio_fw_file = open(*fu_path,__FS_OPEN_EXISTING|__FS_READ);
	/* open ok ? */
	if( radio_fw_file == NULL )
	{
		/* open fw file fail */
		fs_write(file_log,"open fw file fail\r\n",19);
    /*-------------------*/		
		return FS_ERR;
	}
	/* open ok */
	fs_seek(radio_fw_file,fw_offset,0);
	/*-------------------------------*/
	while(1)
	{
		int real_len = fw_size > sizeof(fw_read_buffer) ? sizeof(fw_read_buffer) : fw_size;
		/* read file */
		len = fs_read(radio_fw_file,(char *)fw_read_buffer,real_len);
		/*-----------*/
		if( len != real_len )
		{
			/* read fail */
			fs_write(file_log,"read fw file fail\r\n",19);
			/* return fail */
			return FS_ERR;
		}
		/*---------------------------------------------------------------*/
    fs_write(radio_uart5,(const char *)fw_read_buffer,len);
		/*---------------------*/
		fw_size -= len;
		/*---------------------*/
		if( fw_size == 0 )
		{
			/* close file */
			fs_close(radio_fw_file);
			/*-------------------*/
			fs_write(file_log,"write fw file ok\r\n",18);
			/* return ok */
			return FS_OK;
		}
	}
}





































