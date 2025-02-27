/*****************************************Copyright(C)******************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: Dispkey.h
* Author			:
* Date First Issued	:
* Version			:
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__DISPKEY_H_
#define	__DISPKEY_H_
/* Includes-----------------------------------------------------------------------------------*/
#include <rtthread.h>
#include "stdint.h"

/* Private typedef----------------------------------------------------------------------------*/
//键值
typedef enum
{
    LCD_KEY1 = 1,// 1，固定返回主界面 （在UI界面左下角）
    LCD_KEY2,    // 2, 固定返回上一页/确定 （在UI界面右下角）
    LCD_KEY3,    // 3, 固定下一页  （在UI界面中间）
    LCD_KEY4,    // 4
    LCD_KEY5,    // 5
    LCD_KEY6,    // 6
    LCD_KEY7,    // 7
    LCD_KEY8,    // 8
    LCD_KEY9,    // 9
    LCD_KEY10,   // 10
    LCD_KEY11,   // 11
    LCD_KEY12,   // 12
    LCD_KEY13,   // 13
    LCD_KEY14,   // 14
    LCD_KEY15,   // 15
    LCD_KEY16,   // 16
    LCD_KEY17,   // 17
    LCD_KEY18,   // 18
    LCD_KEY19,   // 19
    LCD_KEY20,   // 20
    LCD_KEY21,   // 21
    LCD_KEY22,   // 22
    LCD_KEY23,   // 23
    LCD_KEY24,   // 24
    LCD_KEY25,   // 25
    LCD_KEY26,   // 26
    LCD_KEY27    // 27
} _LCD_KEYVALUE;

typedef struct st_menu
{
    struct st_menu* Menu_PrePage;                     		//指上一级菜单
    const int FrameID;                                      //当前画面ID,通常为图片ID号 （切换页面）
    void (*function2)(_LCD_KEYVALUE *keyval);               //当前画面的按键处理
    void (*function3)(void);                                //当前画面变量显示函数
    uint16_t CountDown;										//当前页面页面倒计时时间 0表示不需要倒计时
} ST_Menu;

void HYKeyEvent25(_LCD_KEYVALUE *keyval);

void HYKeyEvent1(_LCD_KEYVALUE *keyval);
void HYKeyEvent2(_LCD_KEYVALUE *keyval);
void HYKeyEvent3(_LCD_KEYVALUE *keyval);
void HYKeyEvent4(_LCD_KEYVALUE *keyval);
void HYKeyEvent5(_LCD_KEYVALUE *keyval);
void HYKeyEvent6(_LCD_KEYVALUE *keyval);
void HYKeyEvent7(_LCD_KEYVALUE *keyval);
void HYKeyEvent8(_LCD_KEYVALUE *keyval);
void HYKeyEvent9(_LCD_KEYVALUE *keyval);
void HYKeyEvent10(_LCD_KEYVALUE *keyval);
void HYKeyEvent11(_LCD_KEYVALUE *keyval);
void HYKeyEvent12(_LCD_KEYVALUE *keyval);
void HYKeyEvent13(_LCD_KEYVALUE *keyval);
void HYKeyEvent14(_LCD_KEYVALUE *keyval);
void HYKeyEvent15(_LCD_KEYVALUE *keyval);
void HYKeyEvent16(_LCD_KEYVALUE *keyval);
void HYKeyEvent17(_LCD_KEYVALUE *keyval);
void HYKeyEvent18(_LCD_KEYVALUE *keyval);
void HYKeyEvent19(_LCD_KEYVALUE *keyval);
void HYKeyEvent20(_LCD_KEYVALUE *keyval);
void HYKeyEvent21(_LCD_KEYVALUE *keyval);
void HYKeyEvent22(_LCD_KEYVALUE *keyval);
void HYKeyEvent23(_LCD_KEYVALUE *keyval);
void HYKeyEvent24(_LCD_KEYVALUE *keyval);
void HYKeyEvent26(_LCD_KEYVALUE *keyval);
void HYKeyEvent27(_LCD_KEYVALUE *keyval);
void HYKeyEvent28(_LCD_KEYVALUE *keyval);
void HYKeyEvent30(_LCD_KEYVALUE *keyval);
void HYKeyEvent31(_LCD_KEYVALUE *keyval);
void HYKeyEvent32(_LCD_KEYVALUE *keyval);
void HYKeyEvent33(_LCD_KEYVALUE *keyval);
void HYKeyEvent34(_LCD_KEYVALUE *keyval);
void HYKeyEvent35(_LCD_KEYVALUE *keyval);
void HYKeyEvent36(_LCD_KEYVALUE *keyval);
void HYKeyEvent37(_LCD_KEYVALUE *keyval);

/***********************************************************************************************
* Function		: DealWithKey
* Description	:按键动作
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void DealWithKey(_LCD_KEYVALUE *keyval);
#endif	//__KEY_H_
/************************(C)COPYRIGHT 2018 *****END OF FILE****************************/
