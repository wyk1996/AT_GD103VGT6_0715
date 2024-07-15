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
 * @brief TCU ��������  
 * @param[in]  stWzGprsWork ��ָ��gprs�����Ĳ��� 
 *             pucMsgData   �����ص�ַ
 *             uiDataLen    ����ַ����
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
 * @brief TCU ���ظ��´���  
 * @param[in]  stWzGprsWork ��ָ��gprs�����Ĳ��� 
 *             pucMsgData   �����ص�ַ
 *             uiDataLen    ����ַ����
 * @param[out]   
 * @return 0:
 * @note
 */ 
void wzgprs_tcu_upgrade_proc(WZGPRS_WORK_T *pstWzGprsWork,uint8_t *pucMsgData,uint32_t uiDataLen)
{
	int iSizeUrl      = 0 ; 
	int iFileNameSize = 0 ; 
	
	S_UPDATE_CODE94 *pstUpdate = RT_NULL ; 
	
	if(WZGPRS_WORK_DEVICE_OFFLINE != pstWzGprsWork->ucWorkState                          /* �豸δ���� */
		&& WZGPRS_UPGRADE_IDIE == pstWzGprsWork->stFileDown.ucDownWorkStat)              /* �������� */
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

		/* ��ʼ����ʱ����ʼ������Ĭ�ϲ���300k�Ŀռ䣬������Ӱ�����ܣ�����ʱ��Ҫʮ���룬����Ҳ��Ҫ���� */
		mq_service_wzgprs_send_msg_to_http(WZGPRS_TO_HTTP_START_ERASE,300 * 1024,0,RT_NULL) ; 
		
        rt_kprintf("File start download\r\n"); 		
	}
        
         
}
/**
 * @brief ��׮gprsģ����մ���TCUģ����Ϣ 
 * @param[in]  stWzGprsWork ��ָ��gprs�����Ĳ��� 
 *             uiMsgCode    ����Ϣ��
 *             uiMsgVar     ����Ϣ����
 *             pucMsgData   ����Ϣ����
 *             uiDataLen    �����ݳ���
 * @param[out]   
 * @return 0:�豸���ݴ��� ��0:ʵ�ʵ�һ֡���ݳ���
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
