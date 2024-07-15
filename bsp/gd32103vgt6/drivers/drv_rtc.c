/*******************************************************************************
 *          Copyright (c) 2020-2050, .
 *                              All Right Reserved.
 * @file rtc.c
 * @note 
 * @brief
 * 
 * @author   
 * @date     2020-12-01
 * @version  V1.0.0
 * 
 * @Description RTC check and config,time_show and time_adjust function
 *  
 * @note History:        
 * @note     <author>   <time>    <version >   <desc>
 * @note  
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h> 
#include <time.h>
#include <board.h>
#include "drv_rtc.h"
#include "user_lib.h"
#include "common.h"

extern SYSTEM_RTCTIME 	gs_SysTime;

/* enter the second interruption,set the second interrupt flag to 1 */
__IO uint32_t timedisplay;

rt_rtc_dev_t rtc;

/**
 * @brief configure the RTC 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32767);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/**
 * @brief return the time entered by user, using Hyperterminal
 * @param[in]   
 * @param[out] 
 * @return current time of RTC counter value
 * @note
 */
uint32_t time_regulate(void)
{
    uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;

    rt_kprintf("\r\n==============Time Settings=====================================");
    rt_kprintf("\r\n  Please Set Hours");

    while (tmp_hh == 0xFF)
	{
        tmp_hh = 17;
    }
    rt_kprintf(":  %d", tmp_hh);
    rt_kprintf("\r\n  Please Set Minutes");
    while (tmp_mm == 0xFF)
	{
        tmp_mm = 25;
    }
    rt_kprintf(":  %d", tmp_mm);
    rt_kprintf("\r\n  Please Set Seconds");
    while (tmp_ss == 0xFF)
	{
        tmp_ss = 37;
    }
    rt_kprintf(":  %d", tmp_ss);

    /* return the value  store in RTC counter register */
    return((tmp_hh*3600 + tmp_mm*60 + tmp_ss));
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
static time_t get_rtc_timestamp(time_t *timer)
{
    uint32_t uiSecond = 0;
    uiSecond = rtc_counter_get();
    if (timer != NULL)
    {
        *timer = uiSecond;
    }

    return uiSecond;
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
static time_t set_rtc_time_stamp(time_t time_stamp)
{
    struct tm *p_tm;
    struct tm t;

    rt_memset(&t, 0, sizeof(t));
	
	p_tm = localtime(&time_stamp);
    
    t.tm_sec = p_tm->tm_sec;
    t.tm_min = p_tm->tm_min;
    t.tm_hour = p_tm->tm_hour;
    t.tm_mday = p_tm->tm_mday;
    t.tm_mon = p_tm->tm_mon;
	/* 从1900年开始开始计算的,如果是2018年,这时p_tm->tm_year的值是118 */
    t.tm_year = p_tm->tm_year;

//    rt_kprintf("%u-%u-%u %u:%u:%u", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return	(mktime(&t));
}

/**
 * @brief time_adjust
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void time_adjust(void)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(time_regulate());
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/**
 * @brief time_set
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void time_set(time_t timer)
{
	time_t tm_cnt = 0;
	
	/* 设置rtc时间戳 */
	tm_cnt = set_rtc_time_stamp(timer);
	
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(tm_cnt);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/**
 * @brief display the current time
 * @param[in] timeVar: RTC counter value   
 * @param[out] 
 * @return
 * @note
 */
void time_display(uint32_t timevar)
{
    uint32_t thh = 0, tmm = 0, tss = 0;

    /* compute  hours */
    thh = timevar / 3600;
    /* compute minutes */
    tmm = (timevar % 3600) / 60;
    /* compute seconds */
    tss = (timevar % 3600) % 60;

    rt_kprintf(" Time: %0.2d:%0.2d:%0.2d\r\n", thh, tmm, tss);
}

/**
 * @brief show the current time (HH:MM:SS) on the Hyperterminal
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void time_show(void)
{
    rt_kprintf("\n\r");

	/* if 1s has paased */
	if (timedisplay == 1)
	{
		/* display current time */
		time_display(rtc_counter_get());
		timedisplay = 0;
	}
	rt_thread_delay(1000);
}

/**
 * @brief rtc时钟初始化
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void rt_rtc_init(void)
{
	nvic_irq_enable(RTC_IRQn,0,8);

	if (bkp_data_read(BKP_DATA_0) != 0xA5A5)
	{
		/* backup data register value is not correct or not yet programmed
		(when the first time the program is executed) */
		rt_kprintf("\r\n\n RTC not yet configured....\r\n");
		
		/* RTC configuration */
		rtc_configuration();

		rt_kprintf("\r\n RTC configured....\r\n");

		/* adjust time by values entred by the user on the hyperterminal */
		time_adjust();

		bkp_data_write(BKP_DATA_0, 0xA5A5);
	}
	else
	{
		/* check if the power on reset flag is set */
		if (rcu_flag_get(RCU_FLAG_PORRST) != RESET)
		{
			rt_kprintf("\r\n\n Power On Reset occurred....\r\n");
		}
		else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET)
		{
			/* check if the pin reset flag is set */
			rt_kprintf("\r\n\n External Reset occurred....\r\n");
		}

		/* allow access to BKP domain */
		rcu_periph_clock_enable(RCU_PMU);
		pmu_backup_write_enable();

		rt_kprintf("\r\n No need to configure RTC....\r\n");
		/* wait for RTC registers synchronization */
		rtc_register_sync_wait();

		/* enable the RTC second */
		rtc_interrupt_enable(RTC_INT_SECOND);

		/* wait until last write operation on RTC registers has finished */
		rtc_lwoff_wait();
	}

	#ifdef RTCCLOCKOUTPUT_ENABLE
	/* enable PMU and BKPI clocks */
	rcu_periph_clock_enable(RCU_BKPI);
	rcu_periph_clock_enable(RCU_PMU);
	/* allow access to BKP domain */
	pmu_backup_write_enable();

	/* disable the tamper pin */
	bkp_tamper_detection_disable();

	/* enable RTC clock output on tamper Pin */
	bkp_rtc_calibration_output_enable();
	#endif

	/* clear reset flags */
	rcu_all_reset_flag_clear();

}

