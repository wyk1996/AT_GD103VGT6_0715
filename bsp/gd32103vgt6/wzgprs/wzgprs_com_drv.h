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

#ifndef __WZGPRS_COM_DRV_H 
#define __WZGPRS_COM_DRV_H 

#include <stdint.h>

#define WZ_COM_MAX_READ_LEN 200   //�����ݵ�������ݳ���
#define WZ_COM_WRITE_LEN    50    //д���ݵ�������ݳ���

// port setting
#define WZ_COM_DEFAULT_RESPONSE_TIMEOUT     500 //500ms
#define MAX_DEVICE_NAME_LEN 10              //����豸������

//L501 �����ṹ��
typedef struct wz_com_t
{
    uint8_t addr[6];    //�ӻ���ַ
    uint8_t debug;      //���Ա�־
    int (*write)(struct wz_com_t *ctx, uint8_t *buf, uint16_t len);     //�ײ�д����
    int (*read) (struct wz_com_t *ctx, uint8_t *msg, uint16_t len);     //�ײ������
    void *port_data;                                                //��ֲ����չ�ӿ�
} WZ_COM_T;

//�����ṩ��������
extern WZ_COM_T   st_WzComParam ;


//ʮ����תBCD��(32λ)
extern uint32_t dec_to_bcd(uint32_t val);
//�ַ���תBCD��ʽ
extern int str_to_bcd(char *str, uint8_t *bcd_store_address, uint16_t bcd_len);

//l501�ɼ�Ӳ�����ʼ��
int wzgprs_port_init(void)  ; 

#endif









