/**
 ****************************************************************************************************
 * @file        delay.h
 * @version     V1.0
 * @date        2020-04-17
 * @brief       使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
 *              提供delay_init初始化函数， delay_us和delay_ms等延时函数
 ****************************************************************************************************
 ****************************************************************************************************
 */
 
#ifndef __DELAY_H
#define __DELAY_H

#include "../sys/sys.h"


void delay_init(uint16_t sysclk);       /* 初始化延迟函数 */
void delay_ms(uint16_t nms);            /* 延时nms */
void delay_us(uint32_t nus);            /* 延时nus */

#if (!SYS_SUPPORT_OS)                   /* 如果不支持OS */
    void HAL_Delay(uint32_t Delay);     /* HAL库的延时函数，HAL库内部用到 */
#endif

#endif





























