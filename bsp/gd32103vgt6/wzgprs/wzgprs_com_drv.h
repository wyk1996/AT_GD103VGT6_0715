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

#define WZ_COM_MAX_READ_LEN 200   //读数据的最大数据长度
#define WZ_COM_WRITE_LEN    50    //写数据的最大数据长度

// port setting
#define WZ_COM_DEFAULT_RESPONSE_TIMEOUT     500 //500ms
#define MAX_DEVICE_NAME_LEN 10              //最大设备名长度

//L501 环境结构体
typedef struct wz_com_t
{
    uint8_t addr[6];    //从机地址
    uint8_t debug;      //调试标志
    int (*write)(struct wz_com_t *ctx, uint8_t *buf, uint16_t len);     //底层写函数
    int (*read) (struct wz_com_t *ctx, uint8_t *msg, uint16_t len);     //底层读函数
    void *port_data;                                                //移植层拓展接口
} WZ_COM_T;

//对外提供环境声明
extern WZ_COM_T   st_WzComParam ;


//十进制转BCD码(32位)
extern uint32_t dec_to_bcd(uint32_t val);
//字符串转BCD形式
extern int str_to_bcd(char *str, uint8_t *bcd_store_address, uint16_t bcd_len);

//l501采集硬件层初始化
int wzgprs_port_init(void)  ; 

#endif









