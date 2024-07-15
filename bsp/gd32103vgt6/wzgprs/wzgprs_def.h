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
#ifndef _WZGPRS_DEF_H_
#define _WZGPRS_DEF_H_

#include <stdint.h> 
#include <time.h>
#include <rtthread.h>

#define  WZGPRS_READ_CMD       0x10
#define  WZGPRS_WRITE_CMD      0x11 

#define  WZGPRS_DEVICE_FR_HEAD       0xFA
//#define  WZGPRS_FILE_NAME           "rtthread.rbl"
//#define  WZGPRS_FILE_URL            "http://test-webapi.wanzhuangkj.com/gatewaymsg/device/upgrade"

/* 万桩GPRS模块指令表,直接从提供的文件中copy过来的 */
enum WZGPRS_CMD_TAB {
    GetVERN  = 1,             /* 获取模块软件版本 */
    GetIMEI,                  /* 获取模块IMEI,模块也会定时上传 */
    GetIMSI,                  /* 获取模块的IMSI,模块也会定时上传 */
    GetCSQ,                   /* 获取模块的CSQ,模块也会定时上传 */
    TCPSTATUS,                /* 获取模块的TCPSTATUS，模块也会定时上传 */
    GetLng,                   /* 获取模块的经度 */
    GetLat,                   /* 获取模块的维度 */
    Update,                   /* 更新脚本 */
    ConnectionType,           /* 连接类型 */
    TxData ,                  /* 发送数据 */ 
	
	SetIPP = 21,              /* 设置IP地址 */
    
	FileDownLoad = 23,        /* 文件下载 */
    DownLoadState,            /* 下载状态 */
    FileFetch,                /* 获取下载的文件 */
    FileTrans,                /* 文件数据上传 */
    UpdateFailReason          /* 更新失败原因 */
}; 

/* 万桩GPRS模块文件下载状态 */
enum WZGPRS_FILE_DOWNLOADSTATE
{
    Undownload = 0,           /* 未下载 */
    Downloading,              /* 正在下载 */
    DownloadSuccess,          /* 下载成功 */
    DownloadFail              /* 下载失败 */

}; 

/* 万桩GPRS模块工作状态 */
enum WZGPRS_WORK_STATE 
{
    WZGPRS_WORK_IDIE,           /* 模块空闲 */
    WZGPRS_WORK_NET_OFFLINE, 
	WZGPRS_WORK_NET_ONLINE,     /* 模块网络在线 */ 
	WZGPRS_WORK_DEVICE_OFFLINE,        /* 模块离线 */
    WZGPRS_WORK_DELAY,    
}; 

enum WZGPRS_INTERMSG_CODE
{
    WZGPRS_SERIAL_RECV = 0,      /* 串口接收数据 */
    WZGPRS_TIMER,                /* 万桩定时器事件 */    
}; 

enum WZGPRS_UPGRADE_STATE
{
    WZGPRS_UPGRADE_IDIE = 0 ,     /* 空闲 */
    WZGPRS_UPGRADE_DOWNLOAD,      /* 下载中 */
    WZGPRS_UPGRADE_FILETRANS      /* 文件传输 */ 
} ; 

typedef struct
{
    uint8_t     ucDownWorkStat;
	uint8_t     ucUrlSize     ; 
	uint8_t     ucFileNameSize    ; 
	uint8_t     ucRev         ; 
	
	char        sUrl[100]        ;
	char        sFileName[50]    ; 
	uint32_t    uiTimeous        ;                /* 下载超时时间 */
    uint32_t    uiStartTick   ; 	
	uint32_t    uiTimeCnt; 
	
	uint32_t    uiStartIndex ; 
	uint32_t    uiTotalLen   ; 
	uint32_t    uiGetLen    ; 
}WZGPRS_HTTP_DOWN_T; 

typedef struct
{
    uint8_t             ucWorkState;            /* 工作状态 */
	uint8_t             ucCsq;                  /* csq 信号质量 */
	uint8_t             ucOfflineFlag;          /* 工作模块离线标志 */
	uint8_t             ucTcpStat;              /* tcp 状态 */
	char                caIMEI[24];             /* imei 码 */
	char                caIMSI[24];             /* imsi 码 */
	uint32_t            uiRecvTick;             /* 接收数据的时间 */
	uint32_t            uiJumpTick;             /* 状态跳转时间 */ 
	uint32_t            uiTxTcpStatTick;        /* 发送TCP状态时间 */
	uint32_t            uiTimeCnt      ;        /* 定时 */
	WZGPRS_HTTP_DOWN_T  stFileDown; 
}WZGPRS_WORK_T; 

typedef struct __attribute__((packed)) 
{
    uint8_t     ucFrHead;                       /* 帧头*/
	uint8_t     ucIdData[4];                    /* id数据 */
	uint16_t    usDataLen;                      /* 数据长度 */
	uint8_t     ucCtlWord;                      /* 控制字 */
	uint8_t     ucCrc0;                         /* 前面数据的crc */
	uint8_t     ucCrc1;                         /* 后面数据的crc */
}WZGPRS_DEVICE_FRHEAD_T; 

#define  WZGPRS_DEVICE_FRHEAD_LEN       sizeof(WZGPRS_DEVICE_FRHEAD_T)	
#define  WZGPRS_JUMP_NEW_STATE(pstWzGprsWork,ucNewstate)    do { \
                                                                pstWzGprsWork->ucWorkState = ucNewstate;   \
                                                                pstWzGprsWork->uiJumpTick  = rt_tick_get();\
                                                              } while(0)

uint8_t  wzgprs_crc8_MAXIM(uint8_t *ptr, uint16_t len); 
uint16_t wzgprs_crc16(uint8_t *ptr, uint16_t len); 
int wzgprs_serial_recive_init(void);
void    wzgprs_serial_send_data(uint8_t *pcData,uint16_t usLen);	
uint8_t wzgprs_device_data_send(uint8_t ucCtlWord,uint8_t ucCmd,uint8_t *pucData,uint16_t usDataLen) ;															  
void wzgprs_inter_msg_proc(WZGPRS_WORK_T *pstWzGprsWork,uint32_t uiMsgCode,uint32_t uiMsgVar,uint8_t *pucMsgData,uint32_t uiDataLen); 
void wzgprs_device_start_file_down(char *sFileName,uint8_t ucFileLen,char *sUrl,uint8_t ucUrlLen);
void wzgprs_device_get_file(char *sFileName,uint8_t ucFileNameLen,uint32_t uiStartIndex,uint32_t uiGetLen) ; 
#endif 
