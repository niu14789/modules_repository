

#ifndef __SHELL_H__
#define __SHELL_H__

typedef struct{
	unsigned short line_num;
	unsigned short i_flags;//status
	char name[24];//4*6
	int type;//node or shell
	int size;/* function or param or others and param numbers and returns or not */
	unsigned int size_pl;
	unsigned char data[200];
}node_def;

typedef struct{
	unsigned short cmd;
	unsigned short line_num;
	unsigned char line_gid[10][2];
}LINE_CONFIG_DEF;

#define ALL_NAME             0xff
#define ALL_SHELL_NAME       0xfe
#define ALL_DETAILS          0xfd
#define CMD                  0xfc
#define PIC                  0xfb
#define MDO                  0xfa
#define ONE_NODE_DETAILS     0x01
#define ONE_SHELL_DETAILS    0x02

#define GETDATA   "get"

static int shell_default_config(void);
static int shell_heap_init(void);
void shell_thread(void);
int shell_simulation_pushlish(float * simulation_data,unsigned int status);
int publish_all_name(unsigned int dummy);
unsigned short export_version( unsigned int index );
void factory_unit( unsigned char * data , unsigned char len );
void line_task(void);
int factory_callback(int type,void * data,int len);
void gs_simple_task(void);
#endif











