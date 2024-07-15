/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file drv_iwdg.c
 * @note 
 * @brief 
 * 
 * @author   
 * @date     2021-08-23
 * @version  V1.0.0
 * 
 * @Description 
 *  
 * @note History:        
 * @note     <author>   <time>    <version >   <desc>
 * @note  
 * @warning
 *******************************************************************************/
#include <rtdevice.h>

#ifdef RT_USING_WDT

#include "drv_iwdg.h"
#include "gd32f10x.h"

static rt_device_t iwdg_dev;
static rt_uint8_t is_start;
static struct rt_watchdog_ops ops;
static rt_watchdog_t watchdog;

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
ErrStatus rcu32k_config(void)
{
    ErrStatus reval = ERROR;
    rcu_osci_on(RCU_LXTAL);
    reval= rcu_osci_stab_wait(RCU_LXTAL);
    return reval;
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
ErrStatus IWDG_Init(void)
{
    ErrStatus reval = ERROR;
    rcu_osci_on(RCU_LXTAL);
    reval= rcu_osci_stab_wait(RCU_LXTAL);    
    fwdgt_config(IWDG_RELOAD_VALUE,IWDG_PRESCALER_DIV);
    fwdgt_enable();
    return reval;
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
void IWDG_Feed(void)
{
    fwdgt_counter_reload();
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
static void iwdg_feed(void)
{
    rt_device_control(iwdg_dev,RT_DEVICE_CTRL_WDT_KEEPALIVE,NULL);
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
int iwdg_thread(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t timeout = 10;
    
    iwdg_dev = rt_device_find(IWDG_DEVICE_NAME);
    if(!iwdg_dev)
    {
        rt_kprintf("find %s failed!\n",IWDG_DEVICE_NAME);
        return RT_ERROR;
    }
    
    ret = rt_device_init(iwdg_dev);
    if(ret != RT_EOK)
    {
        rt_kprintf("initialite %s failed!\n",IWDG_DEVICE_NAME);
        return RT_ERROR;
    }
    
    ret = rt_device_control(iwdg_dev,RT_DEVICE_CTRL_WDT_SET_TIMEOUT,&timeout);
    if(ret != RT_EOK)
    {
        rt_kprintf("set %s tiemout failed!\n",IWDG_DEVICE_NAME);
        return RT_ERROR;
    }
    
    rt_thread_idle_sethook(iwdg_feed);
    
    return ret;
}

INIT_APP_EXPORT(iwdg_thread);

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
static rt_err_t wdt_init(rt_watchdog_t *wdt)
{
    return RT_EOK;
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
static rt_err_t wdt_contorl(rt_watchdog_t *wdt,int cmd,void *arg)
{
	switch(cmd)
	{
		case RT_DEVICE_CTRL_WDT_KEEPALIVE:
			fwdgt_counter_reload();
			break;
		case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
			fwdgt_config(IWDG_RELOAD_VALUE,IWDG_PRESCALER_DIV);
			if(is_start)
			{
				if(IWDG_Init() != SUCCESS)
				{
					rt_kprintf("wgt set timeout failed.");
					return -RT_ERROR;
				}
			}
			break;
		case RT_DEVICE_CTRL_WDT_GET_TIMELEFT:
			fwdgt_config((*((rt_uint32_t*)arg)),IWDG_PRESCALER_DIV);
			break;
		case RT_DEVICE_CTRL_WDT_START:
			if(IWDG_Init() != SUCCESS)
			{
				rt_kprintf("wgt set timeout failed.");
				return -RT_ERROR;
			}
			is_start = 1;
			break;
		case RT_DEVICE_CTRL_WDT_STOP:
			fwdgt_write_disable();
			break;
		default:
			rt_kprintf("This command is not supported.");
			return -RT_ERROR;
	}
	return RT_EOK;
}

/**
 * @brief 看门狗初始化
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
int rt_wdt_init(void)
{
	is_start = 0;
	
    IWDG_Init();
	
	ops.init = &wdt_init;
	
	ops.control = &wdt_contorl;
	
	watchdog.ops = &ops;

	if(rt_hw_watchdog_register(&watchdog,"wdt",RT_DEVICE_FLAG_DEACTIVATE,RT_NULL) != RT_EOK)
	{
		rt_kprintf("wdt device register failed.");
		return -RT_ERROR;
	}
	rt_kprintf("wdt device register success.");

	return RT_EOK;
}
//INIT_BOARD_EXPORT(rt_wdt_init);

#endif
