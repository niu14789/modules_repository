///** 
//* @file      	os_init.c 
//* @brief     	initialize for OS
//* @details		such as : task create, the para to fill... 	 
//* @author   	tangcf 
//* @date      	2016-8-15
//* @version  	FM_IFC_3.0_Plane
//* @par Copyright (c):  
//*       ���ڷ�������˿Ƽ����޹�˾��������
//*/  

#include "string.h"
#include "uartd.h"
#include "fs.h"
#include "fs_config.h"
#include "f_shell.h"
/* 
* uart0-> index:0
* uart1-> index:1
* uart2-> index:2
* uart3-> index:3
* uart4-> index:4
* usart0->index:5
* usart1->index:6
* usart2->index:7
*/
/*
*   static params
*/
sXdmad dmaDrv;
/* file interface */
static struct file filp_s[8];
/*
*    usart0~3:channel->{[usart0-tx][usart0-rx]}
*/
static UsartChannel chs_txrx[3][2];
/*
*    uart0~4:channel->{[uart0-tx][uart0-rx}}
*/
static UartChannel ch_txrx[5][2];
/*
*    XDMA receive buffer 
*/
COMPILER_ALIGNED(32)  uint8_t receive[16][64];
/* single mode recive length */
static unsigned int  length_single_usart[3];
/* dma config buffer */
static UsartDma usart_dma[3];
static UartDma  uart_dma[5];
/* uart buf read offset */

