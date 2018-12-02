/*-------------------------------------------------------------*/
#include "fs.h"
#include "f_shell.h"
#include "f_ops.h"
#include "flash_ops.h"
#include "memory.h"
#include "flashd.h"
#include "efc.h"
#include "board_e70_xpld.h"
#include "string.h"
/* declare the file heaper */
static struct file * d200f_open(struct file * filp);
static int fd_ioctl(struct file *filp, int cmd, unsigned long arg,void *pri_data);
static int flash_default_config(void);
/*--------------------------*/
static unsigned char flash_config;
/* fs inode system register */
FS_INODE_REGISTER("/flash/d200f/",d200f,flash_heap_init,0);
/*------*/
FS_SHELL_REGISTER(flash_config);
/* heap init */
int flash_heap_init(void)
{
	/* flash ops init */
	volatile unsigned char ucError;
	unsigned int dwLastPageAddress;
	/* do the heap init . first time*/
	/* full of zero */
	memset(&d200f,0,sizeof(d200f));
	/* shell init */
	d200f.shell_i = shell_sched_getfiles();
	d200f.config = flash_default_config;
	/* file ops */
	d200f.ops.open = d200f_open;
	d200f.ops.ioctl = fd_ioctl;
	/* dirver */
	d200f.drv_ops.config = NULL;
	d200f.drv_ops.write = fd_write;
	d200f.drv_ops.config = fd_manage;
	/* file interface */
	d200f.flip.f_inode = &d200f;
	d200f.flip.f_path = "/storage/d200f/";
	/* shell init */
	FS_SHELL_INIT(flash_config,flash_config,0x010001 , _CB_VAR_);
	/* Update internal flash Region to Full Access*/
	MPU_UpdateRegions(MPU_DEFAULT_IFLASH_REGION, IFLASH_START_ADDRESS, \
		MPU_AP_FULL_ACCESS |
		INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
		MPU_CalMPURegionSize(IFLASH_END_ADDRESS - IFLASH_START_ADDRESS) |
		MPU_REGION_ENABLE);
	/* Set 6 WS for internal Flash writing (refer to errata) */
	EFC_SetWaitState(EFC, 6);
	/* Initialize flash driver */
	FLASHD_Initialize(BOARD_MCK, 0);
	/* Performs tests on last page (to avoid overriding existing program).*/
	dwLastPageAddress = IFLASH_ADDR;
	/* Unlock page */
	ucError = FLASHD_Unlock(dwLastPageAddress, dwLastPageAddress + IFLASH_SIZE ,0, 0);
	/* unlock ok ? */
	if( ucError != 0 )
	{
		/* -- */
		return FS_ERR;
	}
	/* init ok */
	return FS_OK;
}
/**/
static int flash_default_config(void)
{
	/* some stack */
	return FS_OK;
}
/*-----------------------------------------------------------------------
*    file init
------------------------------------------------------------------------*/
static struct file * d200f_open(struct file * filp)
{
	/* file ofilp */
	if( filp->f_oflags == ( __FS_OPEN_EXISTING | __FS_DIRTY ) )
	{
		/* only open the driver */
		return &d200f.flip;
	}
	/* not supply file system now */
	return NULL;
}
/*-----------------------------------------------------------------------
* diver ops
------------------------------------------------------------------------*/
/*
* fd write 
*/
static int fd_write(int addr,void *buffer,int width,unsigned int size)
{
	int ret;
	/*--------------------*/
	if( addr > IFLASH_SIZE / 2 )
	{
		/* rang limted */
		return FS_ERR;
	}
	/* width */
	if( width == 1 )
	{
		/* write fd */
		ret = FLASHD_Write( IFLASH_ADDR + IFLASH_SIZE / 2 + addr , buffer, size );
	}else if( width == 2 )
	{
		/* write fd */
		ret = FLASHD_Write( IFLASH_ADDR + addr , buffer, size );
	}else
	{
		return FS_ERR;
	}
	/* write ok ? */
	if( ret != 0 )
	{
		return FS_ERR;
	}
	/* write ok */
	return FS_OK;	
}
/* 
 fd ioctrl
*/
static int fd_ioctl(struct file *filp, int cmd, unsigned long arg,void *pri_data)
{
	/*--------*/
	int i,ret = FS_OK;
	/*--------*/
	switch(cmd)
	{
		case 0:/* smart erase */
			for( i = 0 ; i < arg ; i ++ )
		  {
				ret += FLASHD_EraseSector( IFLASH_ADDR + IFLASH_SIZE / 2 + i * 128 * 1024 );
		  }
			break;
		case 1:/* manul erase */
			ret = FLASHD_EraseSector( IFLASH_ADDR + IFLASH_SIZE / 2 + arg );
			break;
		case 2:/* erase specifty sector */
			ret = FLASHD_EraseSector( IFLASH_ADDR + arg );
			break;
		case 3:/* erase all the backup area */
			for( i = 0 ; i < 8 ; i ++ )
		  {
				ret += FLASHD_EraseSector( IFLASH_ADDR + IFLASH_SIZE / 2 + i * 128 * 1024 );
		  }
			break;
		default:
			break;
	}
	/* return */
	return ret;
}
/*-----------------------------------------------------------------------
* diver ops
------------------------------------------------------------------------*/
/* ----------------config -----------------*/
static int fd_manage( void * p_arg , int size )
{
	/* ------------ */
	unsigned char buffer[512];
	/*--------------*/
	int len  = 0, now  = 0 , real,segma ;
	/* define the file */
	struct file * fd;
	/* for force */
	fd = ( struct file * )p_arg;
	/*-------erase--------*/
	if( fd_ioctl( fd , 0 , size / 0x20000 + 1 , NULL ) != FS_OK )
	{
		/* erase error return */
		return FS_ERR;
	}
  /* get data */
	while(1)
	{  
		 /*-----------------*/
		 segma = size - now;
		 /* calibration */
		 real = ( segma > sizeof(buffer) ) ? sizeof(buffer) : segma;
		 /* read the buffer */
		 len = fs_read(fd,(char *)buffer,real);
		 /* write the data to flash buffer */
		 fd_write(now,buffer,1,real);
		 /* ------file size fault----- */
		 if( len != real )
		 {
				fs_close(fd);
				return FS_ERR;//file size fault
		 } 
		 /* ------ ok -------*/
		 if( real < sizeof(buffer) )
		 {
				/* firmware has over */
				/* start the backup plan */
			  shell_execute("disable_all_it",0);
				backup_plan(size);	
			  fs_close(fd);
        break;		 
		 }				 
		 /*-----addr increme-----*/
		 now += real;
	}
	return FS_OK;
}
/* for buffer */
static int backup_plan(int fw_size)
{
	/* copy the data to the specified addr */
	memcpy((void *)0x2041E000,__FS_memory,sizeof(__FS_memory));
	/* static unsigned id enter */
	inode_identify *id_enter = (inode_identify *)(0x2041E000);
  /* search the enterance */
	for(int i = 0; i < 0x1000 ; i ++)
	{
		if(id_enter->id_l == 0xAFBC562D && id_enter->id_h == 0x2b74113c)
		{
			((int(*)(int))id_enter->entrance)(fw_size);
		}
		id_enter = (inode_identify *)(((unsigned int)id_enter) + 4);
	}
	return 0;
}




































