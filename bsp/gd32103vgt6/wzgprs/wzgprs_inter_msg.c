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
#include "rtthread.h"
#include "common.h" 

/*
===============================================================================================================================
                                            ****** 类型定义/宏定义 ******
===============================================================================================================================
*/
#define    WZGPRS_REMOTE_ADDR         "114.55.139.53:6020"

/*
===============================================================================================================================
                                            ****** 变量定义 ******
===============================================================================================================================
*/


/*
===============================================================================================================================
                                            ****** 函数定义/声明 ******
===============================================================================================================================
*/
void wzgprs_device_set_remote(char *sRemoteAddr,uint16_t uiLen);  
void wzgprs_device_get_file_down_stat(char *sFileName,uint8_t ucFileLen); 

/**
 * @brief    
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            ucTcpState: tcu状态
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_device_data_tcpstate(WZGPRS_WORK_T *pstWzGprsWork,uint8_t ucTcpState)
{
	if(pstWzGprsWork->ucTcpStat != ucTcpState)
	{
	    pstWzGprsWork->ucTcpStat = ucTcpState; 
		pstWzGprsWork->uiTxTcpStatTick = rt_tick_get();  
		mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_NETSTATE,ucTcpState,0,RT_NULL);
	} 
	
    if(1 == ucTcpState) 
	{
	    if(WZGPRS_WORK_NET_OFFLINE == pstWzGprsWork->ucWorkState)
		{
            /* 到联网状态 */
		    WZGPRS_JUMP_NEW_STATE(pstWzGprsWork,WZGPRS_WORK_NET_ONLINE); 
		}
	}
	else 
	{
	    if(WZGPRS_WORK_NET_ONLINE == pstWzGprsWork->ucWorkState)
		{
            /* 到离线状态 */
		    WZGPRS_JUMP_NEW_STATE(pstWzGprsWork,WZGPRS_WORK_NET_OFFLINE);   
		}
	}
} 
 
/**
 * @brief 接收imei数据 
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            pucImei: imei数据 
 *            usDataLen: imei数据长度
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_rx_imei(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucImei,uint16_t usDataLen)
{
	uint32_t uiLimitLen = 0; 
	
	uiLimitLen = CM_DATA_GET_MIN((uint32_t)sizeof(pstWzGprsWork->caIMEI),(uint32_t)usDataLen); 
	
    if(memcmp(pstWzGprsWork->caIMEI,pucImei,uiLimitLen) != 0)
	{
	    memcpy(pstWzGprsWork->caIMEI,pucImei,uiLimitLen); 
		
		mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_IMEI,0,uiLimitLen,(uint8_t *)pstWzGprsWork->caIMEI);
	}
}
/**
 * @brief 接收imsi数据 
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            pucImsi: imsi数据 
 *            usDataLenimsi: 数据长度
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_rx_imsi(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucImsi,uint16_t usDataLen)
{
	uint32_t uiLimitLen = 0; 
	
	uiLimitLen = CM_DATA_GET_MIN((uint32_t)sizeof(pstWzGprsWork->caIMEI),(uint32_t)usDataLen); 
	
    if(memcmp(pstWzGprsWork->caIMSI,pucImsi,uiLimitLen) != 0)
	{
	    memcpy(pstWzGprsWork->caIMSI,pucImsi,uiLimitLen); 
		
		mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_IMSI,0,uiLimitLen,(uint8_t *)pstWzGprsWork->caIMSI);
	}
}
/**
 * @brief 接收 csq 数据 
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            ucCsq: 数据 
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_inter_msg_rx_csq(WZGPRS_WORK_T *pstWzGprsWork,uint8_t ucCsq)
{
    if(ucCsq != pstWzGprsWork->ucCsq)
	{
		pstWzGprsWork->ucCsq = ucCsq; 
	    mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_CSQ,ucCsq,0,RT_NULL);     
	}
}

/**
 * @brief    
 * @param[in] 
 *            
 * @param[out]   
 * @return
 * @note
 */ 
