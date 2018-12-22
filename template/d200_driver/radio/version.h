

#ifndef __version_KEY_H__
#define __version_KEY_H__

typedef struct{
	char  *id;
	unsigned short version_m;//modules version
	unsigned short version_u;//updated version
	unsigned short status; 
	unsigned short dev_id;
	unsigned short index;
	unsigned short up;
	unsigned int fw_size;
	unsigned int now_addr;
	unsigned int offset;
}modules_status;

int version_heap_init(void);
int version_default_config(void);
void version_thread(void);
int gs_callback(int type,void * data,int len);
void time_stamp_ms(void);

#endif

















 


















