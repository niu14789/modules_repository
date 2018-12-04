

#ifndef __factory_H__
#define __factory_H__

const unsigned char factory_READY[] = 
{0x46,0x4D,0x7E,0x01,0x01,0x7B,0xFF,0xAA,0x55};

const unsigned char factory_READY_FB[] = 
{0x46,0x4D,0xFE,0x00,0x01,0x11,0xAA,0x55};

const unsigned char factory_START_WORK[] = 
{0x46,0x4D,0x55,0x00,0x01,0x11,0xAA,0x55};

const unsigned char factory_RES_FB[] = 
{0x46,0x4D,0xFE,0x01,0x01,0x10,0x00,0xAA,0x55};
	
int factory_disable(int mode);
int factory_enable(int mode);
int factory_wtire(int type,void *buffer,int width,unsigned int size);
struct file * factory_dev_open(FAR struct file * filp);
int factory_config(void * p_arg , int argc);
int factory_default_config(void);
int factory_heap_init(void);
void factory_thread(void);
void factory_upgrade(void);
static void factory_ready(void);
static void factory_result(unsigned char id,unsigned char msg);
static void frame_prase(unsigned char c);
static void factory_process(unsigned char * buffer , unsigned int len );
static int factory_firmware_send(void);
#endif
