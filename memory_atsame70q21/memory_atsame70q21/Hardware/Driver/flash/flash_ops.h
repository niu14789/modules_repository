#ifndef __FLASH_OPS_H__
#define __FLASH_OPS_H__

static int flash_heap_init(void);
static int fd_write(int addr,void *buffer,int width,unsigned int size);
static int fd_manage( void * p_arg , int argc );
static int backup_plan(int fw_size);
#endif
