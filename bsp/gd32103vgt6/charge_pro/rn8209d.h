/*******************************************************************************
 *          Copyright (c) 2020-2050,  Co., Ltd.
 *                              All Right Reserved.
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include <string.h>
#include "chtask.h"
#include "drv_gpio.h"


#ifndef _RN8209D_H_
#define _RN8209D_H_

#define RN8209D_HW_SPI                0
#define RN8209D_SF_SPI                1

#define RN8209D_VOLT_VGA              2                              // 
#define RN8209D_CURR_VGA              2                              // 

#define RN8209D_PULSE_CONSTANT        3200ul

#define RN8209D_MAX_E                 ((uint64_t)0xffffff * 1000 / RN8209D_PULSE_CONSTANT)    // 最大可计量电量  


#define RN8209D_VOLT_CURR_LIM_RES     200000ul                       // 
#define RN8209D_VOLT_RES              200ul                          // 
#define RN8209D_CURR_RES              499ul                          // 
#define RN8209D_CURR_SENSOR_RATIO     2000ul                         // 

#define RN8209D_RMS_MASK              (1ul << 23)
#define RN8209D_ADC_VALUE             (RN8209D_RMS_MASK - 1)         // 

#define RN8209D_REF_VOLT              1000                           // 

#define RN8209D_USE_SPI_TYPE          RN8209D_SF_SPI

#define RN8209D_SPC_CMD_REG           0xEA

#define RN8209D_SYSCON_REG            0x00
#define RN8209D_EMUCON_REG            0x01
#define RN8209D_HFCONST_REG           0x02
#define RN8209D_PSTART_REG            0x03
#define RN8209D_DSTART_REG            0x04
#define RN8209D_GPQB_REG              0x06
#define RN8209D_IBRMSOS_REG           0x0F
#define RN8209D_EMUCON2_REG           0x17

#define RN8209D_IARMS_REG             0x22
#define RN8209D_IBRMS_REG             0x23
#define RN8209D_URMS_REG              0x24

#define RN8209D_ENERGYP_REG           0x29
#define RN8209D_ENERGYP2_REG          0x2A
#define RN8209D_ENERGYD_REG           0x2B
#define RN8209D_ENERGYD2_REG          0x2C
#define RN8209D_EMUSTATUS_REG         0x2D

#define RN8209D_INT_REG               0x41
#define RN8209D_SYSSTATUS_REG         0x43
#define RN8209D_RDATA_REG             0x44
#define RN8209D_WDATA_REG             0x45

#define RN8209D_DEVICEID              0x7F


#define RN8209D_B_EN                  0x04

#define RN8209D_WRITE_EN              0xE5
#define RN8209D_WRITE_DIS             0xDC

#define RN8209D_A_CH_EN               0x5A
#define RN8209D_B_CH_EN               0xA5

#define RN8209D_CMD_RESET             0xFA

uint8_t  rn8209d_init(void);

uint32_t rn8209d_get_volt(void);

uint32_t rn8209d_get_current(void);

uint32_t rn8209d_get_TotalE(uint32_t *puiTotalE);

#endif










