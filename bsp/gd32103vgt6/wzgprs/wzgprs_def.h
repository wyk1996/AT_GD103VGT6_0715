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

/* ��׮GPRSģ��ָ���,ֱ�Ӵ��ṩ���ļ���copy������ */
enum WZGPRS_CMD_TAB {
    GetVERN  = 1,             /* ��ȡģ������汾 */
    GetIMEI,                  /* ��ȡģ��IMEI,ģ��Ҳ�ᶨʱ�ϴ� */
    GetIMSI,                  /* ��ȡģ���IMSI,ģ��Ҳ�ᶨʱ�ϴ� */
    GetCSQ,                   /* ��ȡģ���CSQ,ģ��Ҳ�ᶨʱ�ϴ� */
    TCPSTATUS,                /* ��ȡģ���TCPSTATUS��ģ��Ҳ�ᶨʱ�ϴ� */
    GetLng,                   /* ��ȡģ��ľ��� */
    GetLat,                   /* ��ȡģ���ά�� */
    Update,                   /* ���½ű� */
    ConnectionType,           /* �������� */
    TxData ,                  /* �������� */ 
	
	SetIPP = 21,              /* ����IP��ַ */
    
	FileDownLoad = 23,        /* �ļ����� */
    DownLoadState,            /* ����״̬ */
    FileFetch,                /* ��ȡ���ص��ļ� */
    FileTrans,                /* �ļ������ϴ� */
    UpdateFailReason          /* ����ʧ��ԭ�� */
}; 

/* ��׮GPRSģ���ļ�����״̬ */
enum WZGPRS_FILE_DOWNLOADSTATE
{
    Undownload = 0,           /* δ���� */
    Downloading,              /* �������� */
    DownloadSuccess,          /* ���سɹ� */
    DownloadFail              /* ����ʧ�� */

}; 

/* ��׮GPRSģ�鹤��״̬ */
enum WZGPRS_WORK_STATE 
{
    WZGPRS_WORK_IDIE,           /* ģ����� */
    WZGPRS_WORK_NET_OFFLINE, 
	WZGPRS_WORK_NET_ONLINE,     /* ģ���������� */ 
	WZGPRS_WORK_DEVICE_OFFLINE,        /* ģ������ */
    WZGPRS_WORK_DELAY,    
}; 

enum WZGPRS_INTERMSG_CODE
{
    WZGPRS_SERIAL_RECV = 0,      /* ���ڽ������� */
    WZGPRS_TIMER,                /* ��׮��ʱ���¼� */    
}; 

enum WZGPRS_UPGRADE_STATE
{
    WZGPRS_UPGRADE_IDIE = 0 ,     /* ���� */
    WZGPRS_UPGRADE_DOWNLOAD,      /* ������ */
    WZGPRS_UPGRADE_FILETRANS      /* �ļ����� */ 
} ; 

typedef struct
{
    uint8_t     ucDownWorkStat;
	uint8_t     ucUrlSize     ; 
	uint8_t     ucFileNameSize    ; 
	uint8_t     ucRev         ; 
	
	char        sUrl[100]        ;
	char        sFileName[50]    ; 
	uint32_t    uiTimeous        ;                /* ���س�ʱʱ�� */
    uint32_t    uiStartTick   ; 	
	uint32_t    uiTimeCnt; 
	
	uint32_t    uiStartIndex ; 
	uint32_t    uiTotalLen   ; 
	uint32_t    uiGetLen    ; 
}WZGPRS_HTTP_DOWN_T; 

typedef struct
{
    uint8_t             ucWorkState;            /* ����״̬ */
	uint8_t             ucCsq;                  /* csq �ź����� */
	uint8_t             ucOfflineFlag;          /* ����ģ�����߱�־ */
	uint8_t             ucTcpStat;              /* tcp ״̬ */
	char                caIMEI[24];             /* imei �� */
	char                caIMSI[24];             /* imsi �� */
	uint32_t            uiRecvTick;             /* �������ݵ�ʱ�� */
	uint32_t            uiJumpTick;             /* ״̬��תʱ�� */ 
	uint32_t            uiTxTcpStatTick;        /* ����TCP״̬ʱ�� */
	uint32_t            uiTimeCnt      ;        /* ��ʱ */
	WZGPRS_HTTP_DOWN_T  stFileDown; 
}WZGPRS_WORK_T; 

typedef struct __attribute__((packed)) 
{
    uint8_t     ucFrHead;                       /* ֡ͷ*/
	uint8_t     ucIdData[4];                    /* id���� */
	uint16_t    usDataLen;                      /* ���ݳ��� */
	uint8_t     ucCtlWord;                      /* ������ */
	uint8_t     ucCrc0;                         /* ǰ�����ݵ�crc */
	uint8_t     ucCrc1;                         /* �������ݵ�crc */
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
