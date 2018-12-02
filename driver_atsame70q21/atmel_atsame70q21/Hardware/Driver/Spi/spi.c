/*
 * spi.c
 *
 *  Created on: 2017-5-17
 *      Author: niu
 */
#include "board.h"
#include "fs.h"
#include "fs_config.h"
#include "f_shell.h"
#include "string.h"
#include "spi.h"
#include "f_drv.h"
/* spi pin definition */
const Pin spi_pins[] = {         ///<Pins to configure for the application.
	PIN_SPI_MISO,
	PIN_SPI_MOSI,
	PIN_SPI_SPCK,
	PIN_SPI_NPCS2,
	PIN_SPI_NPCS3
};
const Pin spi1_pins[] = {         ///<Pins to configure for the application.
	PIN_SPI1_MISO,
	PIN_SPI1_MOSI,
	PIN_SPI1_SPCK,
	PIN_SPI1_NPCS0,
	PIN_SPI1_NPCS1,
	PIN_SPI1_NPCS2,
	PIN_SPI1_NPCS3
};
/* declaration */
struct file * spi_dev_open(FAR struct file * filp);
/* fs inode system register */
FS_INODE_REGISTER("/spi.o",spi,spi_heap_init,0);
/* shell interface */
FS_SHELL_REGISTER(spi_handler);//spi0
/* heap init */
int spi_heap_init(void)
{
  /* full of zero */
	memset(&spi,0,sizeof(spi));
	/* shell base */
	spi.shell_i = shell_sched_getfiles();
	/* file opearation */
	spi.ops.open = spi_dev_open;
	/* driver config */
	spi.config = spi_default_config;
	spi.drv_ops.config = spi_config;
	spi.drv_ops.write = spi_wtire;
	spi.drv_ops.read = spi_read;
	spi.drv_ops.selectchip = spi_selectchip;
	/* file interface */
	spi.flip.f_inode = &spi;
	spi.flip.f_path = "/spi.o";
	/* heap */
  FS_SHELL_INIT(spi_handler,spi_handler,4,_CB_IT_|_CB_PRIORITY_(2)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(SPI0_IRQn)|_CB_ENABLE_);
	/* add your own code here */
  spi.i_flags = __FS_IS_INODE_OK|__FS_IS_INODE_INIT;
	/* ------end of file------ */
	return FS_OK;
}
/* setting default */
int spi_default_config(void)
{
	spi_config_msg msg;
	msg.freq = 5000000;//500Khz
	spi_config(&msg,sizeof(msg));

	return FS_OK;
}
/* spi config */
int spi_config(void * p_arg , int argc)
{
	/* pointer */
	spi_config_msg * msg;
	/* if the length not squal */	
	if(sizeof(spi_config_msg) != argc)
	{
		return FS_ERR;// not supply this format
	}
	/* force */
	msg = ( spi_config_msg *)p_arg;
	/* io config */
	PIO_Configure(spi_pins, PIO_LISTSIZE(spi_pins));///<IO config
	/* configure */
	SPI_Configure(SPI0, ID_SPI0, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI0_CS2 ))); ///<loopback enable, cs3 enable
	/* configure */
	SPI_Configure(SPI0, ID_SPI0, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI0_CS3 ))); ///<loopback enable, cs3 enable
  /* config cs */
	SPI_ConfigureNPCS( SPI0,
			SPI0_CS2,
//	    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 1000, BOARD_MCK ) |
			SPI_DLYBS(1000, BOARD_MCK) |
			SPI_SCBR( msg->freq, BOARD_MCK) ); ///<config clock 50000 ,chip select	
  /* config cs */
	SPI_ConfigureNPCS( SPI0,
			SPI0_CS3,
//	    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 1000, BOARD_MCK ) |
			SPI_DLYBS(1000, BOARD_MCK) |
			SPI_SCBR( msg->freq, BOARD_MCK) ); ///<config clock 50000 ,chip select
	/* config cs mode */
	SPI_ConfigureCSMode(SPI0,SPI0_CS2,1);//CS controlled by last transfer.	
	SPI_ConfigureCSMode(SPI0,SPI0_CS3,1);//CS controlled by last transfer.
