/** 
* @file      msc.h 
* @brief     
* @details  
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/
#ifndef _MSC_H_
#define _MSC_H_
#include "board.h"
#include "libstoragemedia.h"
#include "libsdmmc.h"

#include "MSDDriver.h"
#include "MSDLun.h"

#define USBHS_PRI                       3
#define HSMCI_PRI                       2
#define XDMAC_PRI                       1

#define MAX_LUNS            2 ///<Maximum number of LUNs which can be defined.



#define DRV_RAMDISK         0    ///< RAM disk 
#define DRV_SDMMC           0    ///< SD card 
#define DRV_FLASH           1
//#define DRV_NAND            2    ///< Nand flash 

#define BLOCK_SIZE          512 ///<Size of one block in bytes

#define MSD_BUFFER_SIZE     (128 * BLOCK_SIZE) ///<Size of the MSD IO buffer in bytes (150K, more the better)

void msc_init(void);
#endif




