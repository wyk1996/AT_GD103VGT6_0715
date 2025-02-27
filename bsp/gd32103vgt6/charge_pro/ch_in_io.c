/*******************************************************************************
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
#include "ch_port.h"
#include "common.h"
#include "dlt645_port.h"

/* drv_gpio.c PD13 */
#define INPUT_PIN_YX1_DOOR   61
/* drv_gpio.c PD14 */
#define INPUT_PIN_YX2     62
/* drv_gpio.c PD15 */
#define INPUT_PIN_YX3_EM        63
/* drv_gpio.c PC6 */
#define INPUT_PIN_YX4        38
/* drv_gpio.c PC7 */
#define INPUT_PIN_YX5        39


typedef struct
{
    uint8_t  ucLowCnt;
    uint8_t  ucHighCnt;
    uint8_t  ucFault;
} INPUT_IO_T;

typedef struct
{
    INPUT_IO_T stInputIo[INPUT_YX_MAX];
} INPUT_IO_FAULT;

const rt_base_t rtPinTab[INPUT_YX_MAX] =
{
    INPUT_PIN_YX1_DOOR,
    INPUT_PIN_YX2,
    INPUT_PIN_YX3_EM,
    INPUT_PIN_YX4,
    INPUT_PIN_YX5,
};

typedef struct
{
    uint8_t   ov_v_flg;
    uint8_t   ov_c_flg;
    uint8_t   ov_v_cnt;
    uint8_t   ov_c_cnt;
} _AMM_FAULT;

static rt_timer_t pstInputIoTimer;
static INPUT_IO_FAULT stFault;
static _AMM_FAULT  stAmmFault = {0};

extern _dlt645_info dlt645_info;


/**
 * @brief 发送消息到 充电任务
 * @param[in] ucMsgCode: 消息码
 *         	  ucData: 消息数据
 * @param[out]
 * @return
 * @note
 */
static void send_msg_to_ch(uint8_t ucMsgCode,uint8_t ucData)
{
    mq_service_xxx_send_msg_to_xxx(CM_CHTASK_MODULE_ID,ucMsgCode,ucData,0,RT_NULL);
}



extern CH_TASK_T stChTcb;

/**
 * @brief 是否设置故障
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t is_set_input_io_fault(uint8_t ucIndex,int iData)
{
    if(1 == iData)
    {
        /* 高正常 */
        return 0;
    }

    stFault.stInputIo[ucIndex].ucLowCnt = 0;
    if(++stFault.stInputIo[ucIndex].ucHighCnt < 3)
    {
        return 0;
    }

    if(0 == stFault.stInputIo[ucIndex].ucFault)
    {
        stFault.stInputIo[ucIndex].ucFault = 1;
        /* 故障发生 */

        stChTcb.stIOFault.uiFaultFlg |= 0x00000001 << ucIndex;
        if(stChTcb.stIOFault.ucFrist == 0)
        {
            stChTcb.stIOFault.ucFrist = ucIndex;
        }
    }

    stFault.stInputIo[ucIndex].ucHighCnt = 0;
    return 1;
}

/**
 * @brief 故障是否恢复
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t is_input_io_fault_resume(uint8_t ucIndex,int iData)
{
    uint8_t i = 0;
    if(0 == iData)
    {
        /* 不正常 不能清除 */
        return 0;
    }

    stFault.stInputIo[ucIndex].ucHighCnt = 0;
    if(++stFault.stInputIo[ucIndex].ucLowCnt < 3)
    {
        return 0;
    }

    if(1 == stFault.stInputIo[ucIndex].ucFault)
    {
        stFault.stInputIo[ucIndex].ucFault = 0;
        /* 故障恢复 */
        stChTcb.stIOFault.uiFaultFlg &= ~(0x00000001 << ucIndex);
        stChTcb.stIOFault.ucFrist = 0;
        for(i = 0; i < INPUT_YX_MAX; i++)
        {
            if(stChTcb.stIOFault.uiFaultFlg & ((0x00000001) << i))
            {
                stChTcb.stIOFault.ucFrist = i;
            }
        }

    }
    stFault.stInputIo[ucIndex].ucLowCnt = 0;
    return 1;
}

/**
 * @brief 输入io定时检测函数
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void input_io_period_check(void *parg)
{
    uint8_t i = 0;
    int iRet = 0;

#warning "YXY"
    for(i = 0; i < INPUT_YX_MAX; i++)
    {
			
      iRet = rt_pin_read(rtPinTab[i]);
			
//		if(0 == i)
//		{
//            /* 0是漏电检测引脚,正常为低,需要反一下 */
//            iRet = (0 == iRet ? 1 : 0);
//		}

        if(is_set_input_io_fault(i,iRet))
        {
            /* 需要设置故障 */
            continue;
        }
        is_input_io_fault_resume(i,iRet);
    }
}

/**
 * @brief 输入io初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void input_io_init(void *pstRn8209)
{
    uint8_t i = 0;

    pstInputIoTimer = rt_timer_create("input_io_period_check",
                                      input_io_period_check,
                                      pstRn8209,
                                      CH_TIME_20_MSEC,
                                      RT_TIMER_FLAG_PERIODIC);

    RT_ASSERT(pstInputIoTimer != RT_NULL);

    for(i = 0; i < INPUT_YX_MAX; i++)
    {
        rt_pin_mode(rtPinTab[i],PIN_MODE_INPUT);
    }

    memset(&stFault,0,sizeof(INPUT_IO_FAULT));

    rt_timer_start(pstInputIoTimer);
}


