

#ifndef __RADIO_H__
#define __RADIO_H__

const unsigned char RADIO_READY[] = 
{0x46,0x4D,0x7E,0x01,0x01,0x7B,0xFF,0xAA,0x55};

const unsigned char RADIO_READY_FB[] = 
{0x46,0x4D,0xFE,0x00,0x01,0x11,0xAA,0x55};

const unsigned char RADIO_START_WORK[] = 
{0x46,0x4D,0x55,0x00,0x01,0x11,0xAA,0x55};

const unsigned char RADIO_RES_FB[] = 
{0x46,0x4D,0xFE,0x01,0x01,0x10,0x00,0xAA,0x55};
	
int radio_disable(int mode);
int radio_enable(int mode);
int radio_wtire(int type,void *buffer,int width,unsigned int size);
struct file * radio_dev_open(FAR struct file * filp);
int radio_config(void * p_arg , int argc);
int radio_default_config(void);
int radio_heap_init(void);
void radio_thread(void);
void radio_upgrade(void);
static void radio_ready(void);
static void radio_result(unsigned char id,unsigned char msg);
static void frame_prase(unsigned char c);
static void radio_process(unsigned char * buffer , unsigned int len );
static int radio_firmware_send(void);
#endif
