/*
 * @Author: your name
 * @Date: 2020-06-12 10:57:24
 * @LastEditTime: 2020-06-12 10:58:06
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \steeldust_mcu_softwarec:\Users\Administrator\Desktop\UCOSII\ucos_debug\debug.h
 */ 
#ifndef OS_DEBUG_H
#define OS_DEBUG_H

#define RTT_DEBUG
void rt_hw_console_output(const char *str);
void ucos_kprintf(const char *fmt, ...);
#endif


