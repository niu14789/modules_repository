/** 
* @file      fatfs.c 
* @brief     fatfs文件系统初始化. 
* @details   fatfs文件系统初始化，emmc挂载、格式化、分区. 
* @author   zbb 
* @date      2016-7-27
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       深圳飞马机器人科技有限公司（北京）
*/ 
//#include "fatfs.h"
#include "FatFs.h"
#include "emmc.h"
#include "libsdmmc.h"
#include "fatfs_config.h"
#include "Media.h"
#include "MEDSdcard.h"
#include <string.h>
#include <stdio.h>
#include "board.h"
#include "diskio.h"
static char SDPath1[4]="0:";
static char SDPath2[4]="1:";
static char SDPath3[4]="2:";
//extern sXdmad dmaDrv;
//extern sMcid mciDrv[BOARD_NUM_MCI]; ///<MCI driver instance.
extern sMedia medias[2];
//extern  sSdCard sdDrv[BOARD_NUM_MCI]; ///<SDCard driver instance.
//static const Pin pinsSd[] = {BOARD_MCI_PINS_SLOTA, BOARD_MCI_PIN_CK};///<SD card pins instance.
//static const Pin pinsCd[] = {BOARD_MCI_PIN_CD};///<SD card detection pin instance.
uint8_t MBR_BUFF[512]={0};
DWORD plist[] = {86,14, 0, 0}; 
DWORD plist1[] = {100,0, 0, 0};
BYTE work[_MAX_SS];
#define AllOCSIZE        4096
#define DATA_SIZE 1024
uint8_t data[DATA_SIZE];
typedef struct _ALIGN_FATFS 
{
    
	uint8_t padding[16];
    FATFS fs;
	
} ALIGN_FATFS;

COMPILER_ALIGNED(32) ALIGN_FATFS aligned_fs[3];;



const char *FileName = STR_ROOT_DIRECTORY "Basic.bin";
const char *FileNameReadMe = STR_ROOT_DIRECTORY "ReadMe.txt";

const char *ReadMeText = "Samv7 FatFS example: Done!!";
#define TEST_PERFORMENCT_SIZE   (4*1024*1024) ///< Test settings: Number of bytes to test performance


#define ASSERT(condition, ...)  { \
		if (!(condition)) { \
			printf("-F- ASSERT: "); \
			printf(__VA_ARGS__); \
		} \
	}
//int file_test(void);
////extern FATFS *FatFs[_VOLUMES]; 
//FIL file_w_FIL;
//char txt[512];
//uint16_t file_w,file_w_n;
//uint32_t wsize,file_size;
//uint32_t seek_count;
extern sSdCard sdDrv[BOARD_NUM_MCI];
char file_path[16];
/**
 * funcation  FatFs_init
 * @brief fatfs初始化
 * @param 无
 * @return 无
 */