uint8_t wzgprs_inter_msg_file_get(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucFrData,uint16_t usDataLen)
{
    uint8_t    ucFileNameSize = 0 ; 
	uint16_t   usIdx          = 0 ; 
	uint32_t   uiStarAddr     = 0 ; 
    uint32_t   uiSendLen      = 0 ; 
    uint32_t   uiFileSize     = 0 ; 
	uint32_t   *puiInt        = RT_NULL ; 
   
    if(pstWzGprsWork->stFileDown.ucDownWorkStat == WZGPRS_UPGRADE_FILETRANS)
	{   
		ucFileNameSize = pucFrData[0] ; 
        if((ucFileNameSize + 13 + pstWzGprsWork->stFileDown.uiGetLen) <= usDataLen)
		{  
			usIdx  = ucFileNameSize + 1 ; 
		    puiInt = (uint32_t *)&pucFrData[usIdx] ; 
			
			uiStarAddr  = *puiInt ; 
			
			usIdx      += 4 ; 
			puiInt      = (uint32_t *)&pucFrData[usIdx] ;
			
			uiSendLen   = *puiInt ; 
			
			usIdx      += 4 ; 
			puiInt      = (uint32_t *)&pucFrData[usIdx] ;
			
			uiFileSize  = *puiInt ; 
			usIdx      += 4 ; 
			
			if((uiStarAddr != pstWzGprsWork->stFileDown.uiStartIndex) 
				|| (uiSendLen != pstWzGprsWork->stFileDown.uiGetLen))
			{
			    rt_kprintf("Get File Respone Parameter abnormal\r\n"); 
                return 1 ; 				
			} 
			
			if(0 == pstWzGprsWork->stFileDown.uiTotalLen)
			{
			    if(uiFileSize < 1024) 
				{
				    rt_kprintf("File Size is too samll: %d\r\n",uiFileSize); 
                    return 2 ; 					
				}

                pstWzGprsWork->stFileDown.uiTotalLen = uiFileSize ;  				
			}
			else
			{
			    if(pstWzGprsWork->stFileDown.uiTotalLen != uiFileSize)
				{
					rt_kprintf("The file size of the two uploads is inconsistent: %d %d\r\n",
					                    pstWzGprsWork->stFileDown.uiTotalLen,uiFileSize); 
                    return 3 ; 
				}
			} 
			
			mq_service_wzgprs_send_msg_to_http(WZGPRS_TO_HTTP_DOWN_FILE,uiFileSize,uiSendLen,&pucFrData[usIdx]) ; 
		
			if((uiStarAddr + uiSendLen) < uiFileSize)
			{
			    pstWzGprsWork->stFileDown.uiStartIndex +=  uiSendLen ; 
                pstWzGprsWork->stFileDown.uiGetLen      =  (uiFileSize - (uiStarAddr + uiSendLen)) > 1024 
				                                             ? 1024 : (uiFileSize - (uiStarAddr + uiSendLen))	; 
                pstWzGprsWork->stFileDown.uiStartTick  = rt_tick_get(); 
				pstWzGprsWork->stFileDown.uiTimeCnt    = 0 ; 
				wzgprs_device_get_file(pstWzGprsWork->stFileDown.sFileName,pstWzGprsWork->stFileDown.ucFileNameSize,
		             pstWzGprsWork->stFileDown.uiStartIndex,pstWzGprsWork->stFileDown.uiGetLen) ; 
                return 0 ; 				
			}
			
			rt_kprintf("Get File Cplt\r\n"); 
		    memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T));
			return 0 ; 
		} 
		
		return 0xff ; 
	}		
	  
    return 0xff ; 	
} 

/**
 * @brief    
 * @param[in] 
 *            
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_file_download_cplt(WZGPRS_WORK_T *pstWzGprsWork)
{
    rt_kprintf("File download cplt\r\n");  
    
	pstWzGprsWork->stFileDown.ucDownWorkStat = WZGPRS_UPGRADE_FILETRANS ; 
	pstWzGprsWork->stFileDown.uiStartTick  = rt_tick_get(); 
    pstWzGprsWork->stFileDown.uiStartIndex = 0 ; 
	pstWzGprsWork->stFileDown.uiTotalLen   = 0 ;                /* 首次不知道总长,先置零 */
    pstWzGprsWork->stFileDown.uiGetLen     = 1024 ; 
	pstWzGprsWork->stFileDown.uiTimeCnt    = 0 ; 
	
    wzgprs_device_get_file(pstWzGprsWork->stFileDown.sFileName,pstWzGprsWork->stFileDown.ucFileNameSize,
		             pstWzGprsWork->stFileDown.uiStartIndex,pstWzGprsWork->stFileDown.uiGetLen) ;
    	
} 