/* define pins */
const Pin uart_dev_pins_tx[8][12] = {{PINS_UART0_TX},{PINS_UART1_TX},{PINS_UART2_TX},{PINS_UART3_TX},{PINS_UART4_TX},{PINS_USART0_TX},{PINS_USART1_TX},{PINS_USART2_TX}};
const Pin uart_dev_pins_rx[8][12] = {{PINS_UART0_RX},{PINS_UART1_RX},{PINS_UART2_RX},{PINS_UART3_RX},{PINS_UART4_RX},{PINS_USART0_RX},{PINS_USART1_RX},{PINS_USART2_RX}};
const unsigned int u_s_art_id[8] =  {ID_UART0,ID_UART1,ID_UART2,ID_UART3,ID_UART4,ID_USART0,ID_USART1,ID_USART2};
const unsigned int u_s_art_source[5] = {UART0_IRQn,UART1_IRQn,UART2_IRQn,UART3_IRQn,UART4_IRQn};
const unsigned int usart_addr[3] = { 0x40024000,0x40028000,0x4002C000U};
const unsigned int uart_addr[5] = {0x400E0800U,0x400E0A00U,0x400E1A00U,0x400E1C00U,0x400E1E00U};
/* declice */
int u_s_art_write(struct file * filp, FAR const char *buffer, unsigned int buflen);
struct file * u_s_art_open(FAR struct file * filp);
unsigned int u_s_art_read(FAR struct file *filp, FAR char *buffer, unsigned int buflen);
int u_s_art_close(FAR struct file *filp);
/* uart config(shell) */
static unsigned char uart_config[8];
/* callback function */
static callback_function uart_dma_callback[16];
/* static last cda */
static unsigned int cda_last[8];
/* static unsigned int addr_pdf */
//static unsigned int addr_pdf[100][4];
//static unsigned short addr_cnt;
//static unsigned char seq[3];
/* DMA LLI static malloc */
static unsigned char lli_malloc[16][sizeof(LinkedListDescriporView1) * LLI_SIZE_STATIC];
/* fs inode system register */
FS_INODE_REGISTER("/UART/",u_s_art,u_s_art_heap_init,0);
/* fs shell register */
//FS_SHELL_REGISTER(dma);
//FS_SHELL_REGISTER(u_s_art_callback_5s);//usart 0
//FS_SHELL_REGISTER(u_s_art_callback_6s);//usart 1
//FS_SHELL_REGISTER(u_s_art_callback_7s);//usart 2
FS_SHELL_REGISTER(uart_config);
FS_SHELL_REGISTER(receive);//u_s_art config
FS_SHELL_REGISTER(uart_dma_callback);//uart_callback function
/* u(s)art init */
int u_s_art_heap_init(void)
{
	  /* full of zero */
		memset(&u_s_art,0,sizeof(u_s_art));
		/*------------------*/
		u_s_art.shell_i = shell_sched_getfiles();
	  u_s_art.config = uart_default_config;
		/* file ops */
	  u_s_art.ops.open = u_s_art_open;
		u_s_art.ops.write = u_s_art_write;
	  u_s_art.ops.read = u_s_art_read;
	  u_s_art.ops.close = u_s_art_close;
		/* driver ops */
		u_s_art.drv_ops.config = u_s_art_config;
	  /* file interface */
	  u_s_art.flip.f_inode = &u_s_art;
	  u_s_art.flip.f_path = "/UART/";
		/* shell */
//		FS_SHELL_INIT(u_s_art_callback_5s,u_s_art_callback,0,_CB_IT_|_CB_PRIORITY_(0)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(USART0_IRQn)|_CB_ENABLE_);
//	  FS_SHELL_INIT(u_s_art_callback_6s,u_s_art_callback,0,_CB_IT_|_CB_PRIORITY_(0)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(USART1_IRQn)|_CB_ENABLE_);
//	  FS_SHELL_INIT(u_s_art_callback_7s,u_s_art2_idle,0,_CB_IT_|_CB_PRIORITY_(0)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(USART2_IRQn)|_CB_ENABLE_);
//		FS_SHELL_INIT(dma,dma,0,_CB_IT_|_CB_PRIORITY_(0)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(XDMAC_IRQn)|_CB_ENABLE_);
		FS_SHELL_INIT(receive,receive,0x010000 + 16*64,_CB_ARRAY_);
		FS_SHELL_INIT(uart_dma_callback,uart_dma_callback,0x040000 + 16,_CB_ARRAY_);
		FS_SHELL_INIT(uart_config,uart_config,0x010000 + 8 , _CB_ARRAY_);
		/* static init param */
		memset(&dmaDrv,0,sizeof(dmaDrv));
		memset(&chs_txrx,0,sizeof(chs_txrx));
		memset(&ch_txrx,0,sizeof(ch_txrx));
		memset(&usart_dma,0,sizeof(usart_dma));
		memset(&uart_dma,0,sizeof(uart_dma));	
		memset(&uart_dma_callback,0,sizeof(uart_dma_callback));
		memset(&receive,0,sizeof(receive));
		memset(&uart_config,0,sizeof(uart_config));
		memset(&lli_malloc,0,sizeof(lli_malloc));
		memset(&filp_s,0,sizeof(filp_s));
		memset(&length_single_usart,0,sizeof(length_single_usart));
		memset(cda_last,0,sizeof(cda_last));
//		memset(addr_pdf,0,sizeof(addr_pdf));
//		memset(seq,0,sizeof(seq));
//		addr_cnt = 0;
		/* lib files that need init*/
		dma_init();
		/* add your own code here */
	  u_s_art.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
		/* ------end of file------ */
		return FS_OK;
}

int uart_default_config(void)
{
	#if 0	
		/* default for test */
	  /* usart0 */
		uart_config_msg msg;
		msg.mode = 
				  US_MR_USART_MODE_NORMAL
				| US_MR_CHRL_8_BIT
				| US_MR_PAR_NO
				| US_MR_NBSTOP_1_BIT
				| US_MR_CHMODE_NORMAL;
		msg.baudrate = 115200;
		msg.tx_mode = _UART_TX_DMA;
		msg.rx_mode = _UART_RX_IT;
		msg.rx_dma_deepth = 64;
		msg.index = 5;
	  msg.rx_dma_buffer = NULL;
		u_s_art_config(&msg,sizeof(msg));
	  /* usart1 */
	  msg.index = 6;
	  u_s_art_config(&msg,sizeof(msg));
	  /* usart2 */	
	  msg.index = 7;
	  u_s_art_config(&msg,sizeof(msg));	
  
		/* uart 0 */
		msg.mode = 
					  UART_MR_PAR_NO
					| UART_MR_BRSRCCK_PERIPH_CLK
					| UART_MR_CHMODE_NORMAL;
		msg.index = 0;
		msg.rx_mode = _UART_RX_DMA; 
		u_s_art_config(&msg,sizeof(msg));	
		 
		/* uart1 */
		msg.index = 1;
		u_s_art_config(&msg,sizeof(msg));
		/* uart2 */
		msg.index = 2;
		u_s_art_config(&msg,sizeof(msg));

		/* uart3 */
		msg.index = 3;
		u_s_art_config(&msg,sizeof(msg));
		
		/* uart4 */
		msg.index = 4;
		u_s_art_config(&msg,sizeof(msg));		
#endif
		return 0;
}

