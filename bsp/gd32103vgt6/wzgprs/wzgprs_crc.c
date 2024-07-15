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
 #include <stdint.h>
 
/**
 * @brief  万桩GPRS模块crc-8校验
 * @param[in] ptr: 指向输入的数据 
 *            len: 数据长度
 * @param[out]   
 * @return 计算号的 crc-8 结果
 * @note 
 */
uint8_t wzgprs_crc8_MAXIM(uint8_t *ptr, uint16_t len)
{
    uint8_t retCRCValue=0x00;
    uint8_t *pData;
    uint16_t i=0;
    uint8_t pDataBuf=0;

    pData=(uint8_t *)ptr;

    while(len--)
    {
        pDataBuf=*pData++;
        for(i=0; i<8; i++)
        {
            if((retCRCValue^(pDataBuf))&0x01)
            {
                retCRCValue^=0x18;
                retCRCValue>>=1;
                retCRCValue|=0x80;
            }
            else
            {
                retCRCValue>>=1;
            }
            pDataBuf>>=1;
        }
    }
    return retCRCValue;
}
/**
 * @brief 万桩GPRS模块crc-16校验
 * @param[in] ptr: 指向输入的数据 
 *            len: 数据长度
 * @param[out]   
 * @return 计算号的 crc-16 结果
 * @note 
 */
uint16_t wzgprs_crc16(uint8_t *ptr, uint16_t len)
{
    uint16_t i, j, tmp, CRC16;
    CRC16 = 0xffff;

    for(i = 0; i < len; i++)
    {
        CRC16 = *ptr ^ CRC16;

        for(j = 0; j < 8; j++)
        {
            tmp = CRC16 & 0x0001;
            CRC16 = CRC16 >> 1;
            if(tmp)
            {
                CRC16 = CRC16 ^ 0xa001;
            }
        }
        ptr++;
    }
    return (CRC16);
}