/**
 * @brief    
 * @param[in] 
 *            
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_device_data_downstate(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucFrData,uint16_t usDataLen)
{
	uint8_t    ucFileNameSize = 0 ; 
	uint8_t    ucState        = 0 ; 
	
    if(pstWzGprsWork->stFileDown.ucDownWorkStat == WZGPRS_UPGRADE_DOWNLOAD)
	{
	    ucFileNameSize = pucFrData[0] ; 
        if((ucFileNameSize + 2) <= usDataLen)
		{
			ucState = pucFrData[ucFileNameSize + 1] ; 
		    switch(ucState)
			{
			    case Undownload:                    /* 未下载 */
					break; 
                case Downloading:                   /* 正在下载 */
					break ;
                case DownloadSuccess:               /* 下载成功 */
					wzgprs_inter_msg_file_download_cplt(pstWzGprsWork) ; 
					break ;
                case DownloadFail:                  /* 下载失败 */ 
					rt_kprintf("WZ Device Respone File download fail\r\n"); 
		            memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T));
                    break ; 					
			}
		}			
	}
} 
/**
 * @brief    
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            ucReason: 失败原因 
 * @param[out]   
 * @return
 * @note
 */ 
void wzgprs_inter_msg_device_data_downfailreason(WZGPRS_WORK_T *pstWzGprsWork,uint8_t ucReason)
{
    if(pstWzGprsWork->stFileDown.ucDownWorkStat == WZGPRS_UPGRADE_DOWNLOAD
		&& ucReason > 0)
	{
	    rt_kprintf("File download fail: %d\r\n",ucReason); 
		memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T));
	}
} 
/**
 * @brief 设备数据 
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            pucFrData: 帧数据
 *            uiDataLen: 数据长度
 * @param[out]   
 * @return 0: 设备数据错误 非0: 实际的一帧数据长度
 * @note
 */ 
void wzgprs_inter_msg_device_data_parsing(WZGPRS_WORK_T *pstWzGprsWork,uint8_t ucCmd,uint8_t *pucFrData,uint16_t usDataLen)
{
    switch(ucCmd)
	{
	    case GetVERN:                   /* 获取模块软件版本 */
            break; 
		case GetIMEI:                   /* 获取模块IMEI,模块也会定时上传 */
            wzgprs_inter_msg_rx_imei(pstWzGprsWork,pucFrData,usDataLen); 
		    break; 
		case GetIMSI:                   /* 获取模块的IMSI,模块也会定时上传 */
            wzgprs_inter_msg_rx_imsi(pstWzGprsWork,pucFrData,usDataLen);
		    break; 
		case GetCSQ:                    /* 获取模块的CSQ,模块也会定时上传 */
		    wzgprs_inter_msg_rx_csq(pstWzGprsWork,pucFrData[0]);
		    break; 
		case TCPSTATUS:                 /* 获取模块的TCPSTATUS，模块也会定时上传 */
            wzgprs_inter_msg_device_data_tcpstate(pstWzGprsWork,pucFrData[0]); 
		    break; 
		case GetLng:                    /* 获取模块的经度 */
            break;    
		case GetLat:                    /* 获取模块的维度 */
            break; 
		case Update:                    /* 更新脚本 */
            break;    
		case ConnectionType:            /* 连接类型 */
            break; 
	    case SetIPP:                    /* 设置IP地址 */
            break;
	    case FileDownLoad:              /* 文件下载 */
            break; 
		case DownLoadState:             /* 下载状态 */
			wzgprs_inter_msg_device_data_downstate(pstWzGprsWork,pucFrData,usDataLen);
            break; 
		case FileFetch:                 /* 获取下载的文件 */
            break; 
		case FileTrans:                 /* 文件数据上传 */
			wzgprs_inter_msg_file_get(pstWzGprsWork,pucFrData,usDataLen) ; 
            break; 
		case UpdateFailReason:          /* 更新失败原因 */
			wzgprs_inter_msg_device_data_downfailreason(pstWzGprsWork,pucFrData[usDataLen - 1]); 
			break; 
	}
} 

/**
 * @brief 设备数据 
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            pucFrData: 帧数据
 *            uiDataLen: 数据长度
 * @param[out]   
 * @return 0: 设备数据错误 非0: 实际的一帧数据长度
 * @note
 */ 
