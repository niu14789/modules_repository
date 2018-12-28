/** 
* @file      main.c 
* @brief     ���������. 
* @details  ��������ڣ�Ӳ����ʼ����ϵͳ����. 
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       ���ڷ�������˿Ƽ����޹�˾��������
*/  
#include "stdio.h"
#include "fs.h"
#include "flashd.h"
#include "efc.h"
#include "board_e70_xpld.h"
#include "string.h"

int flash_ops(int fwsize);
void flashd_heap_init(void);

int main(void)
{
	/* dummy */
	return FS_OK;
}

FS_REGISTER_ENTRANCE(flash_ops,__FS_NORMAL);
/* flash write */
static int fd_write(int addr,void *buffer,int width,unsigned int size)
{
	int ret;
	/*--------------------*/
	if( addr > IFLASH_SIZE / 2 )
	{
		/* rang limted */
		return FS_ERR;
	}
	/* write fd */
	ret = FLASHD_Write( IFLASH_ADDR + addr , buffer, size );
	/* write ok ? */
	if( ret != 0 )
	{
		return FS_ERR;
	}
	/* write ok */
	return FS_OK;	
}
/* main prccess */
int flash_ops(int fwsize)
{	
	/* stack */
	int ret;
	unsigned char *read_buffer , *write_buffer;
	/* flash ops init */
	volatile unsigned char ucError;
	unsigned int dwLastPageAddress;
	/* heap init */
	flashd_heap_init();
	/* mpu init */
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
	ucError = FLASHD_Unlock(dwLastPageAddress, dwLastPageAddress + IFLASH_SIZE / 2 ,0, 0);
	/* retry */
	erase:
	/* stack init */
	ret = 0;
	/* two buffers */
	read_buffer = (unsigned char *)( IFLASH_ADDR + IFLASH_SIZE / 2 );
	write_buffer = (unsigned char *)( IFLASH_ADDR );
  /* flash ops init */
	for( int i = 0 ; i < ( fwsize / 0x4000 + 1 ) ; i ++ )
	{
		ret += FLASHD_ErasePages( IFLASH_ADDR + i * 0x4000 , 32 );
	}
	/* erase ok ? */
	if( ret != 0 )
	{
		return FS_ERR;
	}
  /* ok */
  fd_write( 0 , read_buffer , 1 , fwsize );
	/* verify flash ? */
	for( int i = 0 ; i < fwsize ; i ++ )
	{
		if( read_buffer[i] != write_buffer[i] )
		{
			goto erase;
		}
	}
	/* reset */
	NVIC_SystemReset();
	/* return */
	return FS_OK;
}




























