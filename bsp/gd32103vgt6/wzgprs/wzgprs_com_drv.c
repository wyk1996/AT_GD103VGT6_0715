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
#include "wzgprs_com_drv.h"
#include "rtthread.h"
#include "drv_gpio.h"

/* 万桩使用的串口名 */
#define WZGPRS_SERIAL_NAME "uart2"

/* 硬件拓展结构体 */
typedef struct
{
	/* 用于串口接收的信号量 */
    rt_sem_t wz_sem;
	/* 字节间的超时时间 */
    uint32_t byte_timeout;
} wz_port_t;

/*硬件拓展结构体变量 */
static wz_port_t wz_port = 
{
	/* 用于串口接收的信号量 */
    .wz_sem = RT_NULL,
	/* 接收字节间超时时间 */
    .byte_timeout = 10,
};
/* l501 采集设备句柄 */
static rt_device_t wz_device = RT_NULL;
/* l501 采集接收信号量 */
static struct rt_semaphore wz_receive_sem;
/* 串口配置参数 */
struct serial_configure uart2_config = RT_SERIAL_CONFIG_DEFAULT;

/* 串口接收数据回调函数 */
rt_err_t uart2_handler(rt_device_t dev, rt_size_t size)
{
    /* 接收到一个数据释放信号量 */
    rt_sem_release(&wz_receive_sem);
    return RT_EOK;
}

/**
 * @brief  硬件层接收数据 
 * @param[in] @ctx:运行环境
 *            @msg:接收数据存放地址
 *            @len:数据最大接收长度  
 * @param[out]   
 * @return
 * @note 
 */
static int wz_hw_read(WZ_COM_T *ctx, uint8_t *msg ,uint16_t len)
{
    /* 实际接收长度 */
    int read_len = 0;
    /* 清缓存变量 */
//    uint8_t buf = 0;
    
//    /* 清空缓存 */
//    while(rt_device_read(wz_device,0,&buf,1));
    /* 等待串口接收到数据 */
    if(rt_sem_take(&wz_receive_sem, 1000) == -RT_ETIMEOUT)
    {
        return 0;
    }
    /* 每次读取一个字节的数据 */
    while (rt_device_read(wz_device, 0, msg + read_len, 1) == 1)
    {
//        if(read_len > len)
//        {
//            return 0;
//        }
//        else
//        {
            read_len ++;
		    if(read_len >= len)             // 数据已经到达
			{
			    break; 
			}
//        }
        /* 读取超时标志一帧数据读取完成 */
        if (rt_sem_take(&wz_receive_sem, ((wz_port_t *)(ctx->port_data))->byte_timeout) == -RT_ETIMEOUT)
        {
            break;
        }
    }
    return read_len;
}
/**
 * @brief 
 * @param[in] @ctx:运行环境
 *            @msg:接收数据存放地址
 *            @len:数据最大接收长度   
 * @param[out]   
 * @return
 * @note 
 */
static int wz_hw_write(WZ_COM_T *ctx, uint8_t *buf, uint16_t len)
{
    /* 串口发送数据 */
    return rt_device_write(wz_device,0,buf,len);
}

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note 
 */
int wzgprs_port_init(void)
{
    /* 串口初始化 */
    wz_device = rt_device_find(WZGPRS_SERIAL_NAME);
    
    if (wz_device == RT_NULL)
    {
        rt_kprintf("cannot find device %s \r\n", WZGPRS_SERIAL_NAME);
        return -RT_ERROR;
    }
    
    if (rt_device_open(wz_device, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("cannot open device %s \r\n", WZGPRS_SERIAL_NAME);
        return -RT_ERROR;
    }
    else
    {
        uart2_config.baud_rate = BAUD_RATE_115200;
        uart2_config.data_bits = DATA_BITS_8;
        uart2_config.stop_bits = STOP_BITS_1;
        uart2_config.parity = PARITY_NONE;
        rt_device_control(wz_device, RT_DEVICE_CTRL_CONFIG, &uart2_config);
        rt_kprintf("device %s open success \r\n", WZGPRS_SERIAL_NAME);
    }

    /* 信号量初始化 */
    if (rt_sem_init(&wz_receive_sem, "receive_sem", 0, RT_IPC_FLAG_FIFO) == RT_EOK)
    {
        wz_port.wz_sem = &wz_receive_sem;
    }
    else
    {
        return -RT_ERROR;
    }

    /* 设置串口接收回调函数 */
    rt_device_set_rx_indicate(wz_device, uart2_handler);
    return  RT_EOK;
}

/* 万桩gprs环境结构体变量初始化 */
WZ_COM_T st_WzComParam = 
{
    {0},
    0,
    wz_hw_write,
    wz_hw_read,
    (void *)&wz_port
};


