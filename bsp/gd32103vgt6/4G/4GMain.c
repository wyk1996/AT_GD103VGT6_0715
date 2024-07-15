/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.c
* Author			:
* Date First Issued	:
* Version			:
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "4GMain.h"
#include <string.h>
#include "ModuleA7680C.h"
#include "dwin_com_pro.h"
#include "HYFrame.h"
#include "APFrame.h"
#include "YKCFrame.h"
#include "gd32f10x_gpio.h"
#include "4GRecv.h"
/* Private define-----------------------------------------------------------------------------*/
#define   GPRSMAIN_Q_LEN  								20
//临时IP端口放在这里，后面应该重屏幕下发读取

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSMAINOSQ[GPRSMAIN_Q_LEN];					// 消息队列
/*static OS_EVENT *GPRSMainTaskEvent;				            // 使用的事件
OS_EVENT *SendMutex;                 //互斥锁，同一时刻只能有一个任务进行临界点访问*/
static rt_mutex_t dynamic_mutex = RT_NULL;
_START_NET_STATE StartNetState[GUN_MAX] = {NET_STATE_ONLINE,NET_STATE_ONLINE};		//启动的时候状态
    //{XY_YKC,	{121,43,69,62},8767,{"121.43.69.62"}	,		   	1},  //显示YKC IP
_NET_CONFIG_INFO  NetConfigInfo[XY_MAX] =
{
    {XY_HY,		{116,62,125,35},8000, {"116.62.125.35"} ,		1},
	
    {XY_YKC,	{121,43,69,62},8767,{"121.43.69.62"}	,		1},  //显示YKC IP
	
	//{XY_YKC,	{81,70,61,150},50000, {"81.70.61.150"} ,		1},     //显示塑云 IP
	
    {XY_AP,		{114,55,186,206},5738, {"114.55.186.206"}  ,	 	1},

    {XY_YL1,	{11,11,11,11},1111,{0} ,		1},

    {XY_YL2,	{120,55,83,107},44444,{0} ,	  	1},

    {XY_YL3,	{120,55,83,107},44444,{0} ,		1},

    {XY_YL4,	{120,55,83,107},44444,{0} ,		1},

    {XY_YL5,	{120,55,83,107},44444,{0} ,		1},
};

_RESEND_BILL_CONTROL ResendBillControl[GUN_MAX] = { {0},{0}};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : APP_SetResendBillState
* Description  : 设置是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
void APP_SetResendBillState(uint8_t gun,uint8_t state)
{
    if(gun >= GUN_MAX)
    {
        return;
    }
    ResendBillControl[gun].ResendBillState = state;
    ResendBillControl[gun].SendCount = 0;
}

