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
#include "protocol.h"

typedef struct __attribute__((packed)) 
{
	uint16_t		    usTxLen  ; 
	uint8_t				ucaBuf[0] ;			   
}WZ_TCU_TX_T ;

uint8_t    ucaTcuTxBuf[512] = {0} ; 
/**
 * @brief TCU 发送数据  
 * @param[in]  stWzGprsWork ：指向gprs工作的参数 
 *             pucMsgData   ：下载地址
 *             uiDataLen    ：地址长度
 * @param[out]   
 * @return 0:
 * @note
 */ 
void wzgprs_tcu_data_tx(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucTxData,uint32_t uiDataLen)
{
    uint32_t     uiRealLen = 0 ; 
	WZ_TCU_TX_T *pstTcuTx = RT_NULL ; 
	
	if(WZGPRS_WORK_NET_ONLINE == pstWzGprsWork->ucWorkState)	
	{
	    uiRealLen =  uiDataLen >= sizeof(ucaTcuTxBuf) ? sizeof(ucaTcuTxBuf) : uiDataLen ; 
        pstTcuTx  =  (WZ_TCU_TX_T *)ucaTcuTxBuf ;

        pstTcuTx->usTxLen = uiRealLen ; 
        memcpy(pstTcuTx->ucaBuf,pucTxData,uiRealLen) ;
        wzgprs_device_data_send(WZGPRS_WRITE_CMD,TxData,ucaTcuTxBuf,pstTcuTx->usTxLen + 2);   		
	}		
}
/**
 * @brief TCU 下载更新处理  
 * @param[in]  stWzGprsWork ：指向gprs工作的参数 
 *             pucMsgData   ：下载地址
 *             uiDataLen    ：地址长度
 * @param[out]   
 * @return 0:
 * @note
 */ 
void wzgprs_tcu_upgrade_proc(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucMsgData,uint32_t uiDataLen)
{
	int iSizeUrl      = 0 ; 
	int iFileNameSize = 0 ; 
	
	S_UPDATE_CODE94 *pstUpdate = RT_NULL ; 
	
	if(WZGPRS_WORK_DEVICE_OFFLINE != pstWzGprsWork->ucWorkState                          /* 设备未离线 */
		&& WZGPRS_UPGRADE_IDIE == pstWzGprsWork->stFileDown.ucDownWorkStat)              /* 升级空闲 */
	{
	    pstUpdate = (S_UPDATE_CODE94 *)pucMsgData ; 
		
		iSizeUrl = strlen(pstUpdate->url) ;
		if(iSizeUrl<= 0 || iSizeUrl > 100)
		{
			rt_kprintf("File download URL size error,size:%d\r\n",iSizeUrl);
		    return ; 
		} 
		
		iFileNameSize = strlen(pstUpdate->FileName) ; 
		if(iFileNameSize <= 0 || iFileNameSize > 50)
	    {
			rt_kprintf("File download filename size error,size:%d\r\n",iFileNameSize);
		    return ; 
		} 
			
		pstWzGprsWork->stFileDown.ucUrlSize      = iSizeUrl; 
	    pstWzGprsWork->stFileDown.ucFileNameSize = iFileNameSize   ; 
		
		memcpy(pstWzGprsWork->stFileDown.sUrl,pstUpdate->url,iSizeUrl) ;
		memcpy(pstWzGprsWork->stFileDown.sFileName,pstUpdate->FileName,iFileNameSize) ;
		
		pstWzGprsWork->stFileDown.ucDownWorkStat = WZGPRS_UPGRADE_DOWNLOAD ; 
		pstWzGprsWork->stFileDown.uiTimeCnt      = 0;
        pstWzGprsWork->stFileDown.uiStartTick    = rt_tick_get();
		pstWzGprsWork->stFileDown.uiTimeous      = pstUpdate->ucTimeOut ; 
        
		wzgprs_device_start_file_down(pstWzGprsWork->stFileDown.sFileName,pstWzGprsWork->stFileDown.ucFileNameSize,
		               pstWzGprsWork->stFileDown.sUrl,pstWzGprsWork->stFileDown.ucUrlSize);

		/* 开始下载时，开始擦除，默认擦除300k的空间，这样不影响性能，下载时需要十几秒，擦除也需要几秒 */
		mq_service_wzgprs_send_msg_to_http(WZGPRS_TO_HTTP_START_ERASE,300 * 1024,0,RT_NULL) ; 
		
        rt_kprintf("File start download\r\n"); 		
	}
        
         
}
/**
 * @brief 万桩gprs模块接收处理TCU模块消息 
 * @param[in]  stWzGprsWork ：指向gprs工作的参数 
 *             uiMsgCode    ：消息码
 *             uiMsgVar     ：消息参数
 *             pucMsgData   ：消息数据
 *             uiDataLen    ：数据长度
 * @param[out]   
 * @return 0:设备数据错误 非0:实际的一帧数据长度
 * @note
 */ 
void wzgprs_tcu_msg_proc(WZGPRS_WORK_T *pstWzGprsWork,uint32_t uiMsgCode,uint32_t uiMsgVar,uint8_t *pucMsgData,uint32_t uiDataLen)
{
	switch(uiMsgCode)
	{
	    case TCU_TO_WZGPRS_DATA:
			//wzgprs_serial_send_data(pucMsgData,uiDataLen); 
		    wzgprs_tcu_data_tx(pstWzGprsWork,pucMsgData,uiDataLen) ;
			break; 
		case TCU_TO_WZGPRS_FILEDOWN:
			wzgprs_tcu_upgrade_proc(pstWzGprsWork,pucMsgData,uiDataLen) ; 
			break;
        default:
            break; 			
	} 
}