int u_s_art_config(void * p_arg , int argc)
{
		uint32_t mode = 0;//compliant with C90
		char uart_index = 0;
	  /* get the lengtch */
		if(sizeof(uart_config_msg) != argc)
		{
			return FS_ERR;// not supply this format
		}
		/* get the interface */
		uart_config_msg * ucm = (uart_config_msg *)p_arg;
		/* index ? */
		if( ucm->index > 7 )
			return FS_ERR;// not supply this format
		
		/* config mode normal or not */
    mode = ucm->mode;
		/* only set the baudrate ? */
		if( 0x80000000 & mode )
		{
			/* set baudrate mode */
			if(ucm->index > 4)
			{
				uart_index = ucm->index - 5;//usart0~2
				USART_SetBaudrate(usart_dma[uart_index].pUsartHw , 0 , ucm->baudrate , BOARD_MCK);
			}else
			{
				uart_index = ucm->index;//uart0~4
				UART_Configure(uart_dma[uart_index].pUartHw , ucm->mode & 0xffff , ucm->baudrate , BOARD_MCK);
			}
			return FS_OK;
		}
		/* u(s)art config tx */
		if(ucm->index > 4)
		{
			uart_index = ucm->index - 5;//usart0~2
			if(ucm->tx_mode == 0x0002)
			{
				chs_txrx[uart_index][0].BuffSize = 0;
				chs_txrx[uart_index][0].pBuff = NULL;;
				chs_txrx[uart_index][0].callback = u_s_art_dma_tx_cb;
				chs_txrx[uart_index][0].dmaProgress = 1;
				chs_txrx[uart_index][0].dmaProgrammingMode = XDMAD_SINGLE;
				chs_txrx[uart_index][0].pLLIview = (LinkedListDescriporView1 *)&lli_malloc[ucm->index * 2];
				uart_config[ucm->index] |= 0x40;//dma send
			}
		}else
		{
			uart_index = ucm->index;//uart0~4
			if(ucm->tx_mode == 0x0002)
			{
				ch_txrx[uart_index][0].BuffSize = 0;
				ch_txrx[uart_index][0].pBuff = NULL;;
				ch_txrx[uart_index][0].callback = u_s_art_dma_tx_cb;
				ch_txrx[uart_index][0].sempaphore = 1;
				ch_txrx[uart_index][0].dmaProgrammingMode = XDMAD_SINGLE;
				ch_txrx[uart_index][0].pLLIview = (LinkedListDescriporView1 *)&lli_malloc[ucm->index * 2];
				uart_config[ucm->index] |= 0x40;//dma send
			}
		}

	/* u(s)art config rx */
		if(ucm->index > 4)
		{
			uart_index = ucm->index - 5;//usart0~2
			if(ucm->rx_mode == 0x0001 || ucm->rx_mode == 0x0002)
			{
				chs_txrx[uart_index][1].callback = u_s_art_dma_rx_cb;
				chs_txrx[uart_index][1].dmaProgress = 1;
				chs_txrx[uart_index][1].dmaProgrammingMode = XDMAD_SINGLE;
				chs_txrx[uart_index][1].pLLIview = (LinkedListDescriporView1 *)&lli_malloc[ucm->index * 2 + 1];
				uart_config[ucm->index] &=~ 0x20;//single
				if(ucm->rx_mode == 0x0002)
				{
					chs_txrx[uart_index][1].dmaRingBuffer = 1;
					chs_txrx[uart_index][1].dmaBlockSize = LLI_SIZE_STATIC;
					chs_txrx[uart_index][1].dmaProgrammingMode = XDMAD_LLI;
					uart_config[ucm->index] |= 0x20; //lli
				}
				/* recive data from dma */
				if(ucm->rx_dma_buffer == NULL)
				{
					chs_txrx[uart_index][1].pBuff = &receive[ucm->index*2][0];
					chs_txrx[uart_index][1].BuffSize = (ucm->rx_dma_deepth > 64)?64:ucm->rx_dma_deepth;
					uart_config[ucm->index] &=~ 0x80; 
				}else
				{
					chs_txrx[uart_index][1].pBuff = ucm->rx_dma_buffer;
          chs_txrx[uart_index][1].BuffSize = ucm->rx_dma_deepth;	
          uart_config[ucm->index] |= 0x80;					
				}
			}
		}else
		{
			uart_index = ucm->index;
			if(ucm->rx_mode == 0x0002)
			{
				ch_txrx[uart_index][1].dmaRingBuffer = 1;//ѭ��LLI
				ch_txrx[uart_index][1].dmaBlockSize = LLI_SIZE_STATIC;//����LLI
				ch_txrx[uart_index][1].dmaProgrammingMode = XDMAD_LLI;
				ch_txrx[uart_index][1].callback = u_s_art_dma_rx_cb;
				ch_txrx[uart_index][1].sempaphore = 1;
				ch_txrx[uart_index][1].pLLIview = (LinkedListDescriporView1 *)&lli_malloc[ucm->index * 2 + 1];
				uart_config[ucm->index] |= 0x20;//lli
				/* set dma rx buffer */
				if(ucm->rx_dma_buffer == NULL)
				{
					ch_txrx[uart_index][1].pBuff = &receive[ucm->index*2][0];
					ch_txrx[uart_index][1].BuffSize = (ucm->rx_dma_deepth > 64)?64:ucm->rx_dma_deepth;
					uart_config[ucm->index] &=~ 0x80; 
				}else
				{
					ch_txrx[uart_index][1].pBuff = ucm->rx_dma_buffer;
					ch_txrx[uart_index][1].BuffSize = ucm->rx_dma_deepth;//ÿ�����峤��
					uart_config[ucm->index] |= 0x80;	
				}
			}
		}
		/* u(s)art config dma */
		if(ucm->index > 4)
		{
			uart_index = ucm->index - 5;//usart0~2
			usart_dma[uart_index].pTxChannel = &chs_txrx[uart_index][0];
			usart_dma[uart_index].pRxChannel = &chs_txrx[uart_index][1];
			usart_dma[uart_index].pXdmad = &dmaDrv;
		}else
		{
			uart_index = ucm->index;//usart0~2
			uart_dma[uart_index].pTxChannel = &ch_txrx[uart_index][0];
			uart_dma[uart_index].pRxChannel = &ch_txrx[uart_index][1];
			uart_dma[uart_index].pXdmad = &dmaDrv;
		}
		/* uart config uart tx gpio */
		if(ucm->tx_mode)
		{
				PIO_Configure(uart_dev_pins_tx[ucm->index], PIO_LISTSIZE(uart_dev_pins_tx[ucm->index]));
		}
		/* uart config uart rx gpio */
		if(ucm->rx_mode)
		{
				PIO_Configure(uart_dev_pins_rx[ucm->index], PIO_LISTSIZE(uart_dev_pins_rx[ucm->index]));
		}
		/* enable clock */
		PMC_EnablePeripheral(u_s_art_id[ucm->index]);
		/* uart config real */
		if(ucm->index > 4)
		{
			uart_index = ucm->index - 5;//usart0~2
			USARTD_Configure(&usart_dma[uart_index], u_s_art_id[ucm->index], mode, ucm->baudrate, BOARD_MCK);
		}else
		{
			uart_index = ucm->index;//usart0~2
			UARTD_Configure(&uart_dma[uart_index], u_s_art_id[ucm->index], mode, ucm->baudrate, BOARD_MCK);
		}
		/* config usart it */
		if(ucm->rx_mode == 0x0001)
		{
			if( ucm->index > 4 )
			{			
				uart_index = ucm->index - 5;//usart0~2
				USART_EnableRecvTimeOut(usart_dma[uart_index].pUsartHw, 200);  // 104us/bit * 10bit
				USART_EnableIt(usart_dma[uart_index].pUsartHw,US_IER_TIMEOUT);
			}
		}else if(ucm->rx_mode == 0x0003)
		{
			if( ucm->index > 4 )
			{			
				uart_index = ucm->index - 5;//usart0~2
				USART_EnableIt(usart_dma[uart_index].pUsartHw,US_IER_RXRDY);
			}else
			{
				uart_index = ucm->index;//usart0~2
				UART_EnableIt(uart_dma[uart_index].pUartHw,US_IER_RXRDY);
			}
		}
		/* enable dma rx */
		if(ucm->rx_mode == 0x0001 || ucm->rx_mode == 0x0002)
		{
			if( ucm->index > 4 )
			{			
				uart_index = ucm->index - 5;//usart0~2
				USARTD_EnableRxChannels(&usart_dma[uart_index], &chs_txrx[uart_index][1]);
				USARTD_RcvData(&usart_dma[uart_index]);
			}else
			{
				uart_index = ucm->index;//usart0~2
				UARTD_EnableRxChannels(&uart_dma[uart_index], &ch_txrx[uart_index][1]);
				UARTD_RcvData(&uart_dma[uart_index]);
			}
		}
		/* config SCB */
		if( ucm->index > 4 )
		{			
			uart_index = ucm->index - 5;//usart0~2
			SCB_InvalidateDCache_by_Addr((uint32_t *)chs_txrx[uart_index][1].pBuff,chs_txrx[uart_index][1].BuffSize);
		}
		else
		{
			uart_index = ucm->index;
			SCB_InvalidateDCache_by_Addr((uint32_t *)ch_txrx[uart_index][1].pBuff,ch_txrx[uart_index][1].BuffSize);
		}
		return FS_OK;
}
int u_s_art_callback(int type,void * data,int len)
{
	  int len_data = 0;
    char index = 0;
	  switch(type)
		{
			case USART0_IRQn:
			case USART1_IRQn:
			case USART2_IRQn:
			{
					index = type - USART0_IRQn;
				  if( ((Usart *)usart_addr[index])->US_CSR & (1<<8) )
					{
						  ((Usart *)usart_addr[index])->US_CR |= 1<<11;
							/* usart0 */	  
							USART_AcknowledgeRxTimeOut(usart_dma[index].pUsartHw,0);
						
							USARTD_DisableRxChannels(&usart_dma[index], &chs_txrx[index][1]);
						
							len_data = chs_txrx[index][1].BuffSize - usart_dma[index].pXdmad->pXdmacs->XDMAC_CHID[usart_dma[index].pRxChannel->ChNum].XDMAC_CUBC;
						
							uart_config[5+index] |= 0x10;//get data
							length_single_usart[index] = len_data;
//							if(uart_dma_callback[11 + index * 2] != NULL )
//							{
//								 uart_dma_callback[11 + index * 2](USART0_IRQn+index,chs_txrx[index][1].pBuff,len_data);
//							}
						
							SCB_InvalidateDCache_by_Addr((uint32_t *)chs_txrx[index][1].pBuff,chs_txrx[index][1].BuffSize);
				//				
							// chs_txrx[0][1].pBuff = &receive[11][0];
						
							USARTD_EnableRxChannels(&usart_dma[index], &chs_txrx[index][1]);
							USARTD_RcvData(&usart_dma[index]);	
				  }
					break;
			}
			case UART0_IRQn:
			case UART1_IRQn:
			case UART2_IRQn:
			case UART3_IRQn:
			case UART4_IRQn:
			{
					if(type >= UART2_IRQn)
					{ 
						index = type - UART2_IRQn + 2;
					}else
					{
						index = type - UART0_IRQn;
					}	
					
					if(uart_dma_callback[1 + index * 2] != NULL )
					{
						 uart_dma_callback[1 + index * 2](u_s_art_source[index],ch_txrx[index][1].pBuff,len_data);
					}
					break;
			}
			default: break;
		}
		
	  return len_data;
}