//	/* enable it */
//	SPI_EnableIt(SPI0, SPI_IER_RDRF);
	/* enable */
  SPI_Enable(SPI0);
	/* SPI1 */
	PIO_Configure(spi1_pins, PIO_LISTSIZE(spi1_pins));///<IO config
	/* configure */
	SPI_Configure(SPI1, ID_SPI1, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI1_CS0 ))); ///<loopback enable, cs3 enable
	/* cs1~3 */
		/* configure */
	SPI_Configure(SPI1, ID_SPI1, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI1_CS1 ))); ///<loopback enable, cs3 enable
		/* configure */
	SPI_Configure(SPI1, ID_SPI1, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI1_CS2 ))); ///<loopback enable, cs3 enable
	/* configure */
	SPI_Configure(SPI1, ID_SPI1, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS///<enable pmc clock,reset spi,disable spi
					    | SPI_PCS( SPI1_CS3 ))); ///<loopback enable, cs3 enable							
  /* config cs */
	SPI_ConfigureNPCS( SPI1,
			SPI1_CS0,
//	    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 100, BOARD_MCK ) |
			SPI_DLYBS(100, BOARD_MCK) |
			SPI_SCBR( 5000000, BOARD_MCK) ); ///<config clock 50000 ,chip select
			  /* config cs */
	SPI_ConfigureNPCS( SPI1,
			SPI1_CS1,
//	    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 100, BOARD_MCK ) |
			SPI_DLYBS(100, BOARD_MCK) |
			SPI_SCBR( 5000000, BOARD_MCK) ); ///<config clock 50000 ,chip select
			  /* config cs */
	SPI_ConfigureNPCS( SPI1,
			SPI1_CS2,
	//    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 100, BOARD_MCK ) |
			SPI_DLYBS(100, BOARD_MCK) |
			SPI_SCBR( 5000000, BOARD_MCK) ); ///<config clock 50000 ,chip select
			  /* config cs */
	SPI_ConfigureNPCS( SPI1,
			SPI1_CS3,
//    SPI_CSR_CPOL |
	    SPI_CSR_NCPHA |
			SPI_DLYBCT( 100, BOARD_MCK ) |
			SPI_DLYBS(100, BOARD_MCK) |
			SPI_SCBR( 5000000, BOARD_MCK) ); ///<config clock 50000 ,chip select
	/* config cs mode */
	SPI_ConfigureCSMode(SPI1,SPI1_CS0,1);//CS controlled by last transfer.
	SPI_ConfigureCSMode(SPI1,SPI1_CS1,1);//CS controlled by last transfer.
	SPI_ConfigureCSMode(SPI1,SPI1_CS2,1);//CS controlled by last transfer.
	SPI_ConfigureCSMode(SPI1,SPI1_CS3,1);//CS controlled by last transfer.
//	/* enable it */
//	SPI_EnableIt(SPI0, SPI_IER_RDRF);
	/* enable */
  SPI_Enable(SPI1);
	/* return */
	return FS_OK;
}

/* file opearation */
struct file * spi_dev_open(FAR struct file * filp)
{
	spi.flip.f_oflags = __FS_IS_INODE_OK;
	spi_default_config();
	return &spi.flip;
}
/*------------------------------------------------------------
 *            spi driver interface
 * ---------------------------------------------------------*/
/* spi driver write */
int spi_wtire(int type,void *buffer,int width,unsigned int size)
{
   int i;
   char *p_buffer;
   switch(type)
   {
      case 0:
      case 1:
      case 2:
      case 3:
    	  /* force */
    	  p_buffer = (char *)buffer;
    	  /* send without dma */
    	  for ( i = 0 ; i< size ; i++ )
    	  {
    	     SPI_Write( SPI0 , type , p_buffer[i] );
    	  }
    	  break;
      case 4:
      case 5:
      case 6:
      case 7:
    	  /* force */
    	  p_buffer = (char *)buffer;
    	  /* send without dma */
    	  for ( i = 0 ; i< size ; i++ )
    	  {
    	     SPI_Write( SPI1 , type - 4 , p_buffer[i] );
    	  }
    	  break;
      default:break;
   }
   /* return */
   return FS_OK;
}
/* spi driver read */
int spi_read(int type,void *buffer,int width,unsigned int size)
{
   int i;
   char *p_buffer;
   switch(type)
   {
      case 0:
      case 1:
      case 2:
      case 3:
    	  /* force */
    	  p_buffer = (char *)buffer;
    	  /* send without dma */
    	  for( i = 0 ; i< size ; i++ )
    	  {
    	     p_buffer[i] = SPI_Write( SPI0 , type , 0xff);//
				}
    	  break;
      case 4:
      case 5:
      case 6:
      case 7:
    	  /* force */
    	  p_buffer = (char *)buffer;
    	  /* send without dma */
    	  for( i = 0 ; i< size ; i++ )
    	  {
    	     p_buffer[i] = SPI_Write( SPI1 , type - 4 , 0xff);//
    	  }
    	  break;
      default:break;
   }
   /* return */
   return FS_OK;
}
/* int chip selectchip 
*/
int spi_selectchip(int type,char status)
{
   	switch(type)
		{
      case 0:
      case 1:
      case 2:
      case 3:
				if(status)
				{
				//	PIO_Set(&spi_pins[3]);
					SPI_ReleaseCS(SPI0);
				}else
				{
					PIO_Clear(&spi_pins[3]);
				}
				break;
			case 4:
			case 5:
      case 6:
      case 7:
				if(status)
				{
				//	PIO_Set(&spi_pins[3]);
					SPI_ReleaseCS(SPI1);
				}else
				{
					PIO_Clear(&spi_pins[3]);
				}
				break;
			default:break;
		}
		return FS_OK;
}

int spi_handler(int type,void * data,int len)
{
//	read_data = SPI_Read(SPI0);
//	read_flag = 1;
	return 0;
}




















