/*
 * @Author: your name
 * @Date: 2020-06-07 11:16:22
 * @LastEditTime: 2020-06-12 14:30:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f767-uscoii\App\app_main.c
 */
#include "app_main.h"
#include "debug.h"
#include "gpio.h"

/**
 * @description: ucosii的优先级必须唯一，后面可以靠这个去删除任务和挂起任务操作
 * @param {type} 
 * @return: 
 */
#define LED_STAKE_SIZE  128
OS_STK led_blink_stake[LED_STAKE_SIZE];
#define LED_TASK_PRO        1

#define UART_STAKE_SIZE  128
OS_STK uart_stake[UART_STAKE_SIZE];
#define UART_TASK_PRO       2

void ucos_show_version(void)
{
    ucos_kprintf("\n \\ |  /\n");
    ucos_kprintf("- UCOS -     Ucosii Operating System\n");
    ucos_kprintf(" / |  \\     Version:V%d.%d.%d build %s\n",
               OS_VERSION/10000,OS_VERSION%10000/100,OS_VERSION%100, __DATE__);
    ucos_kprintf(" i - n Copyright by Micrium RTOS.\n");
}

void led_blink_task(void *arg);
void uart_task(void *arg);
void create_app_task(void)
{
    ucos_show_version();
    
    ucos_kprintf ("create ucosii app task.\r\n");
    OSTaskCreate(led_blink_task, (void *)0, (OS_STK *)&led_blink_stake[LED_STAKE_SIZE - 1], LED_TASK_PRO);
    OSTaskCreate(uart_task, (void *)0, (OS_STK *)&uart_stake[UART_STAKE_SIZE - 1], UART_TASK_PRO);
}

void led_blink_task(void *arg)
{
    for (;;)
    {
        /* code */
        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
        OSTimeDly(100);             /*正常的延时函数*/
    }
}

void uart_task(void *arg)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
        /* code */
        OSTimeDlyHMSM(0,0,0,500);   /*按照时分秒的方式进行延时*/
    }
}


