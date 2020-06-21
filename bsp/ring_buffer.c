/*
 * @Author: your name
 * @Date: 2020-06-12 14:46:27
 * @LastEditTime: 2020-06-12 15:26:31
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\bsp\ring_buffer.c
 */ 
#include "ring_buffer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib_mem.h"

//判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))
//取a和b中最小值
#define rb_min(a, b) (((a) < (b)) ? (a) : (b))

//初始化缓冲区
ring_buffer_s *__ring_buffer_init(uint8_t *buffer, uint32_t size)
{
    INT8U err;
    ring_buffer_s *rb = NULL;
    if (!is_power_of_2(size))
    {
        return rb;
    }
    rb = (ring_buffer_s *)malloc(sizeof(ring_buffer_s));
    if (!rb)
    {
        return rb;
    }
    Mem_Set(rb, 0, sizeof(ring_buffer_s));
    rb->buffer = buffer;
    rb->size = size;
    rb->in = 0;
    rb->out = 0;
    rb->lock = OSMutexCreate(0,&err);  //创建互斥信号量
    return rb;
}

ring_buffer_s *ring_uint8_init(uint32_t size)
{
    uint8_t *buffer;
    buffer = (uint8_t *)malloc(size);
    if (buffer == NULL)
    {
        return NULL;
    }
    Mem_Set(buffer, 0, size);
    return __ring_buffer_init(buffer, size);
}

/*释放缓冲区*/
void ring_buffer_free(ring_buffer_s *rb)
{
    if (rb)
    {
        if (rb->buffer)
        {
            free(rb->buffer);
            rb->buffer = NULL;
        }
        free(rb);
        rb = NULL;
    }
}

/*缓冲区的长度*/
uint32_t ring_buffer_len(const ring_buffer_s *rb)
{
    return (rb->in - rb->out);
}

/*从缓冲区中取数据*/
uint32_t ring_buffer_get(ring_buffer_s *rb, uint8_t *buffer, uint32_t len)
{
    uint32_t l = 0;
    len = rb_min(len, rb->in - rb->out);
    /* first get the data from fifo->out until the end of the buffer */
    l = rb_min(len, rb->size - (rb->out & (rb->size - 1)));
    Mem_Copy(buffer, rb->buffer + (rb->out & (rb->size - 1)), l);
    /* then get the rest (if any) from the beginning of the buffer */
    Mem_Copy(buffer + l, rb->buffer, len - l);
    rb->out += len; //每次累加，到达最大值后溢出，自动转为0
    return len;
}

/*向缓冲区中存放数据*/
uint32_t ring_buffer_put(ring_buffer_s *rb, uint8_t *buffer, uint32_t len)
{
    uint32_t l = 0;
    len = rb_min(len, rb->size - rb->in + rb->out);
    /* first put the data starting from fifo->in to buffer end */
    l = rb_min(len, rb->size - (rb->in & (rb->size - 1)));
    Mem_Copy(rb->buffer + (rb->in & (rb->size - 1)), buffer, l);
    /* then put the rest (if any) at the beginning of the buffer */
    Mem_Copy(rb->buffer, buffer + l, len - l);
    rb->in += len; //每次累加，到达最大值后溢出，自动转为0
    return len;
}
