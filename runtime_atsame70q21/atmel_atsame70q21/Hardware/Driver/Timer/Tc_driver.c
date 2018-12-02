#include "Tc_driver.h"
#include "main.h"
#include "hw_board.h"
#include "tc.h"
#include <stdio.h>
volatile uint64_t microSecond = 0;

void us_tick_tc_init(void)
{
	PMC_ConfigurePCK6(1, 11<<4);//1:Main Clock 12MHz  12·ÖÆµ PCK6 1MHz
	PMC_EnablePeripheral(ID_TC0);	
	TC_Configure(TC0, 0, 0);
	NVIC_ClearPendingIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn ,5);
	NVIC_EnableIRQ(TC0_IRQn);
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_COVFS;
	TC_Start(TC0, 0);
}

void TC0_Handler(void)
{
	uint32_t dummy;
	dummy = TC0->TC_CHANNEL[0].TC_SR;
	(void)dummy;
	microSecond += 65535;
}

uint64_t HAL_GetMicros(void)
{
	return microSecond + TC0->TC_CHANNEL[0].TC_CV;
}