int dma(int type,void * data,int len)
{
	  XDMAD_Handler(&dmaDrv);
	  return 0;
}

void u_s_art_dma_rx_cb(char id,uint8_t ichannel, void * data)
{
	char postion = 0;
	char index = 0;
	/* whitch one? */
	switch(id)
	{
		case ID_USART0:
		case ID_USART1:
		case ID_USART2:
		{	  
				index = id - ID_USART0;
				/* nothing */
				postion = uart_config[5+index] & 0x1;
//				if(uart_dma_callback[11+index*2] != NULL )
//				{
//					 uart_dma_callback[11+index*2](USART0_IRQn+index,chs_txrx[index][1].pBuff + postion * chs_txrx[index][1].BuffSize,chs_txrx[index][1].BuffSize);
//				}
        /* clear DCache */
				SCB_InvalidateDCache_by_Addr((uint32_t *)(chs_txrx[index][1].pBuff +  postion * chs_txrx[index][1].BuffSize) , chs_txrx[index][1].BuffSize);
				/* set anti */
				uart_config[5+index] ^= 0x1;
				uart_config[5+index] |= 0x10;//get data
				break;
		}
		case ID_UART0:
		case ID_UART1:
		case ID_UART2:
		case ID_UART3:
		case ID_UART4:
		{
 				if(id >= ID_UART2)
				{ 
					index = id - ID_UART2 + 2;
				}else
				{
					index = id - ID_UART0;
				}	
				/* nothing */
				postion = uart_config[index] & 0x1;
				if(uart_dma_callback[1+index*2] != NULL )
				{
						uart_dma_callback[1+index*2](u_s_art_source[index],ch_txrx[index][1].pBuff + postion * ch_txrx[index][1].BuffSize,ch_txrx[index][1].BuffSize);
				}
				/* clear DCache */
				SCB_InvalidateDCache_by_Addr((uint32_t *)(ch_txrx[index][1].pBuff) , ch_txrx[index][1].BuffSize * 2);
				/* set anti */
				uart_config[index] ^= 0x1;
				uart_config[index] |= 0x10;//get data
				break;
		}
		default : break;
	}
}	

