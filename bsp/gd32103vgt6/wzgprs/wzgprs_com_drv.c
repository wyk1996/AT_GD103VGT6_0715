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

/* ��׮ʹ�õĴ����� */
#define WZGPRS_SERIAL_NAME "uart2"

/* Ӳ����չ�ṹ�� */
typedef struct
{
	/* ���ڴ��ڽ��յ��ź��� */
    rt_sem_t wz_sem;
	/* �ֽڼ�ĳ�ʱʱ�� */
    uint32_t byte_timeout;
} wz_port_t;

/*Ӳ����չ�ṹ����� */
static wz_port_t wz_port = 
{
	/* ���ڴ��ڽ��յ��ź��� */
    .wz_sem = RT_NULL,
	/* �����ֽڼ䳬ʱʱ�� */
    .byte_timeout = 10,
};
/* l501 �ɼ��豸��� */
static rt_device_t wz_device = RT_NULL;
/* l501 �ɼ������ź��� */
static struct rt_semaphore wz_receive_sem;
/* �������ò��� */
struct serial_configure uart2_config = RT_SERIAL_CONFIG_DEFAULT;

/* ���ڽ������ݻص����� */
rt_err_t uart2_handler(rt_device_t dev, rt_size_t size)
{
    /* ���յ�һ�������ͷ��ź��� */
    rt_sem_release(&wz_receive_sem);
    return RT_EOK;
}

/**
 * @brief  Ӳ����������� 
 * @param[in] @ctx:���л���
 *            @msg:�������ݴ�ŵ�ַ
 *            @len:���������ճ���  
 * @param[out]   
 * @return
 * @note 
 */
static int wz_hw_read(WZ_COM_T *ctx, uint8_t *msg ,uint16_t len)
{
    /* ʵ�ʽ��ճ��� */
    int read_len = 0;
    /* �建����� */
//    uint8_t buf = 0;
    
//    /* ��ջ��� */
//    while(rt_device_read(wz_device,0,&buf,1));
    /* �ȴ����ڽ��յ����� */
    if(rt_sem_take(&wz_receive_sem, 1000) == -RT_ETIMEOUT)
    {
        return 0;
    }
    /* ÿ�ζ�ȡһ���ֽڵ����� */
    while (rt_device_read(wz_device, 0, msg + read_len, 1) == 1)
    {
//        if(read_len > len)
//        {
//            return 0;
//        }
//        else
//        {
            read_len ++;
		    if(read_len >= len)             // �����Ѿ�����
			{
			    break; 
			}
//        }
        /* ��ȡ��ʱ��־һ֡���ݶ�ȡ��� */
        if (rt_sem_take(&wz_receive_sem, ((wz_port_t *)(ctx->port_data))->byte_timeout) == -RT_ETIMEOUT)
        {
            break;
        }
    }
    return read_len;
}
/**
 * @brief 
 * @param[in] @ctx:���л���
 *            @msg:�������ݴ�ŵ�ַ
 *            @len:���������ճ���   
 * @param[out]   
 * @return
 * @note 
 */
static int wz_hw_write(WZ_COM_T *ctx, uint8_t *buf, uint16_t len)
{
    /* ���ڷ������� */
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
    /* ���ڳ�ʼ�� */
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

    /* �ź�����ʼ�� */
    if (rt_sem_init(&wz_receive_sem, "receive_sem", 0, RT_IPC_FLAG_FIFO) == RT_EOK)
    {
        wz_port.wz_sem = &wz_receive_sem;
    }
    else
    {
        return -RT_ERROR;
    }

    /* ���ô��ڽ��ջص����� */
    rt_device_set_rx_indicate(wz_device, uart2_handler);
    return  RT_EOK;
}

/* ��׮gprs�����ṹ�������ʼ�� */
WZ_COM_T st_WzComParam = 
{
    {0},
    0,
    wz_hw_write,
    wz_hw_read,
    (void *)&wz_port
};


