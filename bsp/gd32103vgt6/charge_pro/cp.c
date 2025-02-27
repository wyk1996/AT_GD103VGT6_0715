/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description cp采样
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <board.h>
#include <stdlib.h>
#include <board.h>

/* 引脚支持adc1通道10 */
#define   CP_ADC_NAME          "adc1"
#define   CP_ADC_CHANNEL       ADC_CHANNEL_1

/* 1ms */
#define   CP_PERIOD             1000
/* 533333 */
#define   CP_CH_PULSE           467//467 877                     
#define   CP_CH_NULL            0
/* 3300mv */
#define   CP_ADC_REFER_VOLT     3300
/* 转换位数为12位 */
#define   CP_ADC_CONVERT_BITS   (1ul << 12)
#define   CP_ADC_MIN_CONVERT_V  1300
#define   CP_ADC_DIFF            700
/* defined the PWM pin: PA0 */
#define PWM_RUN_PIN             0

/**
 * @brief 通用定时器1初始化PWM函数
 * @param[in] psr:时钟预分频系数,预分频值=psr+1
 *            arr:自动重装载值,计数次数=arr+1
 * @param[out]
 * @return
 * @note
 */
void timer1_pwm_init(uint32_t psr, uint32_t arr, uint32_t duty)
{
    /* 定义一个定时器初始化结构体 */
    timer_parameter_struct timer_init_struct;
    /* 定义一个定时器输出比较参数结构体*/
    timer_oc_parameter_struct timer_oc_init_struct;
    /* 开启定时器时钟 */
    rcu_periph_clock_enable(RCU_TIMER1);
    /* 开启GPIOA时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* 开启复用功能时钟 */
    rcu_periph_clock_enable(RCU_AF);

    /* 初始化PA0(TIMER1 CH0)为复用功能 */
    gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_0);

    /* 初始化定时器 */
    timer_deinit(TIMER1);
    timer_init_struct.prescaler			= psr;	                    /* 预分频系数 */
    timer_init_struct.period			= arr;	                    /* 自动重装载值 */
    timer_init_struct.alignedmode		= TIMER_COUNTER_EDGE;	    /* 计数器对齐模式，边沿对齐 */
    timer_init_struct.counterdirection	= TIMER_COUNTER_UP;		    /* 计数器计数方向，向上 */
    timer_init_struct.clockdivision		= TIMER_CKDIV_DIV1;		    /* DTS时间分频值 */
    timer_init_struct.repetitioncounter = 0;					    /* 重复计数器的值（定时器1无效）*/
    timer_init(TIMER1, &timer_init_struct);

    /* PWM初始化 */
    timer_oc_init_struct.outputstate  = TIMER_CCX_ENABLE;		    /* 通道使能 */
    timer_oc_init_struct.outputnstate = TIMER_CCXN_DISABLE;		    /* 通道互补输出使能（定时器1无效）*/
    timer_oc_init_struct.ocpolarity   = TIMER_OC_POLARITY_HIGH;	    /* 通道极性 */
    timer_oc_init_struct.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;    /* 互补通道极性（定时器1无效）*/
    timer_oc_init_struct.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;    /* 通道空闲状态输出（定时器1无效）*/
    timer_oc_init_struct.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;   /* 互补通道空闲状态输出（定时器1无效）*/
    timer_channel_output_config(TIMER1, TIMER_CH_0, &timer_oc_init_struct);

    /* 通道0占空比设置 */
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, duty);
    /* PWM模式0 */
    timer_channel_output_mode_config(TIMER1,TIMER_CH_0,TIMER_OC_MODE_PWM0);
    /* 不使用输出比较影子寄存器 */
    timer_channel_output_shadow_config(TIMER1,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);
    /* 自动重装载影子比较器使能 */
    timer_auto_reload_shadow_enable(TIMER1);
    /* 使能Timer1 */
    timer_enable(TIMER1);
}

