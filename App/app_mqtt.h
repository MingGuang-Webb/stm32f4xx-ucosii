/*
 * @Author: your name
 * @Date: 2020-06-15 16:08:50
 * @LastEditTime: 2020-06-15 16:34:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\App\app_mqtt.h
 */ 
#ifndef _APP_MQTT_H
#define _APP_MQTT_H

#include "mqtt/mqtt_client.h"
#include "debug.h"
#include "hash/md5.h"

//MQTT server name
#define APP_SERVER_NAME "test.mosquitto.org"
#define TEST_SERVER "iot-as-mqtt.cn-shanghai.aliyuncs.com"
//MQTT server port
#define APP_SERVER_PORT 1883   //MQTT over TCP

#define EXAMPLE_PRODUCT_KEY         "a17CjwhpBtx"
#define EXAMPLE_PRODUCT_SECRET      "tuHhoQD8doiOaCPf"					
#define EXAMPLE_DEVICE_NAME         "GPSTracker_MC20"
#define EXAMPLE_DEVICE_SECRET       "xmgsXcOicHYSkKM91mhr7byApfLrAoJT"
#define EXAMPLE_FIRMWARE_VERSION    "app-1.0.0-20190722.1000"

typedef enum _MQTT_NET
{
    TBOX_MQTT_STATE_IDLE,
    TBOX_MQTT_STATE_INIT,
    TBOX_MQTT_STATE_OPEN,
    TBOX_MQTT_STATE_CONNECT,
    TBOX_MQTT_STATE_SUB,
    TBOX_MQTT_STATE_ALIVE,
    TBOX_MQTT_STATE_DISCONNECT,
    TBOX_MQTT_STATE_CLOSE
}mqtt_net_sta_t;

int mqtt_net_fsm(uint8_t mqtt_net_sta);
void connect_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, int t);
void connect_aliyun_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, int t);
error_t publish_info_to_aliyun(MqttClientContext *context);
error_t mqttConnect(void);
#endif