uint8_t   APP_GetStartNetState(uint8_t gun)
{
    if(gun >= GUN_MAX)
    {
        return _4G_APP_START;
    }
    return (uint8_t)StartNetState[gun];
}
//桩上传结算指令
/*****************************************************************************
* Function     : ReSendBill
* Description  : 重发订单
* Input        : void *pdata  ifquery: 1 查询  0：重复发送
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t  ReSendBill(_GUN_NUM gun,uint8_t* pdata,uint8_t ifquery)
{

    if(pdata == NULL)
    {
        return RT_FALSE;
    }
    ResendBillControl[gun].CurTime = rt_tick_get();		//获取当前时间
    if(ifquery)
    {
#if(NET_YX_SELCT == XY_HY)
        {
            return HY_SendBillData(pdata,200,ifquery);
        }
#endif
        return RT_TRUE;
    }
    if(ResendBillControl[gun].ResendBillState == RT_FALSE)
    {
        return RT_FALSE;			//不需要重发订单
    }
    if((ResendBillControl[gun].CurTime - ResendBillControl[gun].LastTime) >= CM_TIME_10_SEC)
    {
        if(++ResendBillControl[gun].SendCount > 3)
        {
            ResendBillControl[gun].ResendBillState = RT_FALSE;		//发送三次没回复就不发了
            ResendBillControl[gun].SendCount = 0;
            return RT_FALSE;
        }
        ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
#if(NET_YX_SELCT == XY_HY)
        {
            return HY_SendBillData(pdata,200,ifquery);
        }
#endif
  #if(NET_YX_SELCT == XY_AP)
        {
            return AP_SendBillData(pdata,200);
        }
		#endif
#if(NET_YX_SELCT == XY_YKC)
        {
            return YKC_SendBillData(pdata,200);
        }
#endif
    }


    return RT_TRUE;
}

/*****************************************************************************
* Function     : APP_GetResendBillState
* Description  : 获取是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t APP_GetResendBillState(uint8_t gun)
{
    if(gun >= GUN_MAX)
    {
        return RT_FALSE;
    }
    return ResendBillControl[gun].ResendBillState;
}

/*****************************************************************************
* Function     : ReSendOffLineBill
* Description  :
* Input        : 发送离线交易记录订单
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t  ReSendOffLineBill(void)
{
    static uint8_t count = 0;			//联网状态下，连续三次发送无反应则丢失
    uint8_t data[300];
    static uint8_t num = 0;
    //离线交易记录超时不管A枪和B枪都一样，目前都只用A枪
    ResendBillControl[GUN_A].OFFLineCurTime = rt_tick_get();		//获取当前时间

    //获取是否有离线交易记录
    ResendBillControl[GUN_A].OffLineNum = APP_GetNetOFFLineRecodeNum();		//获取离线交易记录
    if(ResendBillControl[GUN_A].OffLineNum > 0)
    {
        if((ResendBillControl[GUN_A].OFFLineCurTime - ResendBillControl[GUN_A].OFFLineLastTime) >= CM_TIME_30_SEC)
        {
            if(num == ResendBillControl[GUN_A].OffLineNum)
            {
                //第一次不会进来
                if(++count >= 3)
                {
                    //联网状态下连续三次未返回，则不需要发送
                    count = 0;
                    ResendBillControl[GUN_A].OffLineNum--;
                    APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
                }
            }
            else
            {
                count = 0;
                num = ResendBillControl[GUN_A].OffLineNum;
            }
            ResendBillControl[GUN_A].OFFLineLastTime = ResendBillControl[GUN_A].OFFLineCurTime;
#if(NET_YX_SELCT == XY_AP)
            {
                APP_RWNetOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_READ,data);   //读取离线交易记录
                AP_SendOffLineBillData(data,300);
                APP_RWNetFSOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_READ,data);   //读取离线交易记录
                AP_SendOffLineBillFSData(data,300);
            }
#endif
        }
    }
    return RT_TRUE;
}

/*****************************************************************************
* Function     : APP_SetStartNetState
* Description  : 设置启动方式网络状态
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_SetStartNetState(uint8_t gun ,_START_NET_STATE  type)
{
    if((type >=  NET_STATE_MAX) || (gun >= GUN_MAX))
    {
        return RT_FALSE;
    }

    StartNetState[gun] = type;
    return RT_TRUE;
}

/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取网络状态
* Input        : 那一路
* Output       : TRUE:表示有网络	FALSE:表示无网络
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
uint8_t  APP_GetNetState(uint8_t num)
{
    if(STATE_OK == APP_GetAppRegisterState(num))
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : 4G_RecvFrameDispose
* Description  :4G接收
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t _4G_RecvFrameDispose(uint8_t * pdata,uint8_t len)
{
#if(NET_YX_SELCT == XY_HY)
    {
        return  HY_RecvFrameDispose(pdata,len);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        return AP_RecvFrameDispose(pdata,len);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        return YKC_RecvFrameDispose(pdata,len);
    }
#endif
    return RT_TRUE;
}


/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint8_t *  APP_GetBatchNum(uint8_t gun)
{
#if(NET_YX_SELCT == XY_HY)
    {
        return APP_GetHYBatchNum(gun);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        return APP_GetAPBatchNum(gun);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        return APP_GetYKCBatchNum(gun);
    }
#endif
    return NULL;
}

/*****************************************************************************
* Function     : APP_GetNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint32_t APP_GetNetMoney(uint8_t gun)
{
#if(NET_YX_SELCT == XY_HY)
    {
        return APP_GetHYNetMoney(gun);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        return APP_GetAPQGNetMoney(gun);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        return APP_GetYKCNetMoney(gun);
    }
#endif
    return 0;
}
/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  :4G发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t  _4G_SendFrameDispose()
{
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendFrameDispose();
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendFrameDispose();
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendFrameDispose();
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   Pre4GBill(_GUN_NUM gun,uint8_t *pdata)
{
#if(NET_YX_SELCT == XY_HY)
    {
        PreHYBill(gun,pdata);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        PreAPBill(gun,pdata);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        PreYKCBill(gun,pdata);
    }
#endif
    return RT_TRUE;
}



/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendDevState(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_HY)
    {
        if(gun == GUN_A)
        {
            HY_SendDevStateA();
        }
        if(gun == GUN_B)
        {
            HY_SendDevStateB();
        }
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_SendRateAck
* Description  : 费率应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendRateAck(uint8_t cmd)
{
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendRateAck(cmd);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendRateAck(cmd);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendRateAck(cmd);
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t    _4G_SendQueryRate(void)
{
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendPriReq();
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_SendRateMode
* Description  : 发送计费模型
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t    _4G_SendRateMode(void)
{
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendPriModel();
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_SendSetTimeAck
* Description  : 校时应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _4G_SendSetTimeAck(void)
{

#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendSetTimeAck();
    }
#endif

#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendSetTimeAck();
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendBill(_GUN_NUM gun)
{
    if(gun >= GUN_MAX)
    {
        return RT_FALSE;
    }
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendBill(gun);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendBill(gun);
        AP_SendTimeSharBill(gun);		//发送分时记录
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendBill(gun);
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_SendCardInfo
* Description  : 发送卡鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendCardInfo(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendCardInfo(gun);
    }
#endif
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendCardInfo(gun);
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_GetStartType
* Description  : 获取快充启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
_4G_START_TYPE   _4G_GetStartType(uint8_t gun)
{
    if(gun >= GUN_MAX)
    {
        return _4G_APP_START;
    }
#if(NET_YX_SELCT == XY_AP)
    {
        return (_4G_START_TYPE)APP_GetAPStartType(gun);
    }
#endif
#if(NET_YX_SELCT == XY_HY)
    {
        return (_4G_START_TYPE)APP_GetHYStartType(gun);
    }
#endif
    return _4G_APP_START;
}

/*****************************************************************************
* Function     : _4G_SetStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _4G_SetStartType(uint8_t gun ,_4G_START_TYPE  type)
{
    if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
    {
        return RT_FALSE;
    }

#if(NET_YX_SELCT == XY_AP)
    {
        APP_SetAPStartType(gun,type);
    }
#endif
#if(NET_YX_SELCT == XY_HY)
    {
        APP_SetHYStartType(gun,type);
    }
#endif
    return RT_TRUE;
}

/*****************************************************************************
* Function     : _4G_SendVinInfo
* Description  : 发送Vin鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendVinInfo(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendVinInfo(gun);
    }
#endif
    return RT_TRUE;
}


/*****************************************************************************
* Function     : _4G_SendCardVinCharge
* Description  : 发送卡Vin充电
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t _4G_SendCardVinCharge(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendCardVinStart(gun);
    }
#endif
    return RT_TRUE;
}
/*****************************************************************************
* Function     : _4G_SendStOPtAck
* Description  : 停止应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _4G_SendStopAck(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendStopAck(gun);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendStopAck(gun);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendStopAck(gun);
    }
#endif
    return RT_TRUE;
}



/*****************************************************************************
* Function     : HFQG_SendStartAck
* Description  : 合肥乾古开始充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   _4G_SendStartAck(_GUN_NUM gun)
{
#if(NET_YX_SELCT == XY_HY)
    {
        HY_SendStartAck(gun);
    }
#endif
#if(NET_YX_SELCT == XY_AP)
    {
        AP_SendStartAck(gun);
    }
#endif
#if(NET_YX_SELCT == XY_YKC)
    {
        YKC_SendStartAck(gun);
    }
#endif
    return RT_TRUE;
}


/*****************************************************************************
* Function     : UART_4GWrite
* Description  :串口写入，因多个任务用到了串口写入，因此需要加互斥锁
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2020-11-26     叶喜雨
*****************************************************************************/
uint8_t UART_4GWrite(uint8_t* const FrameBuf, const uint16_t FrameLen)
{
    /*uint8_t len,err;


    OSMutexPend(SendMutex, 0, &err);					//获取锁
    len = BSP_UARTWrite(GPRS_UART,FrameBuf,FrameLen);
    if(len)
    {
    	OSTimeDly((FrameLen/10 + 10)*SYS_DELAY_1ms);	//等待数据发送完成  115200波特率， 1ms大概能发10个字节（大于10个字节）
    	//OSTimeDly(SYS_DELAY_5ms);
    }
    OSMutexPost(SendMutex); //释放锁
    return len;*/
#warning "互斥需要测试"
    // rt_kprintf("tx %s\r\n",FrameBuf);
    rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
    Uart4GCom.write(FrameBuf,FrameLen);
    rt_thread_mdelay(50);
    rt_mutex_release(dynamic_mutex);
}