/**
 * @brief pwm 占空比100%输出
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void cp_pwm_full(void)
{
    /* 修改比较值,修改占空比 */
    TIMER_CH0CV(TIMER1) =  CP_CH_NULL;
}

/**
 * @brief pwm 制定的 CP_CH_PULSE 输出
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void cp_pwm_ch_puls(void)
{
    TIMER_CH0CV(TIMER1) =  CP_CH_PULSE;
}

/**
 * @brief pwm 输出初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
int32_t cp_pwm_init(void)
{
    /* PWM管脚配置 */
    rt_pin_mode(PWM_RUN_PIN, PIN_MODE_OUTPUT);

    timer1_pwm_init(108 - 1, CP_PERIOD - 1, 300);

    cp_pwm_full();

    return RT_EOK;
}
/**
 * @brief configure the different system clocks
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rcu_config(void)
{
    /* enable GPIOC clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);
}

/**
 * @brief configure the GPIO peripheral
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void gpio_config(void)
{
    /* config the GPIO as analog mode */
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
}

/**
 * @brief configure the ADC peripheral
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void adc_config(void)
{
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1U);

    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
    /* ADC external trigger config */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

    /* enable ADC interface */
    adc_enable(ADC0);
    rt_hw_us_delay(960);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
}

/**
 * @brief ADC channel sample
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint16_t adc_channel_sample(uint8_t channel)
{
    /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 0U, channel, ADC_SAMPLETIME_71POINT5);
    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);

    /* wait the end of conversion flag */
    while(!adc_flag_get(ADC0, ADC_FLAG_EOC));
    /* clear the end of conversion flag */
    adc_flag_clear(ADC0, ADC_FLAG_EOC);
    /* return regular channel sample value */
    return (adc_regular_data_read(ADC0));
}

/**
 * @brief cp adc采样初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
int32_t cp_adc_init(void)
{
    rcu_config();
    /* GPIO configuration */
    gpio_config();
    /* ADC configuration */
    adc_config();

    return RT_EOK;
}

/**
 * @brief 获取cp电压采样值
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint32_t ch_get_cp_volt(uint32_t *puiCpVolt)
{
    uint32_t value0    = 0;
    uint32_t value1    = 0;
    uint32_t value     = 0;
    uint32_t uiDValue  = 0;

    uint32_t i = 0;
    uint32_t n = 0;
    uint32_t uiaBuf[8] = {0};
    uint32_t uiSum = 0;
    uint32_t uiVaule = 0;
    rt_err_t ret = RT_EOK;

    /* 使能设备 */
//    ret = rt_adc_enable(pcp_dev, CP_ADC_CHANNEL);

    if(RT_EOK != ret)
    {
        return 0;
    }


    for(i = 0; i < 100; i++)
    {
        /* 读取采样值 */
//        value = rt_adc_read(pcp_dev, CP_ADC_CHANNEL);
        value0 = adc_channel_sample(CP_ADC_CHANNEL);
        if(value0 < CP_ADC_MIN_CONVERT_V)
        {
            continue;
        }

        value1 = adc_channel_sample(CP_ADC_CHANNEL);
        if(value1 < CP_ADC_MIN_CONVERT_V)
        {
            continue;
        }

        uiDValue = value0 >= value1 ? value0 - value1  : value1 - value0  ;
        if(uiDValue > CP_ADC_DIFF)
        {
            continue;
        }
        value     = (value0 + value1) / 2 ;

        uiaBuf[n] = value * CP_ADC_REFER_VOLT / CP_ADC_CONVERT_BITS;
        uiSum += uiaBuf[n];

        if(++n == 4)
        {
//			rt_adc_disable(pcp_dev,  CP_ADC_CHANNEL);
            //return (uiSum / 8);
            uiVaule = uiSum / 4;
            *puiCpVolt = uiVaule * 4;
            return 0;
        }
    }

//	rt_adc_disable(pcp_dev, CP_ADC_CHANNEL);
    return 0;
}
