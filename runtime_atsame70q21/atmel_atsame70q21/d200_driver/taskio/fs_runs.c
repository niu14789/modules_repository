/*
 * task.c
 *
 *  Created on: 2017-5-19
 *      Author: YJ-User17
 */
#include "fs.h"
#include "f_shell.h"
#include "tc.h" 
#include "runtime.h"
#include "string.h"
#include "fs_config.h"
/* steaed */
static struct shell_cmd * p_shell_link_base = (void*)0;
static struct shell_cmd * p_timer_link_base[16];
/* export the p_timer_link_base */
FS_SHELL_REGISTER(p_timer_link_base);
/* heap init*/
void fs_runs_init(void)
{
	p_shell_link_base = 0;
	memset(p_timer_link_base,0,sizeof(p_timer_link_base));
	FS_SHELL_INIT(p_timer_link_base,p_timer_link_base,0x040000+16,_CB_VAR_);
}
/* system shell init */
int system_shell_init(struct shell_cmd * p_shell_cmd,unsigned int max)
{
	/* for loop */
	for(int i = 0 ; i< max ; i++)
	{
		 /* case what */
	   switch(_IS_CB_TYPE_(p_shell_cmd->it_type))
	   {
				/* enable or disable */
				case _CB_TIMER_:
					if( _IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD0_ID )
					{
						 system_shell_insert(&p_timer_link_base[0],p_shell_cmd);
					}else if( _IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD1_ID )
					{
						 system_shell_insert(&p_timer_link_base[1],p_shell_cmd);
					}else if( _IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD2_ID )
					{
						 system_shell_insert(&p_timer_link_base[2],p_shell_cmd);
					}else if(_IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD3_ID )
					{
						 system_shell_insert(&p_timer_link_base[3],p_shell_cmd);
					}else if(_IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD4_ID )
					{
						 system_shell_insert(&p_timer_link_base[4],p_shell_cmd);
					}else if(_IS_CB_PARAM5_(p_shell_cmd->it_type) == TASK_PERIOD5_ID )
					{
						 system_shell_insert(&p_timer_link_base[5],p_shell_cmd);
					}else
					{ 
						/*cannot identify */
					}						 
					break;
				case _CB_EXE_:
					system_shell_insert(&p_shell_link_base,p_shell_cmd);
					break;
				case _CB_VAR_:
					system_shell_insert(&p_shell_link_base,p_shell_cmd);
					break;
				case _CB_ARRAY_:
					system_shell_insert(&p_shell_link_base,p_shell_cmd);
				  break;
				case _CB_RT_:
					system_shell_insert(&p_shell_link_base,p_shell_cmd);
					break;
				case _CB_IDLE_:
					system_shell_insert(&p_timer_link_base[6],p_shell_cmd);//idle thread
					break;
				default :break;
	   }
	   /* increate */
	   p_shell_cmd++;
	}
	return 0;
}
/* fs ioctrl */
/*----------------------------------------------*/	
__inline int shell_timer_thread(int task_id)
{
	/* default define */
	struct shell_cmd * p;
	/* default task enter */
	void (*task_enter)(void);
	/* init tnd */
	if( p_timer_link_base[task_id] == NULL )
	{
		return FS_ERR;
	}
	/* search the task list and execute them */
	for( p = p_timer_link_base[task_id] ; p != NULL ; p = p->i_child)
	{
		/* is p->enter null? */
		if(p->enter != NULL)
		{
			/* force */
			task_enter = (void (*)(void))p->enter;
			task_enter();
		}
	}	
	/* p has changed by other thread */
	if( p_timer_link_base[task_id] == NULL )
	{
		return FS_OK;
	}
	/* default peer same peer */
	for( p = p_timer_link_base[task_id]->i_peer ; p != NULL ; p = p->i_peer)
	{
		/* is p->enter null? */
		if(p->enter != NULL)
		{
			/* force */
			task_enter = (void (*)(void))p->enter;
			task_enter();
		}		
	}	
	return FS_OK;
}
/* end of file */
/* io ctrl */
int runtime_dev_ioctl(FAR struct file *filp, int cmd, unsigned long arg,void *pri_data)
{
	/* some delights */
	int ret = 0;
	/* switch cmd */
	switch(cmd)
	{
		/*
		   accomplish the link and init it 
		*/
		case 0:
			/*
		    create and insert the linker
		  */
		   ret = system_shell_init((struct shell_cmd * )pri_data,arg);
		/* break */
		break;
		/* this is a test */
		case 1:
		  ret = shell_timer_thread(arg);
		/* break */
	  break;
		/*-------------*/
		default :
			break;//does not supply this format
	}
	/* return */
	return ret;
}















