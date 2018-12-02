
#ifndef __FS_EMMC_H__
#define __FS_EMMC_H__

#define AllOCSIZE        4096

int emmc_heap_init(void);
int emmc_default_config(void);
int emmc_callback(int type,void * data,int len);
/* from emmc.c */
void emmc_file_init(void);
void sdmmc_head_init(void);
void mcid_dma_heap_init(void);
void diskio_heap_init(void);
void ff_heap_init(void);
void emmc_init(void);
int FatFs_init(void);
int usb_handler(int type,void * data,int len);
/* from usb */
void msc_heap_init(void);
void usbddriverdescriptors_heap(void);
void hiddtransferdriver_heap_init(void);
void msdfunction_heap_init(void);
void usbd_hal_heap(void);
void usbd_heap_init(void);
void usbddriver_heap_init(void);
void msdlun_heap_init(void);

#endif












