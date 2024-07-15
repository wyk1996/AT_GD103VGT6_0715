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
#include "wzgprs_com_drv.h"
#include "rtthread.h"
#include "common.h" 
#include "wzgprs_def.h"

uint8_t  caRxBuf[1600 + 2] = {0};  

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_serial_send_data(uint8_t *pcData,uint16_t usLen)
{
	uint16_t i = 0; 
	
	RT_ASSERT(st_WzComParam.write != RT_NULL); 
	
    st_WzComParam.write(&st_WzComParam,pcData,usLen);	

    rt_kprintf("tx len:%d,tx data:",usLen); 

    for(i = 0;i < usLen;i++)
    {
        rt_kprintf("%02x ",pcData[i]);
    }

    rt_kprintf("\r\n");
}


/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_serial_recive_data(void *parameter)
{
	int iRxLen = 0,i = 0; 
	 
	wzgprs_port_init(); 
	
	RT_ASSERT(st_WzComParam.read != RT_NULL); 
	
    while (1)
    {
	    iRxLen = st_WzComParam.read(&st_WzComParam,caRxBuf,1600);
        
		if(iRxLen > 0 && iRxLen <= 1600)
		{  
		    rt_kprintf("rx len:%d,rx data:",iRxLen); 
			
			for(i = 0;i < iRxLen;i++)
	        {
		        rt_kprintf("%02x ",caRxBuf[i]);
	        }
			rt_kprintf("\r\n");    
	
			/* 给万桩GPRS任务模块式发送消息 这个属于模块的内部消息 */ 
            mq_service_xxx_send_msg_to_xxx(CM_WZGPRS_MODULE_ID,WZGPRS_SERIAL_RECV,0,iRxLen,caRxBuf);
			
            memset(caRxBuf,0,iRxLen); 			
		}
    }
}

/**
 * @brief wzgprs_serial_recive_thread线程初始化 
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
int wzgprs_serial_recive_init(void)
{
	rt_thread_t wzgprs_serial_recive_thread = RT_NULL;
    
	/* 创建 wzgprs_serial_recive_thread 线程 */
    wzgprs_serial_recive_thread = rt_thread_create("wzgprs_serial_recive_thread", 
										wzgprs_serial_recive_data, 
										RT_NULL, 
										2048, 
										5, 
										10);
	
    /* 创建成功则启动线程 */
    if(wzgprs_serial_recive_thread != RT_NULL)
    {
        rt_thread_startup(wzgprs_serial_recive_thread);
    }
    else
    {
        goto __exit;
    }
	return  RT_EOK;
	
__exit:
    if (wzgprs_serial_recive_thread)
    {
        rt_thread_delete(wzgprs_serial_recive_thread);
    }
    return RT_ERROR;	
}

//INIT_APP_EXPORT(wzgprs_serial_recive_init);
