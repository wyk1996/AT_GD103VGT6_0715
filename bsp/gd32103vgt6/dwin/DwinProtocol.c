/*******************************************************************************
 *          Copyright (c) 2020-2050,  Co., Ltd.
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
#include "DwinProtocol.h"
#include <rtthread.h>
#include "dwin_com_pro.h"
#include "stddef.h"

static _LCD_SEND_CONTROL LCD_SendControl;

/**
 * @brief 清除LCD发送BUF长度
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void LCD_SendBufLenClear(void)
{
    LCD_SendControl.Len = 0;
}

/**
 * @brief 往LCD缓冲区写入一个字节
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void LCD_SendByteToBuf(uint8_t data) //向串口发送一个字节
{
    LCD_SendControl.Buf[LCD_SendControl.Len++] = data;
    if (LCD_SendControl.Len >= LCD_SEND_BUF_LEN)
    {
        LCD_SendControl.Len = 0;
    }
}

/**
 * @brief 往LCD缓冲区写入一个半字，低字节先写入，高字节后写入
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void LCD_SendTwoBytes(uint16_t data) //向串口发送一个字
{
    LCD_SendByteToBuf( ( (data >> 8) & 0xFF) );
    LCD_SendByteToBuf( (data & 0xFF) );
}

/**
 * @brief 往LCD缓冲区写入一个字，低字节先写入，高字节后写入
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void LCD_SendFourBytes(uint32_t data) //向串口发送一个字
{
    LCD_SendTwoBytes( ( (data >> 16) & 0xFFFF) );
    LCD_SendTwoBytes( (data & 0xFFFF) );
}

/**
 * @brief LCD发送buf数据写入串口
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t LCD_SendToUart(void)
{
    dwin.write(LCD_SendControl.Buf, LCD_SendControl.Len);
    LCD_SendBufLenClear();
    rt_thread_mdelay(20);
    return LCD_SendControl.Len;
}

/**
 * @brief 迪文屏通信帧头
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void Dwin_FrameHead(void) //发送帧开始0x5a 0xa5
{
    LCD_SendTwoBytes(DWIN_LCD_HEAD);
}

/**
 * @brief 显示类型为INT32的数字变量
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintNum32uVariable(uint16_t VariableAddress,uint32_t num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(VariableAddress);				             //地址

    LCD_SendFourBytes(num);						                 //数据

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示类型为INT16的数字变量
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintNum16uVariable(uint16_t VariableAddress,uint16_t num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(VariableAddress);				             //地址

    LCD_SendTwoBytes(num); 						                 //数据

    LCD_SendToUart();         									 //开始发送
}


//LCD向串口发送模拟点击:Xaxis=x轴坐标  Yaxis=Y轴坐标
void simulation_Print(uint32_t Xaxis,uint32_t Yaxis)
{
    //5A A5 0B 82 00 D4 5A A5 00 04 00 64 00 96(x,y范围值要大一点)
    //举例：5AA5 0B 82 00D4 5AA5 0004 00EE 008F
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    LCD_SendByteToBuf(0x0B); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);
    LCD_SendTwoBytes(0x00D4);
    LCD_SendTwoBytes(DWIN_RESET_CMD);
    LCD_SendTwoBytes(0x0004);
    LCD_SendTwoBytes(Xaxis);
    LCD_SendTwoBytes(Yaxis);
    LCD_SendToUart();         		 //开始发送
}



/**
 * @brief 显示类型为uint8_t的数字变量
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintNum8uVariable(uint16_t VariableAddress,uint8_t num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(VariableAddress);				             //地址

    LCD_SendByteToBuf(num); 						             //数据

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示2位小数变量(0-99999.99)
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintDecimalVariable(uint16_t VariableAddress,uint32_t num)
{
    if((num < 0) || (num >100000))
    {
        return;
    }
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
    LCD_SendByteToBuf(sizeof(uint32_t) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）

    LCD_SendTwoBytes(VariableAddress);			                 //地址

    LCD_SendFourBytes((uint32_t)(num*100));						 //发送的数据为实际数据的100倍

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示图标变量
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintIcon(uint16_t VariableAddress,uint16_t num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(VariableAddress);			                 //地址

    LCD_SendTwoBytes(num); 							             //图标位置

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示时间变量
 * @param[in] VariableAddress  变量地址 gRTC	RTC数据
 * @param[out]
 * @return
 * @note
 */
