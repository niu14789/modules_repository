

#include "fs.h"
#include "f_shell.h"
#include "fs_config.h"
#include "string.h"
#include "f_shell_cfg.h"
#include "stdio.h"
#include "f_ops.h"

#define UART0_ENABLE    (0)

#if UART0_ENABLE
/* static file * uart0 */
static struct file * file_debug;
#endif
/* declare */
static unsigned short version_unit[24];
static unsigned short version_v100[24];
static unsigned short version_vion[12];
/* led status */
static unsigned char led_status;
/* calibration status */
static unsigned char cali_status;
/*----------------------------------*/
static LINE_CONFIG_DEF line_cmd;
/*----------------------------------*/
static unsigned char wave_flag = 0;
static unsigned short package_freq = 0;
static unsigned char wave_buffer[64];
/*--------------------------------*/
static unsigned char factory_flags = 0xff;
static unsigned char factory_buffer[16];
/*----------------------------------*/
static struct file * ground_uart5 , *file_log , *bl , *ground;
static char buf_receive[64];
/*----------------------------------*/
void printf_f(struct file * f,const char * p);
/*----------------------------------*/
extern unsigned char D_or_v;
/*----------------------------------*/
/* define the inode */
FS_INODE_REGISTER("/factory.o",shell,shell_heap_init,0);
/*--- some define ---*/
FS_SHELL_REGISTER(version_unit);
/* v100 versions */
FS_SHELL_REGISTER(version_v100);
/* other version */
FS_SHELL_REGISTER(version_vion);
/*----------------------------------*/
FS_SHELL_REGISTER(led_status);
/*----------------------------------*/
FS_SHELL_REGISTER(cali_status);
/*----------------------------------*/
#if UART0_ENABLE
/* fs thread */
FS_SHELL_STATIC(shell_thread,shell_thread,4,_CB_TIMER_|_CB_IT_IRQN_(TASK_PERIOD5_ID));//12ms
#endif
/* heap init */
static int shell_heap_init(void)
{
		 /* full of zero */
		memset(&shell,0,sizeof(shell));
		/* shell base */
		shell.shell_i = shell_sched_getfiles();
		/* driver config */
		shell.config = shell_default_config;
		/* file interface */
		shell.flip.f_inode = &shell;
		shell.flip.f_path = "/factory.o";
	  /* shell */
	  FS_SHELL_INIT(version_unit,version_unit,0x020000+24,_CB_ARRAY_);
	  FS_SHELL_INIT(version_v100,version_v100,0x020000+24,_CB_ARRAY_);
	  FS_SHELL_INIT(version_vion,version_vion,0x020000+12,_CB_ARRAY_);
	  FS_SHELL_INIT(led_status,led_status,0x010001,_CB_VAR_);
	  FS_SHELL_INIT(cali_status,cali_status,0x010001,_CB_VAR_);
		/* heap */
		shell.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
		/* add your own code here */
		memset(version_unit,0,sizeof(version_unit));
	  memset(version_v100,0,sizeof(version_v100));
	  memset(version_vion,0,sizeof(version_vion));
		led_status = 0;
		cali_status = 0;
		memset(&line_cmd,0,sizeof(line_cmd));
		wave_flag = 0;
		package_freq = 0;
		memset(wave_buffer,0,sizeof(wave_buffer));
		factory_flags = 0xff;
		memset(factory_buffer,0,sizeof(factory_buffer));
		ground_uart5 = 0;
		file_log = 0;
		memset(buf_receive,0,sizeof(buf_receive));
		bl = 0;
		ground = 0;
		/* clear */
		aging_heap_init();
		/* ------end of file------ */
		return FS_OK;
}
/* fdg */
static int shell_default_config(void)
{
#if UART0_ENABLE	
	   /* open  */
    file_debug = open("/UART/0",__FS_OPEN_ALWAYS);
    /* if ok ?*/
    if( file_debug == NULL )
    {
        /* open fail */
        shell.i_flags = __FS_IS_INODE_FAIL;
        return FS_ERR;
    }
    /* start config the usart1(id:6) for sbus */
    uart_config_msg msg;
    /* dat */
    msg.mode = 0x20C0;//for sbus , parity:EVEN,stopBit 2 , data :8bit  
    msg.baudrate = 115200;
    msg.tx_mode = _UART_TX_NARMOL;//disable tx
    msg.rx_mode = _UART_RX_DMA;
    msg.rx_dma_deepth = 16;
    msg.index = 0;//for usart 1 ID->6
    msg.rx_dma_buffer = NULL;//frame_data;
    /* start config */
    drv_config(file_debug,&msg,sizeof(msg));
		/* printf somethings */
		fs_write(file_debug,"enter shell?press >enter key< within 2s\r\n",sizeof("enter shell?press >enter key< within 2s\r\n"));
#endif
    /* create a initial file */
		char buffer[64];
		/*-----------------------------*/
		char * p_path = (char *)shell_find("log_num_exprot")->enter;
		/*-----------------------------*/
		/* merge path */
		sprintf(buffer,"/storage/emmc/shell/factory_%s.txt",p_path);
		/* open new file */
		file_log = open(buffer,__FS_CREATE_ALWAYS|__FS_WRITE);	
		/* create */
		printf_f(file_log,"factory create ok\r\n");
		/* open gs uart */
    ground_uart5 = open("/UART/5",__FS_OPEN_ALWAYS);
		/* judge if it's empty */
		if( ground_uart5 == 0 )
		{
			printf_f(file_log,"open uart5 fail . break\r\n");
			return FS_ERR;
		}
		/* ok */
		printf_f(file_log,"open uart5 ok . break\r\n");
		/* open bl uart */
    bl = open("/bootloader.o",__FS_OPEN_ALWAYS);
		/* judge if it's empty */
		if( bl == 0 )
		{
			printf_f(file_log,"open bl fail . break\r\n");
			return FS_ERR;
		}
		/* ok */
		printf_f(file_log,"open bl ok . break\r\n");		
	  /* open groung uart */
    ground = open("/ground.d",__FS_OPEN_ALWAYS);
		/* judge if it's empty */
		if( ground == 0 )
		{
			printf_f(file_log,"open ground fail . break\r\n");
			return FS_ERR;
		}
		/* ok */
		printf_f(file_log,"open ground ok . break\r\n");	
    /* insert the callback */
    unsigned int fun = (unsigned int)factory_callback;
    /* insert */
    if( shell_write("__FS_gs_callback",&fun,4,3*4) == FS_ERR )
		{
			 printf_f(file_log,"insert callback fail . break\r\n");	
			 return FS_ERR;
		}
		/* open */
		printf_f(file_log,"insert callback ok . break\r\n");	
		/* initial */
		aging_config_default();
	  /*return*/
	  return FS_OK;
}
/*- printf to file */
void printf_f(struct file * f,const char * p)
{
	if( file_log != 0 )
	{
		fs_write(file_log,p,strlen(p));
	}
}
/* call back transfer */
int factory_callback(int type,void * data,int len)
{
	command_long_def command_long;
	/* set the filter */
	switch(type)
	{
		/* 0xff is the wave task */
		case 0xff:
			factory_unit(data,len);
		break;
		/* 76 - xxxxxxxx is the aging test */
		case 76:
      /* -- */
			if( len == sizeof(command_long) )
			{
				  /* copy data */
					memcpy(&command_long,data,sizeof(command_long));
					/* case what */
				  switch(command_long.command)
					{
							case MAVLINK_CMD_FACTORY_CMD_1:
							case MAVLINK_CMD_FACTORY_CMD_2:
							case MAVLINK_CMD_FACTORY_CMD_3:
							case MAVLINK_CMD_FACTORY_CMD_4:
							case MAVLINK_CMD_FACTORY_CMD_5:
							case MAVLINK_CMD_FACTORY_CMD_6:	
							case MAVLINK_CMD_FACTORY_CMD_7:	
							case MAVLINK_CMD_FACTORY_CMD_8:	
							case MAVLINK_CMD_FACTORY_CMD_9:	
							case MAVLINK_CMD_FACTORY_CMD_10:	
							case MAVLINK_CMD_FACTORY_CMD_11:	
							case MAVLINK_CMD_FACTORY_CMD_12:
							case MAVLINK_CMD_FACTORY_CMD_13:
								/* ---  */
								gs_factory_handle(command_long.command,data);
								break;
							default :
								break;
					}
			}
		  /* break */
		  break;
			/* default */
		default :
			break;
	}
	/* return */
	return FS_OK;
}
/* get inode link head */
static struct inode * inode_get_header(void)
{
	  /* some details */
		struct inode * base;
	  inode_vmn * d;
		/* get the base */
		d = inode_valid();
		/*--------------*/
		if( d == NULL )
		{
			return NULL;
		}
		/* get base inode */
		base = d->inode;
		/* peer */
		int proctecter = 2000;
		/* get the header */
		while(proctecter--)
		{
			/* judge if it's the tail */
			if( base->i_child == 0 )
			{
				return base;
			}
			/* recommder */
			base = base->i_child;
		}
		/* return error */
		return NULL;
}
/* get shell link head */
static struct shell_cmd * shell_get_header(void)
{
	  /* some details */
  	struct shell_cmd *p_shell;
		/* get the base */
		/* shell section */
		p_shell = shell_node_valid();
		/*--------------*/
		if( p_shell == NULL )
		{
			return NULL;
		}
		/* peer */
		int proctecter = 2000;
		/* get the header */
		while(proctecter--)
		{
			/* judge if it's the tail */
			if( p_shell->i_child == 0 )
			{
				return p_shell;
			}
			/* recommder */
			p_shell = p_shell->i_child;
		}
		/* return error */
		return NULL;
}
/* all name */
static int publish_all_num(unsigned char * node_num,unsigned char * shell_num)
{
		struct inode *n_start;
		struct shell_cmd * p_shell,*s_start;
		unsigned char num_n = 0 , num_s = 0;
		struct inode * base;
		/* get the base */
		base = inode_get_header();
		/*--------------*/
		if( base == NULL )
		{
			return FS_ERR;
		}
		/* peer */
		for(n_start = base;n_start != (void*)0;n_start = n_start->i_peer)
		{
			num_n++;
		}
		/* shell section */
		p_shell = shell_get_header();
		/*--------------*/
		if( p_shell == NULL )
		{
			return FS_ERR;
		}
		/* s_start */
		/* search all shell */
		for(s_start = p_shell ; s_start != (void*)0 ; s_start = s_start->i_peer)
		{ 
			num_s++;
		}
		/* get data */
		*node_num = num_n;
		*shell_num = num_s;
		/* reutrn ok */
		return FS_OK;
}
static int publish_one_node(unsigned char num)
{
		node_def node;
		struct inode *n_start;
		struct inode * base;
		/* get the base */
		base = inode_get_header();
		/*--------------*/
		if( base == NULL )
		{
			return FS_ERR;
		}
		/* base mode */
		n_start = base;
		/* get data */
		for( unsigned char i = 0 ; i < num ; i++ )
		{
			 n_start = n_start->i_peer;
		}
		/* peer */
		memset(node.name,0,sizeof(node.name));
		memcpy(node.name,	n_start->inode_name,strlen(n_start->inode_name));
		/* set the base */
		node.line_num = num;
		node.i_flags = n_start->i_flags;
		node.type = (-1);//node;
		node.size_pl = 0;
		/* switch interface */
		fs_ioctl(ground , 1 , (32<<16) | 0xfe , (unsigned char *)&node);
		/* return ok */
		return FS_OK;
}
/* int publish_one_name(unsigned int dummy) */
static int publish_one_shell( unsigned char num  )
{    
		struct shell_cmd * p_s,*sp_start;
		node_def node;
		int real_len;
		/* shell section */
		p_s = shell_get_header();
		/*--------------*/
		if( p_s == NULL )
		{
			return FS_ERR;
		}
		/* s_start */
		/* search all shell */
		sp_start = p_s;
		/**/
		for( unsigned char i = 0 ; i < num ; i ++ )
		{
		  sp_start = sp_start->i_peer;
		}
		/* start */
		memset(node.name,0,sizeof(node.name));
		memcpy(node.name,sp_start->cmd,strlen(sp_start->cmd));
		/* lines */
		node.line_num = num;
		node.i_flags = 0;
		node.type = sp_start->it_type;
		node.size = sp_start->size;
		//		 /* -- exe -- */
		if( ((node.type & _CB_EXE_) == _CB_EXE_) || ((node.type & _CB_IDLE_) == _CB_IDLE_) 
			 || ((node.type & _CB_TIMER_) == _CB_TIMER_) || ((node.type & _CB_IT_) == _CB_IT_) )
		{
			 unsigned int tmp = (unsigned int)sp_start->enter;
			 /* copy */
			 real_len = 4;
			 memcpy(node.data,&tmp,4);
		}
		else
		{
			 /* not the node */			 
			 if( ( (sp_start->size >> 16) & 0xf ) > 4 )
			 {
					num = 4 * (sp_start->size&0xffff);
			 }else
			 {
					num = ((sp_start->size & 0x7fffff) >> 16) * (sp_start->size&0xffff);
			 }
			 /* get length */
			 real_len = (num>48)?48:num;
			 /* copy data */
			 memcpy(node.data,sp_start->enter,real_len);
		}
		/* get real data */
		node.size_pl = real_len;
		/* publish */
		fs_ioctl(ground , 1 , ((40+real_len)<<16) | 0xfe , (unsigned char *)&node);
		/* return */
		return FS_OK;
}
/* int publish_one_name(unsigned int dummy) */
static struct shell_cmd * gets_one_shell( unsigned char num  )
{    
		struct shell_cmd * p_s,*sp_start;
		/* shell section */
		p_s = shell_get_header();
		/*--------------*/
		if( p_s == NULL )
		{
			return NULL;
		}
		/* s_start */
		/* search all shell */
		sp_start = p_s;
		/**/
		for( unsigned char i = 0 ; i < num ; i ++ )
		{
		  sp_start = sp_start->i_peer;
		}
		/* return */
		return sp_start;
}
/* factory thread with ID is 0xFF */
void factory_unit( unsigned char * data , unsigned char len )
{
	  int i;
	  unsigned int sum = 0;
	  /*---------*/
	  switch(data[0])
		{
			case 0xff:
				/* enter factory mode */
				if( factory_flags & 0x01 )
				{
					/* clear */
					factory_flags &=~ 0x01;
					/* delete the odd task */				
					shell_delete_dynamic("ground_station_task_100ms",0);
					shell_delete_dynamic("ground_station_task_idle",3);
					/* create new  */
					shell_create_dynamic("gs_simple_task",gs_simple_task,3);
				}
				/*-------------------*/
				factory_buffer[0] = 0xff;
				/* get all length */
				if( publish_all_num(&factory_buffer[1],&factory_buffer[2]) != FS_OK )
				{
				  factory_buffer[1] = 0;
					factory_buffer[2] = 0;
				}
				/* stop the wave thread */
				wave_flag = 0;
				/* notify the ground */
				fs_ioctl(ground , 1 , ((3)<<16) | 0xff , factory_buffer);
				/* break */
				break;
			case 0xfe:
				/* get one node or shell */
			  if( len == 3 )
				{
					/* update version */
					for( int i = 0 ; i < 24 ; i ++ )
          {
						version_unit[i] = fs_ioctl(bl,0,i,0);
						version_v100[i] = version_unit[i];
					}
					/* v100 */
					if( D_or_v == __V_SERIES__ )
					{
						for( int i = 24 ; i < 36 ; i ++ )
						{
							version_vion[i-24] = fs_ioctl(bl,0,i,0);
						}
					}
					/*----------------*/
					if( data[1] == 0x00 )
					{
					  publish_one_node(data[2]);
					}else
					{
						publish_one_shell(data[2]);
					}
				}					
			  break;
			case 0xfd:
			case 0xfc:
				/*-------------*/
			  memcpy( &line_cmd ,data , len );
			  /* get data */
			  if( line_cmd.line_num * 2 + 4 == len )
				{
					if( line_cmd.cmd == 0xfd )
					{
						/* create task */
						if( factory_flags & 0x02 )
						{
							/* clear */
							factory_flags &=~ 0x02;
							/* create the line task */
							shell_create_dynamic("line_task",line_task,2);
						}
						/*-------*/
						wave_flag = 0;
						package_freq = 0;
						/*---------------*/
						/* publish */
						fs_ioctl(ground , 1 , ((len)<<16) | 0xff , data);
						/*---------------*/
				  }else
					{
						/* clear the sum */
						sum = 0;
						/* loop */
						for( i = 0 ; i < line_cmd.line_num ; i++ )
            {
							 struct shell_cmd * pri_data = gets_one_shell( line_cmd.line_gid[i][0]);
							 /*---------------*/
							 if( pri_data != NULL )
							 {
									int width = ( pri_data->size >> 16 ) & 0xf ;
									/* get width real */
									if( width > 4 )
									{
									   width = 4;
									}
									/*------------------*/
								  sum += width;
								}
						}	
            /*----------*/
						package_freq = ( sum + 2 + 8 ) / 12 + 1;//(ms)
            /*----------*/						
						wave_flag = 1;
					}
			  }
				break;
			default:
				break;
		}
}
/* line task */
void line_task(void)
{
	static unsigned int cnt = 0;
	unsigned int len_total = 0;
	/*------------*/
	if( wave_flag == 0 )
	{
		return;
	}
	/* cnt */
	cnt++;
	/* return */
	if( !(cnt > package_freq) )
	{
		return;
	}
	/* */ 
	cnt = 0;
	for( int i = 0 ; i < line_cmd.line_num ; i ++ )//wave_buffer 
	{
		/* gets node */
		struct shell_cmd * pri_data = gets_one_shell( line_cmd.line_gid[i][0] );//group
		/*-----------*/
		if( pri_data == NULL )
		{
			return;
		}
		/*------------*/
		int width = ( pri_data->size >> 16 ) & 0xf ;
		/* get width real */
		if( width > 4 )
		{
			width = 4;
		}
		/*--- copy data ---*/
		unsigned char * enter = pri_data->enter;
		/* copy */
		memcpy(&wave_buffer[2+len_total],enter + width * line_cmd.line_gid[i][1] , width );
		/*-----------------*/
		len_total += width;	
  }
	/*------*/
	wave_buffer[0] = 0xfb;
	/* send */
	fs_ioctl(ground , 1 , ((2 + len_total)<<16) | 0xff , (unsigned char *)&wave_buffer);
}
/* gs simple task */
void gs_simple_task(void)//????????
{
  /* len */
	int len;
	/* read from radio */
	len = fs_read(ground_uart5,buf_receive,64);	
	/* get data */
	for( int i = 0 ; i < len ; i++ )
	{
		 fs_ioctl(ground , 5 , buf_receive[i],0);
	}
}




























