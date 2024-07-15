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
#define  WZGPRS_DEVICE_SEND_BUF_SIZE       256
/*
===============================================================================================================================
                                            ****** 变量定义 ******
===============================================================================================================================
*/
uint8_t  ucaSendBuf[WZGPRS_DEVICE_SEND_BUF_SIZE] = {0}; 
/*
===============================================================================================================================
                                            ****** 函数定义/声明 ******
===============================================================================================================================
*/

 

/**
 * @brief       发送指令数据到万桩数据设备
 * @param[in]   ucCmd         ：指令
 *              pucFrData     ：帧数据
 *              usDataLen     ：数据长度 
 * @param[out] 
 * @return
 * @note
 */
uint8_t wzgprs_device_data_send(uint8_t ucCtlWord,uint8_t ucCmd,uint8_t *pucData,uint16_t usDataLen)
{
    WZGPRS_DEVICE_FRHEAD_T *pstFrHead   = RT_NULL;   
    uint16_t               uiValidLen   = 0; 
	uint8_t                *pucDataBuf  = RT_NULL; 

//	RT_ASSERT(pucData   != RT_NULL);
//	RT_ASSERT(usDataLen > 0);
	
   	pstFrHead   = (WZGPRS_DEVICE_FRHEAD_T *)ucaSendBuf; 

    uiValidLen	= sizeof(ucaSendBuf) - sizeof(WZGPRS_DEVICE_FRHEAD_T); 
	
	if(usDataLen + 1 > uiValidLen)
	{
		rt_kprintf("send data too long\r\n"); 
	    return 1; 
	} 
	
	pstFrHead->ucFrHead  = WZGPRS_DEVICE_FR_HEAD; 
	pstFrHead->ucCtlWord = ucCtlWord; 
	pstFrHead->usDataLen = usDataLen + 1; 
	
	pstFrHead->ucCrc0 = wzgprs_crc8_MAXIM(&ucaSendBuf[1],7); 
	
	pucDataBuf    = ucaSendBuf + sizeof(WZGPRS_DEVICE_FRHEAD_T); 
	pucDataBuf[0] = ucCmd; 
	
	if(pucData != RT_NULL && usDataLen > 0)
	{
	    memcpy(pucDataBuf + 1,pucData,usDataLen);     
	}
	
	
	pstFrHead->ucCrc1 = wzgprs_crc8_MAXIM(pucDataBuf,usDataLen + 1); 
	
	wzgprs_serial_send_data(ucaSendBuf,sizeof(WZGPRS_DEVICE_FRHEAD_T) + usDataLen + 1); 
	
	memset(ucaSendBuf,0,sizeof(WZGPRS_DEVICE_FRHEAD_T) + usDataLen + 1); 
	
	return 0; 
}
/**
 * @brief       发送指令数据到万桩数据设备
 * @param[in]   sRemoteAddr ：指令
 *              ucLen       ：帧数据
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_device_set_remote(char *sRemoteAddr,uint8_t ucLen)
{
	uint8_t   ucBuf[64] = {0}; 
	
	ucBuf[0] = ucLen; 
	
	memcpy(&ucBuf[1],sRemoteAddr,CM_DATA_GET_MIN(sizeof(ucBuf) - 1,ucLen)); 
	
    wzgprs_device_data_send(WZGPRS_WRITE_CMD,SetIPP,ucBuf,ucLen + 1);      
} 
/**
 * @brief       
 * @param[in]           
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_device_get_remote(void)
{
    wzgprs_device_data_send(WZGPRS_READ_CMD,SetIPP,RT_NULL,0);      
}
/**
 * @brief       
 * @param[in]           
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_device_start_file_down(char *sFileName,uint8_t ucFileNameLen,char *sUrl,uint8_t ucUrlLen)
{
	uint8_t   ucBuf[256] = {0}; 
	uint8_t   ucIdxLen   = 0;   
	
	ucBuf[0] = ucFileNameLen; 
	
	memcpy(&ucBuf[1],sFileName,ucFileNameLen); 
	
	ucIdxLen = 1 + ucFileNameLen; 
	
	ucBuf[ucIdxLen] = ucUrlLen; 
	
	ucIdxLen += 1; 
	
	memcpy(&ucBuf[ucIdxLen],sUrl,ucUrlLen); 
	
	ucIdxLen += ucUrlLen; 
	
    wzgprs_device_data_send(WZGPRS_WRITE_CMD,FileDownLoad,ucBuf,ucIdxLen);      
}
/**
 * @brief       
 * @param[in]           
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_device_get_file(char *sFileName,uint8_t ucFileNameLen,uint32_t uiStartIndex,uint32_t uiGetLen)
{
	uint8_t   ucBuf[128] = {0}; 
	uint8_t   ucIdxLen   = 0;   
	uint32_t  *puiInt    = RT_NULL ; 
	
 	ucBuf[0] = ucFileNameLen; 
	
	memcpy(&ucBuf[1],sFileName,ucFileNameLen); 
	
	ucIdxLen = 1 + ucFileNameLen; 
	
	puiInt = (uint32_t  *)&ucBuf[ucIdxLen] ; 
	
	*puiInt = uiStartIndex ;
	ucIdxLen += 4 ; 
	puiInt = (uint32_t  *)&ucBuf[ucIdxLen] ;
	*puiInt = uiGetLen ;
	ucIdxLen += 4 ;
	
    wzgprs_device_data_send(WZGPRS_WRITE_CMD,FileFetch,ucBuf,ucIdxLen);      
}
/**
 * @brief       
 * @param[in]           
 * @param[out] 
 * @return
 * @note
 */
void wzgprs_device_get_file_down_stat(char *sFileName,uint8_t ucFileLen)
{
	uint8_t   ucBuf[64] = {0}; 
   
	ucBuf[0] = ucFileLen; 
	
	memcpy(&ucBuf[1],sFileName,CM_DATA_GET_MIN(sizeof(ucBuf) - 1,ucFileLen)); 
	
	ucBuf[1 + ucFileLen] = 0; 
	
    wzgprs_device_data_send(WZGPRS_READ_CMD,DownLoadState,ucBuf,ucFileLen + 2);      
}
