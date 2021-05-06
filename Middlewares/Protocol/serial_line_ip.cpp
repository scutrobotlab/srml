/**
  ******************************************************************************
  * Copyright (c) 2019 - ~, SCUT-RobotLab Development Team
  * @file    serial_line_ip.cpp
  * @author  BigeYoung  SCUT.BigeYoung@gmail.com
  * @brief   SerialLineIP is a simple serial protocol for Data Link Layer.This file
  *          provide functions for framing and transparent transmission. 
  * @date    2018-12-24
  * @version 1.0
  * @see     
  *		       [RFC-1055: SLIP 协议文档](https://tools.ietf.org/html/rfc1055)
  * @warning
  *		        Standard C++03 required!
  *
  ******************************************************************************
  * @attention
  * 
  * if you had modified this file, please make sure your code does not have any 
  * bugs, update the version Number, write dowm your name and the date. The most
  * important thing is make sure the users will have clear and definite under-
  * standing through your new brief.
  *
  * <h2><center>&copy; Copyright (c) 2019 - ~, SCUT-RobotLab Development Team.
  * All rights reserved.</center></h2>
  ******************************************************************************
  */

#include "srml.h"

#if USE_SRML_SERIAL_LINE_IP

/* Includes ------------------------------------------------------------------*/ 
#include "serial_line_ip.h"

/* Private define ------------------------------------------------------------*/
using namespace SerialLineIP;
/* Private variables ---------------------------------------------------------*/
/* Private type --------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
/**
 * @brief Serial Line IP PACK
 * @param p_PDU 起始位置指针
 * @param PDU_len PDU 字节长度
 * @return std::vector<uint8_t> SDU
 * @note Service Data Unit (SDU) 指本层封包后产生的数据单元 \n
 *       Protocol Data Unit (PDU) 指上层协议数据单元
 */
std::vector<uint8_t> SerialLineIP::Pack(const void *const p_PDU, int PDU_len)
{
    uint8_t *p_PDU_data = (uint8_t *)p_PDU;
    /* 计算所需内存空间
     */
    int SDU_len = PDU_len + 2; //添加首尾2个END
    for (uint8_t *p = p_PDU_data; p < p_PDU_data + PDU_len; p++)
    {
        if ((*p == END) || (*p == ESC))
        {
            SDU_len++;
        }
    }

    std::vector<uint8_t> SDU_data;
    SDU_data.reserve(SDU_len);

    /* send an initial END character to flush out any data that may
     * have accumulated in the receiver due to line noise
     */
    SDU_data.push_back(END);

    for (uint8_t *p = p_PDU_data; p < p_PDU_data + PDU_len; p++)
    {
        switch (*p)
        {
        /* if it's the same code as an END character, we send a
         * special two character code so as not to make the
         * receiver think we sent an END
         */
        case END:
            SDU_data.push_back(ESC);
            SDU_data.push_back(ESC_END);
            break;

        /* if it's the same code as an ESC character,
         * we send a special two character code so as not
         * to make the receiver think we sent an ESC
         */
        case ESC:
            SDU_data.push_back(ESC);
            SDU_data.push_back(ESC_ESC);
            break;

        /* otherwise, we just send the character
         */
        default:
            SDU_data.push_back(*p);
        }
    }

    /* tell the receiver that we're done sending the packet
     */
    SDU_data.push_back(END);

    /* 确认计算所需内存空间与实际内存空间一致
     */
//		assert(SDU_data.size() == SDU_len);
//    if(SDU_data.size() == SDU_len)
		return SDU_data;

}

/**
 * @brief Serial Line IP UNPACK
 * @param p_SDU 起始位置指针
 * @param SDU_len SDU 字节长度
 * @return std::vector<uint8_t> PDU
 * @note Service Data Unit (SDU) 指本层解包前的数据单元 \n
 *       Protocol Data Unit (PDU) 指上层协议数据单元
 */
std::vector<uint8_t> SerialLineIP::Unpack(const void *const p_SDU, int SDU_len)
{
    std::vector<uint8_t> PDU_data;
    PDU_data.reserve(SDU_len - 2); //分配足够多的内存空间，减去首位两个END

    bool begin = false;

    uint8_t *p_SDU_data = (uint8_t *)p_SDU;
    for (uint8_t *p = p_SDU_data; p < p_SDU_data + SDU_len; p++)
    {
        /* skip until first END character found
         */
        if (!begin)
        {
            if (*p != END)
                continue;
            else
                begin = true;
        }

        /* handle bytestuffing if necessary
         */
        switch (*p)
        {
        /* if it's an END character then we're done with
         * the packet
         */
        case END:
            /* a minor optimization: if there is no
             * data in the packet, ignore it. This is
             * meant to avoid bothering IP with all
             * the empty packets generated by the
             * duplicate END characters which are in
             * turn sent to try to detect line noise.
             */
            if (PDU_data.size() > 0)
                return PDU_data;
            else
                break;

        /* if it's the same code as an ESC character, wait
         * and get another character and then figure out
         * what to store in the packet based on that.
         */
        case ESC:
            p++;

            /* if "p" is not one of these two, then we
             * have a protocol violation.  The best bet
             * seems to be to leave the byte alone and
             * just stuff it into the packet
             */
            switch (*p)
            {
            case ESC_END:
                PDU_data.push_back(END);
                break;
            case ESC_ESC:
                PDU_data.push_back(ESC);
                break;
            default:
            /* ESC 后面跟了其他字符 
             * 这在协议中没有定义，
             * 则认为这一帧出现差错。
             */
                begin = false;
                PDU_data.clear();
            }
            break;

        /* here we fall into the default handler and let
         * it store the character for us
         */
        default:
            PDU_data.push_back(*p);
        }
    }
    /* 如果没有结束'END'标识符，则返回空
     */
    PDU_data.clear();
    return PDU_data;
}

#endif /* USE_SRML_SERIAL_LINE_IP */

/************************ COPYRIGHT(C) SCUT-ROBOTLAB **************************/