uint32_t wzgprs_inter_msg_device_data(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucFrData,uint32_t uiDataLen)
{
    WZGPRS_DEVICE_FRHEAD_T *pstFrHead = RT_NULL; 
    uint8_t ucCrc0 = 0 ,ucCrc1 = 0; 

    if(uiDataLen < WZGPRS_DEVICE_FRHEAD_LEN + 1)   /* 数据长度不对 */
    {
        rt_kprintf("wzgprs upload data length error\r\n"); 
        return 0; 
    } 

    pstFrHead = (WZGPRS_DEVICE_FRHEAD_T *)pucFrData; 
    if(pstFrHead->usDataLen < 1)                  /* 除指令外最少有一字节数据 */
    {
        rt_kprintf("wzgprs upload effective data length error\r\n");
        return 0; 
    } 

    ucCrc0 = wzgprs_crc8_MAXIM(&pucFrData[1],7);
    if(ucCrc0 != pstFrHead->ucCrc0)
    {
        rt_kprintf("Frame header verification failed\r\n"); 
        return 0;   
    } 

    ucCrc1 = wzgprs_crc8_MAXIM(&pucFrData[WZGPRS_DEVICE_FRHEAD_LEN],pstFrHead->usDataLen);
    if(ucCrc1 != pstFrHead->ucCrc1)
    {
        rt_kprintf("Frame valid data verification failed\r\n"); 
        return 0;   
    } 

    pstWzGprsWork->uiRecvTick = rt_tick_get();
    wzgprs_inter_msg_device_data_parsing(pstWzGprsWork,pucFrData[WZGPRS_DEVICE_FRHEAD_LEN],&pucFrData[WZGPRS_DEVICE_FRHEAD_LEN + 1],pstFrHead->usDataLen - 1); 

    return (pstFrHead->usDataLen + WZGPRS_DEVICE_FRHEAD_LEN); 
} 

/**
 * @brief 串口接收到的消息处理 
 * @param[in]  stWzGprsWork ：指向gprs工作的参数 
 *             pucMsgData   ：消息数据
 *             uiDataLen    ：数据长度
 * @param[out]   
 * @return 0 ：设备数据错误 非0：实际的一帧数据长度
 * @note
 */ 
uint8_t wzgprs_inter_msg_serial_recv(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucMsgData,uint32_t uiDataLen)
{
    uint8_t *pucData = RT_NULL; 
    uint32_t uiLen = 0;
    uint32_t uiFrLen = 0;

    pucData = pucMsgData; 
    uiLen   = uiDataLen; 

    do 
    {
        pucData  += uiFrLen; 
        uiLen -= uiFrLen; 

        // 不是设备指令帧 是数据
        if(WZGPRS_DEVICE_FR_HEAD != pucData[0])          
        {
            mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_DATA,0,uiLen,pucData);
            return 0; 
        }
        
        uiFrLen = wzgprs_inter_msg_device_data(pstWzGprsWork,pucData,uiLen);  
        
    } while((uiFrLen > 0) && (uiFrLen < uiLen)); 

    return 1; 
} 

/**
 * @brief  
 * @param[in] 
 * @param[out]   
 * @return
 * @note
 */
void wzgprs_time_tx_netstat(WZGPRS_WORK_T *pstWzGprsWork)
{
	uint32_t uiTick = 0; 
	
	uiTick = rt_tick_get(); 
    if(uiTick - pstWzGprsWork->uiTxTcpStatTick >= CM_TIME_500_MSEC)
	{
	    pstWzGprsWork->uiTxTcpStatTick  = uiTick;  
        mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_NETSTATE,pstWzGprsWork->ucTcpStat,0,RT_NULL); 		
	}
}
/**
 * @brief 定时事件消息 
 * @param[in] pstWzGprsWork: 指向gprs工作的参数 
 * @param[out]   
 * @return 
 * @note
 */
void wzgprs_is_offline(WZGPRS_WORK_T *pstWzGprsWork)
{
	uint32_t uiTick = rt_tick_get() ; 
	
    if((COMMON_INVALID_FLAG == pstWzGprsWork->ucOfflineFlag)            /* 设备没有离线 */
		&& (uiTick - pstWzGprsWork->uiRecvTick > CM_TIME_60_SEC))       /* 60 秒没有接收到设备数据 */ 
	{
	    pstWzGprsWork->ucOfflineFlag = COMMON_VALID_FLAG ;  
		pstWzGprsWork->ucTcpStat     = 0 ; 
		pstWzGprsWork->uiTxTcpStatTick  = uiTick;  
        mq_service_wzgprs_send_msg_to_tcu(WZGPRS_TO_TCU_NETSTATE,pstWzGprsWork->ucTcpStat,0,RT_NULL); 
        memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T)); 
		rt_kprintf("wz gprs device offline\r\n");                          /* 到设备离线 */
		WZGPRS_JUMP_NEW_STATE(pstWzGprsWork,WZGPRS_WORK_DEVICE_OFFLINE); 		
	}		
} 
/**
 * @brief 更新超时处理
 * @param[in] pstWzGprsWork: 指向gprs工作的参数 
 * @param[out]   
 * @return 
 * @note
 */ 