void u_s_art_dma_tx_cb(char id,uint8_t ichannel, void * data)
{
	char index = 0;
  /* whitch one? */
	switch(id)
	{
		case ID_USART0:
		case ID_USART1:
		case ID_USART2:
		{	  
				index = id - ID_USART0;
				if(uart_dma_callback[10+index*2] != NULL )
				{
					 uart_dma_callback[10+index*2](USART0_IRQn+index,chs_txrx[index][0].pBuff,chs_txrx[index][0].BuffSize);
				}
				if(uart_config[index + 5] & 0x40)//dma send)
				{
					 USARTD_DisableTxChannels(&usart_dma[index], &chs_txrx[index][0]);
				}
		}
		/* end */
		break;
		/* others */
		case ID_UART0:
		case ID_UART1:
		case ID_UART2:
		case ID_UART3:
		case ID_UART4:
		{
				if(id >= ID_UART2)
				{ 
					index = id - ID_UART2 + 2;
				}else
				{
					index = id - ID_UART0;
				}	
				/* nothing */
				if(uart_dma_callback[index*2] != NULL )
				{
						uart_dma_callback[index*2](u_s_art_source[index],ch_txrx[index][0].pBuff,ch_txrx[index][0].BuffSize);
				}
				if(uart_config[index] & 0x40)//dma send)
				{
					 UARTD_DisableTxChannels(&uart_dma[index], &ch_txrx[index][0]);
				}
		}
		/* end */
		break;
		/* break */
		default : break;
	}
}
/* file interface */
struct file * u_s_art_open(FAR struct file * filp)
{
	 int length;
	 char index;
	 /* get length of path */
   length = strlen(	filp->f_path);
	 /* is length more than 7? */
	 if(length == 0x7)
	 {
		 index = filp->f_path[6] - '0';
		 if(index < 8)
		 {
			 /* open always */
			 filp_s[index].f_oflags = __FS_IS_INODE_OK;
			 filp_s[index].f_inode = &u_s_art;
			 filp_s[index].f_multi = index;
			 filp_s[index].f_path = "/UART/0-7";
			 /* allow open */
			 return &filp_s[index];//open succeed	
		 }else
		 {
			 	/* can not find this device*/
		    return NULL;//open fail
		 }
	 }else
	 {
		 /* can not supply this format */
		 return NULL;//open fail
	 }
}
/* fs write */
int u_s_art_write(struct file * filp, FAR const char *buffer, unsigned int buflen)
{ 
	 int i;
	 char index = 0;
	
	 if(filp == NULL)
			 return FS_ERR; //not supply this format

	 if(uart_config[filp->f_multi] & 0x40)
	 {
			 //dma send
			 if(filp->f_multi > 4)
			 {
				 index = filp->f_multi - 5;
				 chs_txrx[index][0].pBuff = (unsigned char *)buffer;
				 chs_txrx[index][0].BuffSize = buflen;
				 u_s_art_dma_tx_cb(u_s_art_id[filp->f_multi],0,NULL);
				 USARTD_EnableTxChannels(&usart_dma[index], &chs_txrx[index][0]);
				 USARTD_SendData(&usart_dma[index]);
			 }else
			 {
				 index = filp->f_multi;
				 ch_txrx[index][0].pBuff = (unsigned char *)buffer;
				 ch_txrx[index][0].BuffSize = buflen;
				 u_s_art_dma_tx_cb(u_s_art_id[filp->f_multi],0,NULL);
				 UARTD_EnableTxChannels(&uart_dma[index], &ch_txrx[index][0]);
				 UARTD_SendData(&uart_dma[index]);
			 }
	 }else
	 {
			if(filp->f_multi > 4)
			{
				index = filp->f_multi - 5;
				/* send */
				for( i = 0 ; i < buflen ; i++ )
				{
					 USART_PutChar((Usart *)usart_addr[index],buffer[i]);//usart_addr
				}
			}else
			{
				index = filp->f_multi;
				/* send */
				for( i = 0 ; i < buflen; i++)
				{
					 UART_PutChar((Uart *)uart_addr[index],buffer[i]);//usart_addr
				}
			}
	 }
	 /* end */
	 return FS_OK;
}	
unsigned int u_s_art_read(FAR struct file *filp, FAR char *buffer, unsigned int buflen)
{
		char index = 0;
		unsigned int buffsize = 0;
	  /*------------------*/
		/* 66666666666666 */
		if(filp->f_multi > 7)
		   return 0; // can not find this device

		if(filp->f_multi > 4) //length_single_usart
		{ 
			 index = filp->f_multi - 5;
			 /* mode */
			 unsigned char mode = ( uart_config[filp->f_multi] & 0x20 ) ? 1 : 0;
			 /* is LLI mode ? */
			 if( mode )
			 {
				  unsigned int ofet_last = (unsigned int)usart_dma[index].pXdmad->pXdmacs->XDMAC_CHID[usart_dma[index].pRxChannel->ChNum].XDMAC_CDA;
				  /*---------------------*/
				  if(  ofet_last != cda_last[filp->f_multi] )
					{
						 if( cda_last[filp->f_multi] == 0 )
						 {
							 cda_last[filp->f_multi] = ofet_last;
							 /* return 0 */
							 return 0;
						 }
						 unsigned int src = (unsigned int)cda_last[filp->f_multi] - (unsigned int)chs_txrx[index][1].pBuff;
						 /* get transfer size */
						 if( ofet_last > cda_last[filp->f_multi] )
						 {
							 buffsize = ofet_last - cda_last[filp->f_multi]; 
						 }else
						 {
							 buffsize = chs_txrx[index][1].BuffSize * 2 - ( cda_last[filp->f_multi] - ofet_last );
						 }
						 /* little */
						 buffsize = ( buflen > buffsize ) ? buffsize : buflen;
//						 /* get the length */
//						 addr_pdf[addr_cnt][0] = ofet_last - (unsigned int)chs_txrx[index][1].pBuff;
//						 addr_pdf[addr_cnt][1] = cda_last[filp->f_multi] - (unsigned int)chs_txrx[index][1].pBuff;
//						 addr_pdf[addr_cnt][2] = buffsize;
//						 addr_pdf[addr_cnt][3] = (unsigned int)src - (unsigned int)chs_txrx[index][1].pBuff;
						 /* copy data */
						 for( int i = 0 ; i < buffsize ; i ++ )
						 {
							  buffer[i] = chs_txrx[index][1].pBuff[ ( src + i ) % ( chs_txrx[index][1].BuffSize * 2 ) ];
////							  /* defodst */
////							  if( buffer[i] == 0 )
////								{
////									buffer[i] = 'z';
////								}
//								 if( seq[0] != buffer[i] )
//								 {
//									 if( (unsigned char)( seq[0] + 1 ) != buffer[i] )
//									 {
//										 seq[0] = buffer[i];
//									 }else
//									 {
//										 seq[0] = buffer[i];
//									 }
//									 /*---------------*/
//									 if( seq[1] != 148 )
//									 {
//										 seq[1] = 1;
//									 }else
//									 {
//										 seq[1] = 1;
//									 }
//								 }else
//								 {
//									 seq[1]++;
//								 } 
						 }
//						 /*---*/
//						 addr_cnt ++;
//						 /*--------*/
//						 addr_cnt = addr_cnt % 100;
						 /* last == ofet last */
						 cda_last[filp->f_multi] = ofet_last;
						 /*-------------------*/
						 return buffsize;
						 /*-------------*/
					}else
					{
				      return 0;
					}
			 }else
			 {
				  buffsize = u_s_art_callback(USART0_IRQn + index , NULL , 0);
				  /* protect */
				  buffsize = ( buflen > buffsize ) ? buffsize : buflen ;
				  /* copy data */
				  memcpy( buffer , chs_txrx[index][1].pBuff , buffsize );
				  /* return length */
				  return buffsize;		  
			 }
		}else
		{
			 index = filp->f_multi;
       /* get ofet last */
			 unsigned int ofet_last = (unsigned int)uart_dma[index].pXdmad->pXdmacs->XDMAC_CHID[uart_dma[index].pRxChannel->ChNum].XDMAC_CDA;
			 /*---------------------*/
			 if(  ofet_last != cda_last[filp->f_multi] )
			 {
				 if( cda_last[filp->f_multi] == 0 )
				 {
					 cda_last[filp->f_multi] = ofet_last;
					 /* return 0 */
					 return 0;
				 }
				 /* get src position */
				 unsigned int src = (unsigned int)cda_last[filp->f_multi] - (unsigned int)ch_txrx[index][1].pBuff;
				 /* get transfer size */
				 if( ofet_last > cda_last[filp->f_multi] )
				 {
					 buffsize = ofet_last - cda_last[filp->f_multi]; 
				 }else
				 {
					 buffsize = ch_txrx[index][1].BuffSize * 2 - ( cda_last[filp->f_multi] - ofet_last );
				 }
				 /* little */
				 buffsize = ( buflen > buffsize ) ? buffsize : buflen;
				 /* copy data */
				 for( int i = 0 ; i < buffsize ; i ++ )
				 {
						buffer[i] = ch_txrx[index][1].pBuff[ ( src + i ) % ( ch_txrx[index][1].BuffSize * 2 ) ];
				 }
				 /* last == ofet last */
				 cda_last[filp->f_multi] = ofet_last;
				 /* clear cache */
				 u_s_art_dma_rx_cb(u_s_art_id[index],0,NULL);
				 /*-------------------*/
				 return buffsize;
				 /*-------------*/
			 }else
			 {
					return 0;
			 }
		}	
}
/* fs close */
int u_s_art_close(FAR struct file *filp)
{
	 if(filp->f_multi < 8)
	 {
		  filp_s[filp->f_multi].f_oflags = 0x0;
		  return FS_OK;
	 }else
	 {
		  return FS_ERR;
	 }
}

/* end */


































