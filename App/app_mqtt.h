/*
 * @Author: your name
 * @Date: 2020-06-15 16:08:50
 * @LastEditTime: 2020-06-18 10:59:11
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
#define APP_SERVER_PORT             1883   //MQTT over TCP
#if 0
#define EXAMPLE_PRODUCT_KEY         "a1Q1NXWxdXt"
#define EXAMPLE_PRODUCT_SECRET      "1kmFBgm8ougDhg5o"					
#define EXAMPLE_DEVICE_NAME         "CycloneTCP-MQTT-DEMO"
#define EXAMPLE_DEVICE_SECRET       "PKOhorJ6NfH4w1RtV8TBxcDx5Kps23GV"
#define EXAMPLE_FIRMWARE_VERSION    "app-1.0.0-20190722.1000"
#endif
#define EXAMPLE_PRODUCT_KEY         "a1pWJ58Ync9"
#define EXAMPLE_PRODUCT_SECRET      "w7XxIIyoqckeEAg7"					
#define EXAMPLE_DEVICE_NAME         "QL-YS1A19100012"
#define EXAMPLE_DEVICE_SECRET       "Zq5YXzbabaBMp5PJscg1ZR3RM1kOzrC1"
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

int mqtt_net_fsm(void);
void connect_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, uint32_t t);
void connect_aliyun_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, uint32_t t);
error_t publish_info_to_aliyun(MqttClientContext *context);
error_t mqttConnect(void);
#endif
