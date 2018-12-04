
#include "fs.h"
#include "f_shell.h"
#include "fs_config.h"
#include "string.h"
#include "f_ops.h"
#include "f_drv.h"
#include "f_shell_cfg.h"
#include "easylink.h"
/* static file * uart0 */
static struct file * file_debug;
static easylink easylink_d;
extern void shell_interface(easylink * easy);
/* send buffer */
static unsigned char send_buffer[36];//8+4*7 = 36;
static unsigned char wnok = 0;
/* define the inode */
FS_INODE_REGISTER("/shell.o",shell,shell_heap_init,0);
/* fs thread */
FS_SHELL_STATIC(shell_thread,shell_thread,4,_CB_TIMER_|_CB_IT_IRQN_(TASK_PERIOD5_ID));//12ms
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
	shell.flip.f_path = "/shell.o";
	/* heap */
  shell.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* add your own code here */

	/* ------end of file------ */
	return FS_OK;
}
/* fdg */
static int shell_default_config(void)
{
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
	  /*return*/
	  return FS_OK;
}
/* thread */
void shell_thread(void)
{
	int len;
	static char buffer[200];
	/* read buffer */
	len = fs_read(file_debug,buffer,200);
	/* enter ? */
	if( len != 0 )
	{
		for( int i = 0 ; i < len ; i++ )
		{
       if( iap_d_icr(buffer[i],&easylink_d) == 0)
		   {
		 	   shell_interface(&easylink_d);
		   }
		}
	}
}
/* shell general send */
int shell_pushlish(unsigned char * buffer , unsigned short len)
{
	/* send */
	fs_write(file_debug,(const char *)buffer,len);

	return FS_OK;
}
































