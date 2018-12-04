/* -------------header ----------*/
#include "fs.h"
#include "f_shell.h"
#include "fs_config.h"
#include "string.h"
#include "f_shell_cfg.h"
#include "stdio.h"
#include "f_ops.h"
#include "f_drv.h"
/* --------------- factor thread----------------*/
extern void printf_f(struct file * f,const char * p);
/* defines */
static struct file *algo;
static unsigned short factor_time = 0;
static unsigned char factory_once_flags = 0;
//static unsigned char calibration_mode = 0;//1
static struct shell_cmd * gs_mrs;
static unsigned short gs_cnt_pwm = 0;
//static unsigned char factory_snr_mode = 0;//2
//static unsigned char factory_optright_mode = 0;//3
static unsigned char snr_flag = 0;
static unsigned char optright_flag = 0;
static struct file * g;
/*----------------------------------------------*/
FS_CALLBACK_STATIC(aging_callback,1);
/* modules heap init */
void aging_heap_init(void)
{
	/* clear all of them */
	factor_time = 0;
	factory_once_flags = 0;
	gs_mrs = 0;
	gs_cnt_pwm = 0;
	snr_flag = 0;
	optright_flag = 0;
	algo = 0;
	/* init callback */
	FS_SHELL_INIT(__FS_aging_callback,__FS_aging_callback,0x040001,_CB_ARRAY_);
}
/* aging config */
void aging_config_default(void)
{
 	  /* open groung uart */
    g = open("/ground.d",__FS_OPEN_ALWAYS);
		/* judge if it's empty */
		if( g == 0 )
		{
			 printf_f(0," open g fail . break\r\n");
			 /* judging */
			 return;
		}
		/* ok */
		printf_f(0," open g ok\r\n");	
		/* open algo */
		algo = open("/algo.o",__FS_OPEN_ALWAYS);
		/* judging */
		if( algo == 0 )
		{
			 printf_f(0," open algo fail . break\r\n");
			 /* judging */
			 return;			
		}		
		/* ok */
		printf_f(0," open algo ok\r\n");		
		/* delete orignal task of motor */
		if( shell_delete_dynamic("vtask_5ms",0xff) != FS_OK )
		{
			 printf_f(0," delete vtask_5ms fail . break\r\n");
			 /* judging */
			 return;					
		}
		/* ok */		
		printf_f(0," delete vtask_5ms ok\r\n");
}
/* defaild */
static void gs_factory_cmd_ack(unsigned short cmd , unsigned char ok)
{
	unsigned short cmd_ack[2] = {cmd,ok};
	/* send */
	fs_ioctl(g,0,3 << 16 | MAVLINK_MSG_COMMAND_ACK , (unsigned char *)&cmd_ack);
}
/*---------*/
int gs_factory_pwm(struct file * f_p,unsigned short pwm)
{
	/* factory mode ? */
	if( !(factory_once_flags & 0x4) )
	{
		return FS_ERR;
	}
	/* set buffer */
	unsigned short pwm_b[4] = {pwm,pwm,pwm,pwm};
	/* output */
	fs_ioctl(f_p,2,0,pwm_b);
	/*------*/
	return FS_OK;
}
static void gs_factory_thread(void)
{
	if( factor_time > 0 )
	{
		/*------------*/
		factor_time--;
		/*---show---*/
		if( gs_cnt_pwm++ > 16 )//200ms
		{
			gs_cnt_pwm = 0;
			/* uploader rms */
			/* send */
			if( gs_mrs != NULL )
			{
				fs_ioctl(g,0,((gs_mrs->size>>16)*(gs_mrs->size&0xffff)) << 16 | MAVLINK_MSG_ID_RMP , (unsigned char *)gs_mrs->enter);
			}
	  }
	}else
	{
		/* exit factory */
		gs_factory_exit();
	}
}
/* gs_factory_mode */
int gs_factory_mode(unsigned short pwm,unsigned short min)
{
	int ret;
	/* create once */
	if( factory_once_flags & 0x1 )
	{
		return FS_ERR;//factory 
	}
	/* set the flags */
	factory_once_flags |= 0x7;
	/* -- */
  gs_factory_pwm(algo,pwm);
	/* set pwm time = min */
	factor_time = min * 83 * 60;
	/* read g_mrs */
	gs_mrs = shell_find("g_mrs");
	/* create the update task */
	ret = shell_create_dynamic("gs_factory_thread",gs_factory_thread,2);//12ms
	/* create ok ? */
	if( ret != FS_OK )
	{
		return ret;//create error
	}
  return FS_OK;
}
/* gs factory exit */
int gs_factory_exit(void)
{
	int ret;
	/* single */
	if( !(factory_once_flags & 0x2) )
	{
		return FS_ERR;
	}
	/* ------- */
	ret = shell_delete_dynamic("gs_factory_thread",0xff);
	/* delete ok ? */
	if( ret != FS_OK )
	{
		return FS_ERR;//create error
	}
	/* flags */
	factory_once_flags &=~ 0x6;
	/* ok */
	return FS_OK;
}
/* handle */
void gs_factory_handle(unsigned short cmd,unsigned char * data)
{
	float tmp[3];
	/*---------*/
	memcpy(tmp,data,12);
	/* len matck ? */
	switch(cmd)
	{
		case MAVLINK_CMD_FACTORY_CMD_1://enter
				/*--------------------------*/
				if( gs_factory_mode((unsigned short)tmp[1],(unsigned short)tmp[2]) == FS_OK )
				{
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_1,0);//ok	
				}else
				{
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_1,1);//ok
				}
				/*----------------*/
			break;
		case MAVLINK_CMD_FACTORY_CMD_2://exit	
				/* do */
				if( gs_factory_exit() == FS_OK )
				{
					/* get correct len */
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_2,0);//ok
				}else
				{
					/* get correct len */
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_2,1);//ok					
				}
			break;
		case MAVLINK_CMD_FACTORY_CMD_3://real time
				/* do */
		    if( gs_factory_pwm(algo,(unsigned short)tmp[1]) == FS_OK )
				{
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_3,0);//ok
				}else
				{
					gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_3,1);//error
				}
				/*----------------*/
			break;
		case MAVLINK_CMD_FACTORY_CMD_4://motor
				/* get correct len */
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_4,0xff);//ok ack
				/* do */
				//--------------
		    FS_CALLBACK_INSERT(aging_callback,0,0,1,0);
				/*----------------*/
			break;
		case MAVLINK_CMD_FACTORY_CMD_5://mag
				/* get correct len */
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_5,0xff);//ok ack
				/* do */
				//--------------
		    FS_CALLBACK_INSERT(aging_callback,0,0,2,0);
				/*----------------*/			
			break;
		case MAVLINK_CMD_FACTORY_CMD_6://hover
				/* get correct len */
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_6,0xff);//ok ack
				/* do */
				//--------------
		    FS_CALLBACK_INSERT(aging_callback,0,0,3,0);
				/*----------------*/			
			break;		
		case MAVLINK_CMD_FACTORY_CMD_7://snr
			/* check mode */
		  if( snr_flag == 0 )
			{
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_7,0xff);//ok ack
				/* -------- set flag ----------- */
				snr_flag = 1;
				/* enter the mode */
				FS_CALLBACK_INSERT(aging_callback,1,0,1,0);
			}else
			{
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_7,0xfe); // 
			}
			break;
		case MAVLINK_CMD_FACTORY_CMD_8://OptRight
			/* check mode */
		  if( optright_flag == 0 )
			{
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_8,0xff);//ok ack
				/* -------- set flag ----------- */
				optright_flag = 1;
				/* enter the mode */
				FS_CALLBACK_INSERT(aging_callback,2,0,1,0);
			}else
			{
				gs_factory_cmd_ack(MAVLINK_CMD_FACTORY_CMD_8,0xfe); // 
			}			
			break;
		default:
			gs_factory_cmd_ack(cmd,1);//error
			break;
	}
}








