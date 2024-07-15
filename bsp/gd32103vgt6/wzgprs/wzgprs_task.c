/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
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
#include "string.h"
#include "wzgprs_def.h"
#include "common.h" 

/*
===============================================================================================================================
                                            ****** 类型定义/宏定义 ******
===============================================================================================================================
*/
#define    WZGPRS_THREAD_STK_SIZE     2048
#define    WZGPRS_THREAD_PRIO         14
#define    WZGPRS_THREAD_TIMESLICE    5
#define    WZGPRS_MAX_MSG_SIZE        2048

/*
===============================================================================================================================
                                            ****** 变量定义 ******
===============================================================================================================================
*/
rt_thread_t    pstWzThread = RT_NULL;
uint8_t        ucaWzMsgDataBuf[WZGPRS_MAX_MSG_SIZE] = {0}; 
WZGPRS_WORK_T  stWzGprsWork = {0}; 
rt_timer_t     pstWzTime = RT_NULL;

/*
===============================================================================================================================
                                            ****** 函数定义/声明 ******
===============================================================================================================================
*/
void wzgprs_tcu_msg_proc(WZGPRS_WORK_T *pstWzGprsWork,uint32_t uiMsgCode,uint32_t uiMsgVar,uint8_t *pucMsgData,uint32_t uiDataLen); 
void wzgprs_device_get_remote(void); 

/**
 * @brief 跳转到新状态 
 * @param[in] ucNewStat: 要跳到的新状态
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_jump_new_stat(uint8_t ucNewStat)
{
    stWzGprsWork.ucWorkState = ucNewStat;
	stWzGprsWork.uiJumpTick  = rt_tick_get(); 
} 

/**
 * @brief  
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_wait_msg(void)
{ 
	MQ_MSG_T stMsg = {0}; 
	
	if(mq_service_recv_msg(CM_WZGPRS_MODULE_ID,&stMsg,ucaWzMsgDataBuf,sizeof(ucaWzMsgDataBuf),RT_WAITING_FOREVER) == 0)
	{
         if(stMsg.uiSrcMoudleId == CM_WZGPRS_MODULE_ID)
		 {
		     wzgprs_inter_msg_proc(&stWzGprsWork,stMsg.uiMsgCode,stMsg.uiMsgVar,stMsg.pucLoad,stMsg.uiLoadLen);
		 }	
         else if(stMsg.uiSrcMoudleId == CM_TCUTASK_MODULE_ID)
		 {
		     wzgprs_tcu_msg_proc(&stWzGprsWork,stMsg.uiMsgCode,stMsg.uiMsgVar,stMsg.pucLoad,stMsg.uiLoadLen); 
		 }			 
	}
}
 
/**
 * @brief 文件下载 
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_file_down(void)
{
	/*if(ucFileFlg == 1)
	{
	    ucFileFlg = 0 ; 
		wzgprs_tcu_upgrade_proc(&stWzGprsWork,(uint8_t *)WZGPRS_FILE_URL,strlen(WZGPRS_FILE_URL),255) ;
	}
	*/
}

/**
 * @brief 设备是否离线恢复
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_is_device_offline_recover(void)
{
    if(COMMON_INVALID_FLAG == stWzGprsWork.ucOfflineFlag)
	{
		rt_kprintf("wz gprs device offline recover\r\n"); 
	    wzgprs_jump_new_stat(WZGPRS_WORK_IDIE);     
	}
}
/**
 * @brief 任务循环处理
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_task_loop(void)
{
    switch(stWzGprsWork.ucWorkState)
	{
	    case WZGPRS_WORK_IDIE:
			//wzgprs_device_set_remote(WZGPRS_REMOTE_ADDR,strlen(WZGPRS_REMOTE_ADDR));
			wzgprs_jump_new_stat(WZGPRS_WORK_DELAY); 
		    break;
		case WZGPRS_WORK_NET_OFFLINE:
			break; 
        case WZGPRS_WORK_NET_ONLINE:
			wzgprs_file_down(); 
			break; 
	    case WZGPRS_WORK_DEVICE_OFFLINE:
			wzgprs_is_device_offline_recover() ; 
			break;
        case WZGPRS_WORK_DELAY:
			if(rt_tick_get() - stWzGprsWork.uiJumpTick > CM_TIME_1_SEC)
			{
			    wzgprs_jump_new_stat(WZGPRS_WORK_NET_OFFLINE); 
                /*wzgprs_device_get_remote();*/ 				
			}
            break;
        default:
            break; 			
	}
}
/**
 * @brief 万桩模块定时任务 
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_time(void *parg)
{
    /* 给万桩GPRS任务模块式发送消息 这个属于模块的内部消息 */ 
    mq_service_xxx_send_msg_to_xxx(CM_WZGPRS_MODULE_ID,WZGPRS_TIMER,0,0,RT_NULL);	
}

/**
 * @brief 万桩GPRS模块任务运行主体
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_thread_entry(void* parg)
{
	memset(&stWzGprsWork,0,sizeof(WZGPRS_WORK_T));
	
	pstWzTime = rt_timer_create("WZ_TIME", 
								wzgprs_time,
								RT_NULL,  
								CM_TIME_100_MSEC, 
								RT_TIMER_FLAG_PERIODIC); 
	
	RT_ASSERT(pstWzTime != RT_NULL);
	
	mq_service_bind(CM_WZGPRS_MODULE_ID,"wzgprs_mq"); 
	
	wzgprs_serial_recive_init(); 
	
	rt_timer_start(pstWzTime);		 
	
    while(1)
	{
	    wzgprs_wait_msg(); 
		wzgprs_task_loop(); 
	}
}
/**
 * @brief 创建万桩GPRS模块任务
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
int wzgprs_thread_init(void)
{
    pstWzThread = rt_thread_create("WZGPRS_TASK", 
							wzgprs_thread_entry, 
							RT_NULL, 
							WZGPRS_THREAD_STK_SIZE, 
							WZGPRS_THREAD_PRIO, 
							WZGPRS_THREAD_TIMESLICE );
    
	if(RT_NULL != pstWzThread)
	{
        rt_kprintf("The wzgprs thread create suceess\n");
		rt_thread_startup(pstWzThread);
	}
	else
    {
        goto __exit;
    }
	
	return RT_EOK;
	
__exit:
    if(pstWzThread)
    {
        rt_thread_delete(pstWzThread);
    }

    return RT_ERROR;
}
INIT_APP_EXPORT(wzgprs_thread_init);
