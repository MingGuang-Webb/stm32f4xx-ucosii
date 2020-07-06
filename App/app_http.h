/*
 * @Author: your name
 * @Date: 2020-06-23 18:32:51
 * @LastEditTime: 2020-06-25 15:30:28
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\App\app_http.h
 */ 
#ifndef _APP_HTTP_H
#define _APP_HTTP_H

#include "stdint.h"

/*HTTP状态机转换*/
enum __HTTP_STATE
{
    HTTP_INIT_STATE = 0,
    HTTP_DNS_STATE,
    HTTP_SET_STATE,
    HTTP_OPEN_STATE,
    HTTP_CONNECT_STATE,
    HTTP_RECV_RESP_STATE,
    HTTP_READ_STATE,
    HTTP_CLOSE_STATE
};

int http_connect(void);
#endif
