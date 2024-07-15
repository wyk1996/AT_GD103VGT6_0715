/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-23     lianzhian        first implementation.
 */
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include <board.h>

volatile uint32_t SysTickCnt = 0;
volatile uint32_t SysSecondCnt = 0;
volatile uint32_t SysMinuteCnt = 0;
volatile uint32_t SysHourCnt = 0;

/* 用于获取RTC毫秒 */
extern int32_t gsi_RTC_Counts;
extern uint32_t gui_RTC_millisecond;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
    }
    /* USER CODE END Error_Handler */
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
    NVIC_SetPriority(SysTick_IRQn, 0);
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    
    /* 用于获取RTC毫秒 */	
    gsi_RTC_Counts=RTC_DIVH;
    gsi_RTC_Counts=gsi_RTC_Counts<<16;
    gsi_RTC_Counts+=RTC_DIVL;
    gui_RTC_millisecond = (32767-gsi_RTC_Counts)*1000/32767;

    rt_tick_increase();
    
    SysTickCnt++;
    
    if(SysTickCnt%1000==0)
	{
		SysSecondCnt++;
	}

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief 获取毫秒时间函数
 * @param[in]  
 * @param[out]   
 * @return
 * @note       
 */
uint32_t GetTickCount( void )
{
	 return SysTickCnt;
}

/**
 * @brief 计算总秒时间函数
 * @param[in]  
 * @param[out]   
 * @return
 * @note       
 */
uint32_t GetSecondCount( void )
{
	return SysSecondCnt;
}

/**
 * @brief 计算总分函数 
 * @param[in]  
 * @param[out]   
 * @return
 * @note       
 */
uint32_t GetMinuteCount( void )
{
	static uint32_t last_sec=0;
    
	if( GetSecondCount() >= ( 60 + last_sec ) )
	{
		SysMinuteCnt++;
		last_sec = GetSecondCount();
	}
	return SysMinuteCnt;
}

/**
 * @brief 计算总小时函数
 * @param[in]  
 * @param[out]   
 * @return
 * @note       
 */
uint32_t GetHourCount( void )
{
	static uint32_t last_min=0;
    
	if( GetMinuteCount() >= (60 + last_min) )
	{
		SysHourCnt ++;
		last_min = GetMinuteCount();
	}
	return SysHourCnt;
}

/**
 * This function will initial GD32 board.
 */
void rt_hw_board_init()
{
    /* NVIC Configuration */
#define NVIC_VTOR_MASK              0x3FFFFF80
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR  = (0x10000000 & NVIC_VTOR_MASK);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08014000 */
    SCB->VTOR  = (0x08000000 & NVIC_VTOR_MASK);
#endif

    SystemClock_Config();

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_HEAP
    rt_system_heap_init((void*)HEAP_BEGIN, (void*)HEAP_END);
#endif
}

/**
 * @brief us延时(此函数不属于系统调用接口,调用此函数不会引起系统调度)
 * @param[in]  
 * @param[out]   
 * @return
 * @note 其中入口参数 us 指示出需要延时的微秒数目,
 *       这个函数只能支持低于 1 OS Tick(本系统中配置为1ms)的延时,
 *       否则 SysTick 会出现溢出而不能够获得指定的延时时间      
 */
void rt_hw_us_delay(uint32_t us)
{
    uint32_t uidelta;
	
    /* 获得延时经过的 tick 数 */
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	
    /* 获得当前时间 */
    uidelta = SysTick->VAL;
	
    /* 循环获得当前时间,直到达到指定的时间后退出循环 */
    while (uidelta - SysTick->VAL< us);
}

/*@}*/