void PrintTime(uint16_t VariableAddress,void * gRTC)
{
//    Dwin_FrameHead();        									 //帧头0x5A 0xA5
//
//    LCD_SendByteToBuf(1 + sizeof(VariableAddress) + 12);          //数据长度=指令（1byte）+地址长度+变量个数
//    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

//	LCD_SendTwoBytes(VariableAddress);				             //地址
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Year) + 2000);						         //年
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Month));						         //月
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Day));						         //日
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Hour));						         //时
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Minute));						         //分
//	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Second));						         //秒
//
//    LCD_SendToUart();         									 //开始发送
//    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
//	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/**
 * @brief 显示字符串
 * @param[in] VariableAddress 变量地址
 *            num 显示数据
 * @param[out]
 * @return
 * @note
 */
void PrintStr(uint16_t VariableAddress,uint8_t *s,uint8_t len)
{
    uint8_t i = 0;

    if((s == NULL) || !len)
    {
        return;
    }

    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1+2); 		 //数据长度=地址长度+变量长度+指令（1byte）+覆盖指令（2byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
    LCD_SendTwoBytes(VariableAddress);			                 //地址

    for (i = 0; i < len; i++)
    {
        LCD_SendByteToBuf(s[i]);
    }
    LCD_SendTwoBytes(DWIN_LCD_COVER);                           //覆盖上次显示内容，只显示本次内容长度数据
    //设置串口发送忙
    LCD_SendToUart();         									//开始发送
}

/**
 * @brief 显示MAC地址
 * @param[in] VariableAddress  变量地址 s	字符串内容
 * @param[out]
 * @return
 * @note
 */
void PrintMAC(uint16_t VariableAddress,uint8_t *s,uint8_t len)
{
    uint8_t i = 0;

    if((s == NULL) || !len)
    {
        return;
    }

    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1); 		 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
    LCD_SendTwoBytes(VariableAddress);			                 //地址

    for (i = 0; i < len; i++)
    {
        LCD_SendByteToBuf(s[i]);
    }

    LCD_SendToUart();         									//开始发送
}

/**
 * @brief 清空某一地址段数据
 * @param[in] VariableAddress 变量地址
 *            len 地址长度
 * @param[out]
 * @return
 * @note
 */