void wzgprs_upgrade_timeous_proc(WZGPRS_WORK_T *pstWzGprsWork)
{
	uint32_t   uiTimeous = 0 ,uiTick = 0 ; 
	
    if(WZGPRS_UPGRADE_DOWNLOAD == pstWzGprsWork->stFileDown.ucDownWorkStat)
	{
	    uiTimeous = (rt_tick_get() - pstWzGprsWork->stFileDown.uiStartTick) / 1000 ; 
        if(uiTimeous > pstWzGprsWork->stFileDown.uiTimeous) 
		{
		     rt_kprintf("File download timedout\r\n"); 
		     memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T)); 
             return ; 			
		}

		if(++pstWzGprsWork->stFileDown.uiTimeCnt > 10)
		{
		    pstWzGprsWork->stFileDown.uiTimeCnt = 0 ; 
			wzgprs_device_start_file_down(pstWzGprsWork->stFileDown.sFileName,pstWzGprsWork->stFileDown.ucFileNameSize,
		               pstWzGprsWork->stFileDown.sUrl,pstWzGprsWork->stFileDown.ucUrlSize);
		}
        
        return ; 		
	}
	
	if(WZGPRS_UPGRADE_FILETRANS == pstWzGprsWork->stFileDown.ucDownWorkStat)
	{
		uiTick    = rt_tick_get() ; 
	    uiTimeous = uiTick - pstWzGprsWork->stFileDown.uiStartTick ; 
        if(uiTimeous > CM_TIME_2_SEC) 
		{
			pstWzGprsWork->stFileDown.uiStartTick = uiTick ; 
			if(++pstWzGprsWork->stFileDown.uiTimeCnt > 20)
			{
			    rt_kprintf("File download timedout\r\n"); 
		        memset(&pstWzGprsWork->stFileDown,0,sizeof(WZGPRS_HTTP_DOWN_T)); 
                return ;    
			}
			
			wzgprs_device_get_file(pstWzGprsWork->stFileDown.sFileName,pstWzGprsWork->stFileDown.ucFileNameSize,
		             pstWzGprsWork->stFileDown.uiStartIndex,pstWzGprsWork->stFileDown.uiGetLen) ;   			
		}		 
	}
}
/**
 * @brief 
 * @param[in] 
 * @param[out]   
 * @return 
 * @note
 */ 
void wzgprs_inter_time_update_ip(WZGPRS_WORK_T *pstWzGprsWork)
{
	uint32_t  uiTime = 0 ; 
	
	uiTime = pstWzGprsWork->ucWorkState == WZGPRS_WORK_NET_ONLINE 
	          ? 10000 : 10 ; 
	
	if(++pstWzGprsWork->uiTimeCnt > uiTime)
	{
	    pstWzGprsWork->uiTimeCnt = 0 ; 
		wzgprs_device_set_remote(WZGPRS_REMOTE_ADDR,strlen(WZGPRS_REMOTE_ADDR)); 
	}   
}
/**
 * @brief 定时事件消息 
 * @param[in] pstWzGprsWork: 指向gprs工作的参数 
 * @param[out]   
 * @return 
 * @note
 */ 
void wzgprs_inter_msg_time(WZGPRS_WORK_T *pstWzGprsWork)
{
    wzgprs_time_tx_netstat(pstWzGprsWork);  

    wzgprs_is_offline(pstWzGprsWork) ; 

    wzgprs_upgrade_timeous_proc(pstWzGprsWork) ;  

    wzgprs_inter_time_update_ip(pstWzGprsWork) ; 	
}

/**
 * @brief 万桩gprs模块内部消息处理
 * @param[in] stWzGprsWork: 指向gprs工作的参数 
 *            uiMsgCode: 消息码
 *            uiMsgVar: 消息参数
 *            pucMsgData: 消息数据
 *            uiDataLen: 数据长度
 * @param[out]   
 * @return 0: 设备数据错误 非0: 实际的一帧数据长度
 * @note
 */ 
void wzgprs_inter_msg_proc(WZGPRS_WORK_T *pstWzGprsWork,uint32_t uiMsgCode,uint32_t uiMsgVar,uint8_t *pucMsgData,uint32_t uiDataLen)
{
	switch(uiMsgCode)
	{
	    case WZGPRS_SERIAL_RECV:
			wzgprs_inter_msg_serial_recv(pstWzGprsWork,pucMsgData,uiDataLen); 
			break; 
		case WZGPRS_TIMER:
			wzgprs_inter_msg_time(pstWzGprsWork); 
			break;
        default:
            break; 			
	} 
}
