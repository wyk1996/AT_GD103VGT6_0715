/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file drv_iwdg.h
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
#ifndef _DRV_IWDG_H_
#define _DRV_IWDG_H_

#define IWDG_DEVICE_NAME        "wdt"
#define IWDG_RELOAD_VALUE       0x0fff
#define IWDG_PRESCALER_DIV      FWDGT_PSC_DIV128

int iwdg_thread(void);

#endif
