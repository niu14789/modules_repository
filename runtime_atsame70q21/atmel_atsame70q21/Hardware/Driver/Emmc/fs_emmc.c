/* file header
 *
 * emmc fs file 
 * niu
 */
#include "fs.h"
#include "fs_emmc.h"
#include "string.h"
#include "board.h"
#include "ff.h"

 /* fs inode system register */
FS_INODE_REGISTER("/sdcard/",emmc,emmc_heap_init,0);
/* register shell interface */
FS_SHELL_REGISTER(emmc_callback);//usart 0
FS_SHELL_REGISTER(usb_handler);//usart 0
/* static init param */
const DWORD plist[] = {86,14, 0, 0}; 
BYTE work[_MAX_SS];
FATFS fs;
FIL fsrc;
UINT  bw;
char buffer[16];
/* heap init */
int emmc_heap_init(void)
{
	/* full of zero */
	memset(&emmc,0,sizeof(emmc));
	/* shell init */
	emmc.shell_i = shell_sched_getfiles();
	emmc.config = emmc_default_config;
	/* file ops */
	emmc.ops.write = NULL;
	/* dirver */
	emmc.drv_ops.config = NULL;
	/* file interface */
	emmc.flip.f_inode = &emmc;
	emmc.flip.f_path = "/sdcard/";
	/* shell */
	FS_SHELL_INIT(emmc_callback,emmc_callback,0,_CB_IT_|_CB_PRIORITY_(4)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(HSMCI_IRQn)|_CB_ENABLE_);
	FS_SHELL_INIT(usb_handler,usb_handler,0,_CB_IT_|_CB_PRIORITY_(2)|_CB_IT_TPYE_(0)|_CB_IT_IRQN_(USBHS_IRQn)|_CB_ENABLE_);
	/* lib files that need init */
	emmc_file_init();
	sdmmc_head_init();
	mcid_dma_heap_init();
	diskio_heap_init();
	ff_heap_init();
	msc_heap_init();
	usbddriverdescriptors_heap();
	hiddtransferdriver_heap_init();
	msdfunction_heap_init();
	usbd_hal_heap();
	usbd_heap_init();
	usbddriver_heap_init();
	msdlun_heap_init();
	/* static init param */
	memset(work,0,sizeof(work));
	memset(&fs,0,sizeof(fs));
	memset(&fsrc,0,sizeof(fsrc));
	/* add your own code here */
	
	/* end of init */
	return 0;
}
extern uint8_t usb_mode;
unsigned int p_sync;
int emmc_default_config(void)
{
	//FatFs_init();
	shell_read("memcync_shell",&p_sync,4,0);
	  usb_mode = 2;
	emmc_init();
	USB_mode_switch(1);
	msc_init();
	return 0;
}

int FatFs_init(void )
{
	FRESULT res;
	
    emmc_init();

	res=f_fdisk(0, plist, work);// ����������
	
//	res = FLASHD_Erase(0x500000);
//	res=f_fdisk(1, plist1, work);// ����������
	if(res !=FR_OK )
	{
	//	printf("fen qu  shibai\n");
	}
	memset(&fs, 0, sizeof(FATFS));
//	
//	FatFs[0] = &aligned_fs[0].fs;//�ļ�ϵͳ��ʼ��
//	FatFs[1] = &aligned_fs[1].fs;
//	FatFs[2] = &aligned_fs[2].fs;
//	
	
	res = f_mount(&fs,"0:", 1);
  if(res != FR_OK)
	{
			if (f_mkfs("0:", 0, AllOCSIZE)!=FR_OK) 
			{
//				printf("geshihua shibai\n");
			}
			res = f_mount(&fs,"0:", 1);
			if(res !=FR_OK )
			{
		//		printf("gua zai shibai\n");
			}
	}
	
	res = f_open(&fsrc,"0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE ); 
	
	if(res == FR_OK)
	{
		memcpy(buffer,"0123456789abcdef",16);
		f_write(&fsrc,buffer,sizeof(buffer),&bw);
		f_sync(&fsrc);
		f_close(&fsrc);
	}
//	{
////		memcpy(buffer,"0123456789abcdef",16);
////		
////		res = f_write(&fsrc,buffer,sizeof(buffer),&bw);
////		
////		if( res == FR_OK && bw == 16)
////		{
////			memset(buffer,0,sizeof(buffer));
//			f_read(&fsrc,buffer,sizeof(buffer),&bw);
////		}
//	}
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




























































