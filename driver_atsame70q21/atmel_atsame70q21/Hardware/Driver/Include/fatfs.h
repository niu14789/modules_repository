/** 
* @file      fatfs.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       ���ڷ�������˿Ƽ����޹�˾��������
*/
#ifndef _FATFS_H_
#define _FATFS_H_

#define ID_DRV DRV_MMC
#define STR_ROOT_DIRECTORY "0:"

int FatFs_init(void);
int file_test(void);
#endif




