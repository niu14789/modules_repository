

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

/* command for factory */

#define MAVLINK_MSG_COMMAND_ACK                 77

#define MAVLINK_CMD_FACTORY_CMD_1             (251)
#define MAVLINK_CMD_FACTORY_CMD_2             (252)
#define MAVLINK_CMD_FACTORY_CMD_3             (253)
#define MAVLINK_CMD_FACTORY_CMD_4             (254)
#define MAVLINK_CMD_FACTORY_CMD_5             (255)
#define MAVLINK_CMD_FACTORY_CMD_6             (256)
#define MAVLINK_CMD_FACTORY_CMD_7             (257)
#define MAVLINK_CMD_FACTORY_CMD_8             (258)

#define MAVLINK_MSG_ID_RMP                    (252)

__packed typedef struct 
{
	float	param1;
	float	param2;
	float	param3;
	float	param4;
	float	param5;
	float	param6;
	float	param7;
	unsigned short command;	
	unsigned char target_system;
	unsigned char target_component;	
	unsigned char confirmation;		
}command_long_def;

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
void aging_heap_init(void);
void aging_config_default(void);
/* gs factory exit */
int gs_factory_exit(void);
void gs_factory_handle(unsigned short cmd,unsigned char * data);

#endif











