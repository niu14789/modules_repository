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

int main(void);

FS_REGISTER_ENTRANCE(main,__FS_NORMAL);

int main(void)
{	
  system_initialization();

	return (int)inode_sched_getfiles();
}
