/**
 * @brief 获取或者设置RTC时间
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
static rt_err_t rt_rtc_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(dev != RT_NULL);
	
    switch (cmd)
    {
		/* rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time); */
		case RT_DEVICE_CTRL_RTC_GET_TIME:
			*(rt_uint32_t *)args = get_rtc_timestamp((rt_uint32_t *)args);
//			rt_kprintf("RTC: get rtc_time %x\n", *(rt_uint32_t *)args);
			break;
		/* rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &time); */
		case RT_DEVICE_CTRL_RTC_SET_TIME:
			time_set(*(rt_uint32_t *)args);
//			rt_kprintf("RTC: set rtc_time %x\n", *(rt_uint32_t *)args);
			break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops rtc_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    rt_rtc_control
};
#endif


rt_err_t rt_hw_rtc_register(rt_rtc_dev_t  *rtc,
                            const char    *name,
                            rt_uint32_t    flag,
                            void          *data)
{
    struct rt_device *device;
    RT_ASSERT(rtc != RT_NULL);

    device = &(rtc->parent);

    device->type        = RT_Device_Class_RTC;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    
    rt_rtc_init();

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &rtc_core_ops;
#else
    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_rtc_control;
#endif /* RT_USING_DEVICE_OPS */
    device->user_data   = data;

    /* register a character device */
    return rt_device_register(device, name, flag);
}

/**
 * @brief rtc驱动初始化
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
int rt_hw_rtc_init(void)
{
    rt_err_t result;
    result = rt_hw_rtc_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR,NULL);
    if (result != RT_EOK)
    {
        rt_kprintf("rtc register err code: %d", result);
        return result;
    }
    rt_kprintf("rtc init success");
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);
