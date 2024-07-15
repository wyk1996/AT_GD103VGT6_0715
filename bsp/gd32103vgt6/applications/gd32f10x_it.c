/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file gd32f10x_it.c
 * @note 
 * @brief interrupt service routines
 * 
 * @author   
 * @date     2020-12-01
 * @version  V1.0.0
 * 
 * @Description 
 *  
 * @note History:        
 * @note     <author>   <time>    <version >   <desc>
 * @note  
 * @warning
 *******************************************************************************/
#include "gd32f10x_it.h"
#include "drv_rtc.h"
extern __IO uint32_t timedisplay;

/**
 * @brief this function handles NMI exception
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void NMI_Handler(void)
{
}

/**
 * @brief this function handles MemManage exception
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/**
 * @brief this function handles BusFault exception
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/**
 * @brief this function handles UsageFault exception
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/**
 * @brief this function handles SVC exception
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void SVC_Handler(void)
{
}

/**
 * @brief this function handles DebugMon exception
 * @param[in]  
 * @param[out]   
 * @return
 * @note
 */
void DebugMon_Handler(void)
{
}
/**
 * @brief 系统中断优先级配置(抢占优先级都为0，响应优先级为0-16)
 * @param[in]   
 * @param[out] 
 * @return
 * @note 0 bits for pre-emption priority 4 bits for subpriority
 */
void nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
}

/**
 * @brief RTC中断处理函数
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void RTC_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_SECOND) != RESET)
	{
        /* clear the RTC second interrupt flag*/
        rtc_flag_clear(RTC_FLAG_SECOND);

        /* enable time update */
        timedisplay = 1;

        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
		
        /* reset RTC counter when time is 23:59:59 */
        if (rtc_counter_get() == 0x00015180)
		{
            rtc_counter_set(0x0);
			
            /* wait until last write operation on RTC registers has finished */
            rtc_lwoff_wait();
        }
    }
}