/*****************************************************************************
* Function     : Connect_4G
* Description  : 4G网络连接
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Connect_4G(void)
{
//#define RESET_4G_COUNT	3    //目前连续1次未连接上服务器 ，则重新启动
//    static uint32_t i = 0;
//    uint8_t count;

//    if(APP_GetSIM7600Status() != STATE_OK)  //模块存在
//    {
//        Module_SIM7600Test();    //测试模块是否存在
//    }
//    if(APP_GetSIM7600Status() != STATE_OK)  //模块不存在
//    {
//        if(++i > RESET_4G_COUNT)
//        {
//            i = 0;
//            SIM7600Reset();
//        }
//        return RT_FALSE;
//    }
//    //到此说明模块已经存在了模块存在了
//    //连接服务器,可能又多个服务器
//    for(count = 0; count < NetConfigInfo[NET_YX_SELCT].NetNum; count++)
//    {
//        if(APP_GetModuleConnectState(count) != STATE_OK) //位连接服务器
//        {
//            if(count == 0)
//            {
//                ModuleSIM7600_ConnectServer(count,(uint8_t*)NetConfigInfo[NET_YX_SELCT].pIp,NetConfigInfo[NET_YX_SELCT].port);
//            }
//            else
//            {
//                ModuleSIM7600_ConnectServer(count,(uint8_t*)GPRS_IP2,GPRS_PORT2);		//连接服务器
//            }
//        }
//        if(APP_GetModuleConnectState(count) != STATE_OK)  //模块未连接
//        {
//            i++;
//            SIM7600CloseNet(count);			//关闭网络操作
////			if(++i > RESET_4G_COUNT)
////			{
////				i = 0;
////				SIM7600Reset();
////			}
//            //	return FALSE;
//        }
//    }
//    //是否所有模块都没有连接上
//    for(count = 0; count < NetConfigInfo[NET_YX_SELCT].NetNum; count++)
//    {
//        if(APP_GetModuleConnectState(count) == STATE_OK)
//        {
//            break;
//        }
//    }
//    //if((count == LINK_NET_NUM)&& (i > RESET_4G_COUNT) )
//    if(i > RESET_4G_COUNT)  //主要满足一个条件  就重连20210714
//    {
//        i = 0;
//        SIM7600Reset();
//        return RT_FALSE;			//所有模块都没有连接到服务器
//    }
//    //说明至少一个连接上服务器了
//    return RT_TRUE;
	
	
	#define RESET_4G_COUNT	5    //目前连续10次未连接上服务器，则重新启动
    static uint8_t i = 6; //第一次上来先复位
	uint8_t count;
	if(APP_GetModuleConnectState(0) !=STATE_OK)   //判断主平台
    {
		if(++i > RESET_4G_COUNT)  
        {
            i = 0;
            SIM7600Reset();
        }
    }
    if(APP_GetSIM7600Status() != STATE_OK)  //判断4g模块是否存在
    {
        Module_SIM7600Test();    //4g模块重联
    }
    if(APP_GetSIM7600Status() != STATE_OK)  //模块不存在
    {
        return RT_FALSE;
    }
	
    //到此说明4g模块已经存在。连接服务器,可能又多个服务器
    for(count = 0; count < NetConfigInfo[NET_YX_SELCT].NetNum; count++)
    {
        if(APP_GetModuleConnectState(count) != STATE_OK) //未连接后台服务器，下面开始连接后台服务器
        {
            if(count == 0)
            {
                ModuleSIM7600_ConnectServer(count,(uint8_t*)NetConfigInfo[NET_YX_SELCT].pIp,NetConfigInfo[NET_YX_SELCT].port);
            }
            else
            {
                ModuleSIM7600_ConnectServer(count,(uint8_t*)GPRS_IP2,GPRS_PORT2);	 
            }
        }
        if(APP_GetModuleConnectState(count) != STATE_OK)  //未连到服务器
        {
            SIM7600CloseNet(count);			//关闭网络操作
        }
    }
    return RT_TRUE;
}


#define MSG_NUM    5
/*****************************************************************************
* Function     : Task4GMain
* Description  : 4G主任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日
*****************************************************************************/
void Task4GMain(void *pdata)
{
    uint8_t err;
	static uint8_t download_count = 0;
    /* 创建一个动态互斥量 */
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return;
    }
    rt_thread_mdelay(2000);
    if(DisSysConfigInfo.standaloneornet != DISP_NET)
    {
        return;
    }
    rt_pin_mode(IO_4G_ONOFF, GPIO_MODE_OUT_PP);
    rt_pin_mode(IO_4G_RES, GPIO_MODE_OUT_PP);
    rt_pin_mode(IO_4G_PWR, GPIO_MODE_OUT_PP);
    //打开电源
    rt_pin_write(IO_4G_PWR, PIN_HIGH );
    rt_thread_mdelay(100);


    rt_pin_write(IO_4G_RES,PIN_LOW);
    rt_pin_write(IO_4G_ONOFF,PIN_LOW);
    rt_thread_mdelay(250);
    //SIM7600Reset();

    while(1)
    {
        //连接4G连接
		if(APP_GetSIM7600Mode() == MODE_DATA)
		{
			
			Connect_4G();          //4G连接，包括模块是否存在，和连接服务
		}
		else
		{
			if(APP_GetModuleConnectState(0) == STATE_OK)
			{
			//	UART_4GWrite((uint8_t*)"AT+CGDCONT=1,\"IP\",\"APN\"\r\n", strlen("AT+CGDCONT=1,\"IP\",\"APN\"\r\n") );
				if(Module_HTTPDownload("http://opinion.people.com.cn/GB/n1/2018/0815/c1003-30228758.html"))
				//if(Module_HTTPDownload("14.205.92.144/dowload/project.hex"))
				//if(Module_HTTPDownload("http://detect.huichongchongdian.com/detect/profile/upload/2022/03/14/project.hex"))
				//if(Module_HTTPDownload("http://baidu.com"))
				{
					//升级成功
					download_count = 0;
					APP_SetSIM7600Mode(MODE_DATA);
					 Send_AT_CIPMODE(); 
					 rt_thread_mdelay(1000);
				}
				else
				{
					if(++download_count > 3)
					{
						//连续三次升级不成功，则返回升级失败
						download_count = 0;
						APP_SetSIM7600Mode(MODE_DATA);
						 Send_AT_CIPMODE(); 
						 rt_thread_mdelay(1000);
					}
				}
			}
		}
        rt_thread_mdelay(1000);
    }
}


/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static int _4G_main_init(void)
{

    rt_thread_t _4g_main_thread = NULL;


    _4g_main_thread = rt_thread_create("_4G_main_init",
                                       Task4GMain,
                                       RT_NULL,
                                       1024,
                                       19,  		//优先级
                                       20);

    if (_4g_main_thread != RT_NULL)
    {
        rt_thread_startup(_4g_main_thread);
    }
    else
    {
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (_4g_main_thread)
    {
        rt_thread_delete(_4g_main_thread);
    }

    return RT_ERROR;
}

INIT_APP_EXPORT(_4G_main_init);

/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

