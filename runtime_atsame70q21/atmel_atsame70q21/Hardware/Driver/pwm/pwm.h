/*
 * pwm.h
 *
 *  Created on: 2017年5月17日
 *      Author: YJ-User17
 */

#ifndef __PWM_PWM_H__
#define __PWM_PWM_H__

int pwm_heap_init(void);
int pwm_default_config(void);
int pwm_config(void * p_arg , int argc);
int pwm_wtire(int type,void *buffer,int width,unsigned int size);
int pwm_enable(int mode);
int pwm_disable(int mode);
#endif /* QUADROTOR_PROFESSIONAL_APP_QUADROTOR_PROFESSIONAL_APP_QUADROTOR_PROFESSIONAL_APP_HARDWARE_DRIVER_PWM_PWM_H_ */
