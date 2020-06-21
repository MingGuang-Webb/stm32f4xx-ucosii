/*
 * @Author: your name
 * @Date: 2020-06-08 10:59:02
 * @LastEditTime: 2020-06-09 10:43:10
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f767-uscoii\App\debug.c
 */ 
#include "debug.h"
#include "SEGGER_RTT.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"
#include "ucos_ii.h"
#include <os_cpu.h>
#include "stdio.h"

void remove_none_printable_char(const char *str)
{
    char *s = (char *)str;
    while (*s != '\0')
    {
        if ((*s < 32 || *s > 126) && *s != '\r' && *s != '\n') 
        {
            *s = '?';
        }
        s++;
    }
}

/**
 * @brief rt_kprintf 重定向输出
 * 
 * @param str 
 */
void rt_hw_console_output(const char *str)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();    // Tell uC/OS-II that we are starting an ISR
    remove_none_printable_char(str);
    /* jlink rtt 输出 */
    SEGGER_RTT_WriteString( 0, str );
    OS_EXIT_CRITICAL();
}

/**
 * @brief rt_kprintf 串口输出
 * 
 * @param str 
 */
void ucos_usart_printf(char *buf,int len)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();    // Tell uC/OS-II that we are starting an ISR
    HAL_UART_Transmit(&huart1,(uint8_t *)buf,len,1000u);
    OS_EXIT_CRITICAL();
}

void ucos_kprintf(const char *fmt, ...)
{
    va_list args;
    int length = 0;
    static char rt_log_buf[512];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);  
    length = length;
#ifdef RTT_DEBUG
    rt_hw_console_output(rt_log_buf);
#else
    ucos_usart_printf(rt_log_buf,length);
#endif    
    va_end(args);
}






