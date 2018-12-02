/*
 * task.h
 *
 *  Created on: 2017Äê5ÔÂ19ÈÕ
 *      Author: YJ-User17
 */

#ifndef __TASK_TASK_H__
#define __TASK_TASK_H__
 
int task_heap_init(void);
int task_default_config(void);
int task_config(void * p_arg , int argc);
int system_start(int argc);
int disable_all_it(int argc);
void fs_runs_init(void);
__inline int shell_timer_thread(int task_id);

void task0_handler(void);
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);
void task5_handler(void);

typedef struct
{
	unsigned int head;
	unsigned int vector;
	void * enter;
	unsigned int tail;
}isr_handler_def;


#endif /* __TASK_TASK_H__ */