void CleanSomePlace(uint16_t VariableAddress,uint8_t len)
{
    uint8_t i;

    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(VariableAddress);				             //地址

    for(i = 0; i<len; i++)
    {
        LCD_SendByteToBuf(0); 						             //数据
    }

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示二维码
 * @param[in] VariableAddress 变量地址
 *            str ASCII地址
 *            len ASCII长度
 * @param[out]
 * @return
 * @note
 */
void DisplayQRCode(uint16_t VariableAddress, uint8_t *str, uint8_t len)
{
    uint8_t i;

    if ( (str == NULL) || (len == 0) )
    {
        return;
    }

    Dwin_FrameHead();        									//帧头0x5A 0xA5
    LCD_SendByteToBuf(sizeof(VariableAddress) + len +1);        //数据长度
    LCD_SendByteToBuf(VARIABLE_WRITE);   						//写变量存储器指令
    LCD_SendTwoBytes(VariableAddress);			                //地址
    for (i = 0; i < len; i ++)
    {
        LCD_SendByteToBuf(str[i]);                              //数据内容
    }
    LCD_SendByteToBuf(0xff);
    LCD_SendByteToBuf(0xff);
    LCD_SendToUart();         									//开始发送
}

/**
 * @brief 键控函数
 * @param[in] keyval 键控值
 * @param[out]
 * @return
 * @note
 */
void KeyControl(uint8_t keyval)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(3u);                                       //数据长度
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写变量存储器指令

    LCD_SendByteToBuf(DWIN_KEY_CONTROL_REGISTER);				 //地址

    LCD_SendByteToBuf(keyval); 						             //键值

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 读迪文屏输入状态
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ReadDwinInputStatus(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(3u);                                       //指令数据长度
    LCD_SendByteToBuf(REGISTER_READ);                            //读寄存器

    LCD_SendByteToBuf(DWIN_INPUT_STATUS_REGISTER);   			 //寄存器地址

    LCD_SendByteToBuf(1u);                                       //数据长度

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 读迪文屏背光亮度值
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ReadLcdBackLight(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(3u);                                       //指令数据长度
    LCD_SendByteToBuf(REGISTER_READ);                            //读寄存器

    LCD_SendByteToBuf(DWIN_LED_STA_REGISTER);   			     //寄存器地址

    LCD_SendByteToBuf(1u);                                       //数据长度

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示屏复位
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void LCD_Reset(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(4u);                                       //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写寄存器

    LCD_SendByteToBuf(DWIN_RESET_REGISTER);				         //地址

    LCD_SendTwoBytes(DWIN_RESET_CMD); 						     //复位指令

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 将uint8_t的HEX转换为对应的BCD码
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t HEXtoBCD(uint8_t value)
{
    uint8_t  value_bcd;

    if(value > 99)
    {
        return 0;
    }

    value_bcd = ((value/10)<<4) + (value%10);

    return value_bcd;
}

/**
 * @brief 将uint8_t的BCD码转换为对应的HEX
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t BCDtoHEX(uint8_t value)
{
    uint8_t  value_bcd;

    if(value > 0X99)
    {
        return 0;
    }

    value_bcd = (value>>4)*10 + (value&0X0F);

    return value_bcd;
}


/*****************************************************************************
* Function     : CmpNBuf
* Description  : 比较换成是否一致
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  CmpNBuf(uint8_t*  pbuf1,uint8_t*  pbuf2,uint8_t len)
{
    uint8_t i;
    if((pbuf1 == NULL) || (pbuf2 == NULL) || !len)
    {
        return RT_FALSE;
    }
    for(i = 0; i < len; i++)
    {
        if(pbuf1[i] != pbuf2[i])
        {
            break;
        }
    }
    if(i == len)
    {
        return RT_TRUE;		//全部都一样
    }
    return RT_FALSE;
}

/**
 * @brief 读取变量
 * @param[in] VariableAddress  变量起始地址
 *            len 读取数据长度(字节),每个单位长度内有2个字节数据,
*             如:len == 6,则从变量开始地址连续读取12字节数据
 * @param[out]
 * @return
 * @note
 */
void ReadDwinVariable(uint16_t VariableAddress,uint8_t len)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    LCD_SendByteToBuf(1 + sizeof(VariableAddress) + sizeof(len));//数据长度=指令（1byte）+地址长度+读取数据长度
    LCD_SendByteToBuf(VARIABLE_READ);   						 //读变量存储器指令83
    LCD_SendTwoBytes(VariableAddress);				       //地址
    LCD_SendByteToBuf(len);						             //长度
    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 设置变量颜色
 * @param[in] DescrPointer 描述指针地址 color	字符串颜色
 * @param[out]
 * @return
 * @note
 */
void SetVariColor(uint16_t DescrPointer,_SHOW_COLOR color)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(DescrPointer + 0x03);		                 //0x03是描述指针偏移地址，此处代表颜色

    LCD_SendTwoBytes(color);

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 隐藏变量
 * @param[in] DescrPointer 描述指针地址
 * @param[out]
 * @return
 * @note
 */
void SetVariHide(uint16_t DescrPointer)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(DescrPointer);		                         //描述指针

    LCD_SendTwoBytes(DWIN_HIDE_CMD);                             //隐藏变量

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 取消隐藏变量
 * @param[in] DescrPointer	描述指针地址 VariableAddress	变量实际地址
 * @param[out]
 * @return
 * @note
 */
void SetVariCancelHide(uint16_t DescrPointer,uint16_t VariableAddress)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

    LCD_SendTwoBytes(DescrPointer);		                         //描述指针

    LCD_SendTwoBytes(VariableAddress);                           //取消隐藏变量

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief
 * @param[in] 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
 * @param[out] IMAGE 图片ID
 * @return
 * @note
 */
void PraPerprintBackImage(uint8_t IMAGE)
{
    Dwin_FrameHead();         		    //帧头0x5A 0xA5
    LCD_SendByteToBuf(0x07);  		    //0x07 表示数据的长度，以字节为单位
    LCD_SendByteToBuf(VARIABLE_WRITE);  //写DGUS寄存器

    LCD_SendByteToBuf(0x00);
    LCD_SendByteToBuf(0x84);
    LCD_SendByteToBuf(0x5a);
    LCD_SendByteToBuf(0x01);
    LCD_SendByteToBuf(0x00);  		    //0x03、0x04 存放图片ID寄存器

    LCD_SendByteToBuf(IMAGE); 			//显示图片

    LCD_SendToUart();         			//开始发送
}

/**
 * @brief 读取迪文RTC
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ReadDWRTC(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(3); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_READ);   						 //写变量存储器指令

    LCD_SendByteToBuf(0x02);

    LCD_SendByteToBuf(0x07);

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 设置迪文RTC
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void SetDWRTC(uint8_t * ptime,uint8_t len)
{
    uint8_t i;
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(9); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写变量存储器指令

    for(i = 0; i < len; i++)
    {
        LCD_SendByteToBuf(ptime[i]);
    }

    LCD_SendToUart();         									 //开始发送
}

/**
 * @brief 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void PrintBackImage(uint8_t Image)
{
    PraPerprintBackImage(Image);
}