int FatFs_init(void )
{
	FRESULT res;
	
    emmc_init();
	
	res=f_fdisk(0, plist, work);// 创建分区表
	
//	res = FLASHD_Erase(0x500000);
//	res=f_fdisk(1, plist1, work);// 创建分区表
	if(res !=FR_OK )
	{
		printf("fen qu  shibai\n");
	}
	memset(&aligned_fs[0].fs, 0, sizeof(FATFS));
	memset(&aligned_fs[1].fs, 0, sizeof(FATFS));
//	
//	FatFs[0] = &aligned_fs[0].fs;//文件系统初始化
//	FatFs[1] = &aligned_fs[1].fs;
//	FatFs[2] = &aligned_fs[2].fs;
//	

	
	res = f_mount(&aligned_fs[0].fs, (TCHAR const*)SDPath1, 1);
  if(res != FR_OK)
	{
			if (f_mkfs("0:", 0, AllOCSIZE)!=FR_OK) 
			{
				printf("geshihua shibai\n");
			}
			res = f_mount(&aligned_fs[0].fs, (TCHAR const*)SDPath1, 1);
			if(res !=FR_OK )
			{
				printf("gua zai shibai\n");
			}
	}
			
	res = f_mount(&aligned_fs[1].fs, (TCHAR const*)SDPath2, 1);
	if(res != FR_OK)
	{
			if (f_mkfs("1:", 0, AllOCSIZE)!=FR_OK) 
			{
				printf("geshihua shibai\n");
			}
			res = f_mount(&aligned_fs[1].fs, (TCHAR const*)SDPath2, 1);
			if(res !=FR_OK )
			{
				printf("gua zai shibai\n");
			}
	}


	res = f_mount(&aligned_fs[2].fs, (TCHAR const*)SDPath3, 1);
  if(res != FR_OK)
	{
			if (f_mkfs("2:", 0, AllOCSIZE)!=FR_OK) 
			{
				printf("geshihua shibai\n");
			}
			res = f_mount(&aligned_fs[2].fs, (TCHAR const*)SDPath3, 1);
			if(res !=FR_OK )
			{
				printf("gua zai shibai\n");
			}
	}
//    memset(txt,110,512);
//    for(uint8_t addr=0; addr<10; addr++)
//    {
//        for(uint16_t i=0; i<512; i++)
//        {
//            txt[i]++;
//        }
//        SD_Write(&sdDrv[0], addr, (void*)txt, 1, NULL, NULL);
//    }
//               
//    for(uint8_t addr1=0; addr1<10; addr1++)
//    {
//        SD_Read(&sdDrv[0], addr1, (void*)txt, 1, NULL, NULL);
//        
//        for(uint16_t i=0; i<512; i++)
//        {
//            if(i%16==0)
//            printf("\n");
//            printf("%d",txt[i]);
//            
//        }
//    }

//  memset(txt,0,512);
//    res = f_mkdir("0:/file_w");
//    for(file_w=0; file_w<30; file_w++)
//    {
////        for(uint16_t i=0; i<512; i++)
////        {
////            txt[i]++;
////        }
//        sprintf(file_path,"0:/file_w/%u.BIN", (unsigned)file_w);
//        memset((void *)&file_w_FIL,0, sizeof(file_w_FIL));
//        
//        res = f_open(&file_w_FIL,file_path, FA_OPEN_ALWAYS | FA_WRITE ); 
//        seek_count = f_size(&file_w_FIL);
//        printf("file id %d  size %d\n", file_w, seek_count);
//        res = f_lseek(&file_w_FIL, seek_count);
//        
//        for(file_w_n=0;file_w_n<100;file_w_n++)
//        {
//            for(uint16_t i=0; i<512; i++)
//            {
//                txt[i]=100;
//            }
//            res = f_write(&file_w_FIL,txt,45,(UINT*)&wsize);
//            
//               
//        }

//        res = f_close(&file_w_FIL);  
//        res = f_open(&file_w_FIL,file_path, FA_OPEN_ALWAYS | FA_READ ); 
//        file_size = f_size(&file_w_FIL);
//        printf("file id %d  size %d\n",file_w, file_size);
//        for(file_w_n=0;file_w_n<100;file_w_n++)
//        {

//            res = f_read(&file_w_FIL,txt,sizeof(txt),(UINT*)&wsize);
//            
//            for(uint16_t i=0; i<512; i++)
//            {
//                if(i%16==0)
//                printf("\n");
//                printf("%d ",txt[i]);
//                
//            }    
//        }
//        
//        res = f_close(&file_w_FIL); 
        
//   }
    
   

return 1;
}
/**
 * funcation scan_files
 * @brief Scan files under a certain path
 * @param path    folder path
 * @return scan result, 1: success.
 */
static FRESULT scan_files (char *path)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int32_t i;
	char *fn;
#if _USE_LFN
	static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif


	res = f_opendir(&dir, path);

	if (res == FR_OK) {
		i = strlen(path);

		for (;; ) {
			res = f_readdir(&dir, &fno);

			if (res != FR_OK || fno.fname[0] == 0) break;

#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif

			if (*fn == '.') continue;

			if (fno.fattrib & AM_DIR) {
				sprintf(&path[i], "/%s", fn);
				res = scan_files(path);

				if (res != FR_OK) break;

				path[i] = 0;
			} else
				printf("%s/%s\n\r", path, fn);
		}
	}

	return res;
}
/**
 * funcation file_test
 * @brief 文件系统测试函数
 * @param 无
 * @return 测试结果
 */
int file_test(void)
{
	FRESULT res;
	DIR dirs;
 FIL FileObject;
	uint32_t ByteWritten;
	res = f_opendir (&dirs, STR_ROOT_DIRECTORY);

	if (res == FR_OK) 
	{
      res = f_open(&FileObject, FileName, FA_CREATE_ALWAYS | FA_WRITE);
		if(res == FR_OK)
		{
			res = f_write(&FileObject, data, DATA_SIZE, (UINT*)&ByteWritten);
		}
		res = f_close(&FileObject);
	}


	return res;
}




