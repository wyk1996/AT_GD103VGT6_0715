/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file
 * @note
 * @brief
 *
 * @author
 * @date
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
#include <time.h>
#include <board.h>
#include <fal.h>
#include <string.h>
#include "user_lib.h"
#include "easyflash.h"
#include "drv_rtc.h"
#include "user_lib.h"
#include "ch_port.h"
#include "mfrc522.h"
#include "4GMain.h"
extern _m1_card_info m1_card_info;

SYSTEM_RTCTIME gs_SysTime;

CP56TIME2A_T gsCP56Time;

/* 用于获取RTC毫秒 */
int32_t gsi_RTC_Counts;

uint32_t gui_RTC_millisecond;

//extern S_DEVICE_CFG gs_DevCfg;
//extern S_APP_CHARGE gs_AppCharge;
extern long list_mem(void);

/* UNIQUE_ID[31: 0] */
uint32_t Unique_ID1;
/* UNIQUE_ID[63:32] */
uint32_t Unique_ID2;
/* UNIQUE_ID[95:63] */
uint32_t Unique_ID3;








//函数功能:设置看门狗复位周期为3.2秒
void FWDG_Config(void)
{
    rcu_osci_on(RCU_IRC40K);//启动"内部40KHz振荡器"
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K))
    {   //等待"内部40KHz振荡器稳定"
    }

    fwdgt_write_enable();//允许访问FWDGT_RLD(看门狗重装载寄存器)和FWDGT_PSC(看门狗预分频器寄存器)
    fwdgt_config((4000-1), FWDGT_PSC_DIV256);  //周期为:128/40KHz=3.2ms  4000*3.2ms=12800ms

//    fwdgt_config((4000-1), FWDGT_PSC_DIV32);
//    //设置IWDG重装载值为3999,重装载寄存器IWDG_RLR有12位值,设置IWDG预分频值为32
//    //独立看门狗的分频器输出周期为:32/40KHz=0.8ms
//    //独立看门狗的复位周期为:4000*32/40KHz=3200ms
    fwdgt_counter_reload();  //喂狗,按照IWDG重装载寄存器FWDGT_RLR的值重装载IWDG计数器
    fwdgt_enable();  //使能IWDG

}




/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
int main(void)
{
    uint8_t i=0;
    rt_device_t device;

    struct tm *Net_time;

    time_t time_now = 0;

    uint32_t timeout = rt_tick_get();

    uint32_t mem_timeout = rt_tick_get();

    device = rt_device_find("rtc");

    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }
    RT_ASSERT(fal_init() > 0);

//    rt_kprintf("The current version of APP firmware is %s\n", FIRMWARE_VERSION);
//    if(rt_memcmp((const char *)&(item_info_node->uc_SN[2]),(const char *)ucDev_Sn,6)==0)

    /* 获取MCU唯一ID */
    Unique_ID1 = *(uint32_t *)(0x1FFFF7E8);
    Unique_ID1 = SW32(Unique_ID1);
    Unique_ID2 = *(uint32_t *)(0x1FFFF7EC);
    Unique_ID2 = SW32(Unique_ID2);
    Unique_ID3 = *(uint32_t *)(0x1FFFF7F0);
    Unique_ID3 = SW32(Unique_ID3);
    rt_kprintf("MCU Id:%08x %08x %08x\n",Unique_ID1,Unique_ID2,Unique_ID3);

    memcpy(&m1_card_info.MCUID[0],&Unique_ID1, 4);
    memcpy(&m1_card_info.MCUID[4],&Unique_ID2, 4);
    memcpy(&m1_card_info.MCUID[8],&Unique_ID3, 4);
    for(i = 0; i < 12; i++)
    {
        if(m1_card_info.MCUID[i]  == 0x00)
        {
            m1_card_info.MCUID[i] = 0xaa;
        }
    }

//    memcpy(&gs_AppCharge.ucUniqueID[0], &Unique_ID1, 4);
//    memcpy(&gs_AppCharge.ucUniqueID[4], &Unique_ID2, 4);
//    memcpy(&gs_AppCharge.ucUniqueID[8], &Unique_ID3, 4);

//	rt_kprintf("gs_AppCharge.ucUniqueID:");
//    for(uint8_t i=0;i<12;i++)
//    {
//        rt_kprintf("%02x ",gs_AppCharge.ucUniqueID[i]);
//    }
//    rt_kprintf("\r\n");
//    LIB_Asc2Bcd("10319042800003", 16, gs_DevCfg.ucDeviceID);
//    memcpy(gs_AppCharge.ucDeviceID, gs_DevCfg.ucDeviceID, 8);
//    memcpy(gs_AppCharge.ucDeviceID, gs_AppCharge.ucUniqueID, 12);


//====看门狗初始化
#if(doguser)
    FWDG_Config();
#endif

    while (1)
    {
        //==独立看门狗喂狗====
        fwdgt_counter_reload(); //喂狗
        /* 每隔1000个滴答(1000ms),更新一下gs_SysTime,使其保持为最新值 */
        if(rt_tick_get()>=(100+timeout))
        {
            timeout = rt_tick_get();
            rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time_now);
            //time_now = time_now + 8*60*60;
            Net_time = localtime(&time_now);
            gs_SysTime.ucYear = Net_time->tm_year;
            gs_SysTime.ucMonth = (Net_time->tm_mon)+1;
            gs_SysTime.ucDay = Net_time->tm_mday;
            gs_SysTime.ucWeek = Net_time->tm_wday;
            gs_SysTime.ucHour = Net_time->tm_hour;
            gs_SysTime.ucMin = Net_time->tm_min;
            gs_SysTime.ucSec = Net_time->tm_sec;
            //list_thread();  //查看线程栈使用率
            localtime_to_cp56time((time_now+28800), &gsCP56Time);
//			rt_kprintf("NTP Time:%04d-%02d-%02d-%02d %02d:%02d:%02d\r\n",(Net_time->tm_year)+1900,\
//                    (Net_time->tm_mon)+1, Net_time->tm_mday, Net_time->tm_wday, Net_time->tm_hour,Net_time->tm_min,gs_SysTime.ucSec);
        }

        //系统时间复位到1970年后，  主要是更改时间，防止存储记录查询不准确
        static uint8_t  NUM = 1;
        if((gs_SysTime.ucYear == 70) && (NUM==1))
        {
            gs_SysTime.ucYear = 123;  //这是年，123标示是23年
            set_time(12, 0, 0);
            set_date(2023, 12, 1);
            NUM = 0;
        }



//        /* 每隔30秒,查询一下内存信息 */
//		if(rt_tick_get()>=(30000+mem_timeout))
//		{
//			mem_timeout = rt_tick_get();
//			list_mem();
//		}

        rt_thread_mdelay(100);

    }
}
