/*
 * @Author: your name
 * @Date: 2020-06-12 14:46:27
 * @LastEditTime: 2020-06-12 15:18:45
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\bsp\ring_buffer.h
 */ 
/**
* @file ring_buffer.h
* @brief ring buffer
* @author rgw
* @version V1.0
* @date 2016-11-02
*/
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include "ucos_ii.h"

typedef struct __ring_buffer
{
    uint8_t *buffer;    //缓冲区
    uint32_t size;      //大小
    uint32_t in;        //入口位置
    uint32_t out;       //出口位置
    OS_EVENT *lock;   //互斥锁
} ring_buffer_s;

uint32_t ring_buffer_get(ring_buffer_s *rb, uint8_t *buffer, uint32_t len);
uint32_t ring_buffer_put(ring_buffer_s *rb, uint8_t *buffer, uint32_t len);
uint32_t ring_buffer_len(const ring_buffer_s *rb);
void ring_buffer_free(ring_buffer_s *rb);
ring_buffer_s *ring_uint8_init(uint32_t size);

#endif
