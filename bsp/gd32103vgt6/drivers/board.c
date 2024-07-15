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

/* ���ڻ�ȡRTC���� */
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
    
    /* ���ڻ�ȡRTC���� */	
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
 * @brief ��ȡ����ʱ�亯��
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
 * @brief ��������ʱ�亯��
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
 * @brief �����ֺܷ��� 
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
 * @brief ������Сʱ����
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
 * @brief us��ʱ(�˺���������ϵͳ���ýӿ�,���ô˺�����������ϵͳ����)
 * @param[in]  
 * @param[out]   
 * @return
 * @note ������ڲ��� us ָʾ����Ҫ��ʱ��΢����Ŀ,
 *       �������ֻ��֧�ֵ��� 1 OS Tick(��ϵͳ������Ϊ1ms)����ʱ,
 *       ���� SysTick �������������ܹ����ָ������ʱʱ��      
 */
void rt_hw_us_delay(uint32_t us)
{
    uint32_t uidelta;
	
    /* �����ʱ������ tick �� */
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	
    /* ��õ�ǰʱ�� */
    uidelta = SysTick->VAL;
	
    /* ѭ����õ�ǰʱ��,ֱ���ﵽָ����ʱ����˳�ѭ�� */
    while (uidelta - SysTick->VAL< us);
}

/*@}*/
