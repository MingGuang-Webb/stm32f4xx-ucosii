#include "app_mqtt.h"
#include "stdint.h"
#include "stdio.h"
#include "infra_sha256.h"
#include "hash/sha256.h"
#include "stm32f4xx.h"
#include "mqtt/mqtt_client.h"

MqttClientContext mqttClientContext;
//Global variables
RNG_HandleTypeDef RNG_Handle;

mqtt_net_sta_t mqtt_net_sta = TBOX_MQTT_STATE_IDLE;

uint32_t HexToChar(uint8_t Hex, char *c)
{
    sprintf(c, "%02x", Hex);
    return 0;
}

/** @brief hex[0] = 0xE1, hex[1] = 0xFF,  hex[2] = 0x99  --> "E1FF99000..."
 *
 * @param Hex uint8_t*
 * @param Str uint8_t*
 * @param Hexlen int
 * @return uint32_t
 *
 */
uint32_t HexToStr(uint8_t *Hex, char *Str, int Hexlen)
{
    int i;
    for(i = 0; i < Hexlen; i++)
    {
        HexToChar(Hex[i], &(Str[i * 2]));
    }
    Str[i * 2] = '\0';
    return 0;
}

void connect_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, int t)
{
    char buf[100] = {0};
    char pass[16] = {0};
    char pass_str[33] = {0};

    memset(buf, 0, 100);
    memset(pass, 0, 16);
    memset(pass_str, 0, 33);
    sprintf(connect_data->settings.clientId, "%s|s=%s&t=%d&v=%d|", device_name, "md5", t, 1);
    sprintf(connect_data->settings.username, "%s&%s", device_name, product_key);
    sprintf(buf, "deviceName=%s&deviceSecret=%s&productKey=%s&t=%d&v=%d", device_name, device_secret, product_key, t, 1);
    TRACE_INFO("buf:%s\r\n",buf);
    md5Compute((unsigned char *)buf, strlen(buf), (uint8_t *)pass);
    HexToStr((uint8_t *)pass, pass_str, 16);
    strncpy(connect_data->settings.password, pass_str, strlen(pass_str));
}

void connect_aliyun_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, int t)
{
    char buf[100] = {0};
    char pass[32] = {0};
    char pass_str[33] = {0};   
    char clientid[32 + 1] = {0};
    
    sprintf(clientid,"%s.%s",product_key,device_name);
    TRACE_INFO("\r\n\r\n*************START***************\r\n\r\n");
    sprintf(connect_data->settings.clientId,"%s|securemode=3,signmethod=hmacsha256,timestamp=%d|",clientid,t);      //clientID
    TRACE_INFO("clientId:%s\r\n",connect_data->settings.clientId);
    sprintf(connect_data->settings.username,"%s&%s",device_name,product_key);                                       //usernanme
    TRACE_INFO("username:%s\r\n",connect_data->settings.username);                                          
    sprintf(buf,"clientId%sdeviceName%sproductKey%stimestamp%d",clientid,device_name,product_key,t);
    TRACE_INFO("buf:%s\r\n",buf);
    TRACE_INFO("\r\n\r\n**************END**************\r\n\r\n");
    
    utils_hmac_sha256((uint8_t *)buf, strlen(buf), (uint8_t *)device_secret,
                      strlen(device_secret), (uint8_t *)pass);   
    
    HexToStr((uint8_t *)pass, pass_str, 32u);                                                                      
    TRACE_INFO("pass_str:%s\r\n",pass_str);
    strcpy(connect_data->settings.password,pass_str);
}

error_t publish_info_to_aliyun(MqttClientContext *context)
{
    error_t error;
    char buffer[128 + 1] = {0};
    char topic[64 + 1] = {0};
    char *fmt = "/%s/%s/user/GPS";
    char *pload = "{\"G\":{\"G1\":%f,\"G2\":%f,\"G3\":%f,\"G4\":%d},\"method\":\"user.GPS\"}";
    sprintf(topic,fmt,EXAMPLE_PRODUCT_KEY,EXAMPLE_DEVICE_NAME);
    sprintf(buffer,pload,1.0f,2.0f,3.0f,40);
    
    error = mqttClientPublish(context, topic,
        buffer, strlen(buffer), MQTT_QOS_LEVEL_0, TRUE, NULL);
    return error;
}

int mqtt_net_fsm(uint8_t mqtt_net_sta)
{
    switch(mqtt_net_sta)
    {
        case TBOX_MQTT_STATE_IDLE:
        case TBOX_MQTT_STATE_INIT:
            
            break;
        case TBOX_MQTT_STATE_OPEN:
            break;
        case TBOX_MQTT_STATE_CONNECT:
            break;
        case TBOX_MQTT_STATE_SUB:
            break;
        case TBOX_MQTT_STATE_ALIVE:
            break;
        case TBOX_MQTT_STATE_CLOSE:
            break;
        default:
            break;
    }
    return 0;
}

/**
 * @brief Publish callback function
 * @param[in] context Pointer to the MQTT client context
 * @param[in] topic Topic name
 * @param[in] message Message payload
 * @param[in] length Length of the message payload
 * @param[in] dup Duplicate delivery of the PUBLISH packet
 * @param[in] qos QoS level used to publish the message
 * @param[in] retain This flag specifies if the message is to be retained
 * @param[in] packetId Packet identifier
 **/

void mqttTestPublishCallback(MqttClientContext *context,
   const char_t *topic, const uint8_t *message, size_t length,
   bool_t dup, MqttQosLevel qos, bool_t retain, uint16_t packetId)
{
   //Debug message
   TRACE_INFO("PUBLISH packet received...\r\n");
   TRACE_INFO("  Dup: %u\r\n", dup);
   TRACE_INFO("  QoS: %u\r\n", qos);
   TRACE_INFO("  Retain: %u\r\n", retain);
   TRACE_INFO("  Packet Identifier: %u\r\n", packetId);
   TRACE_INFO("  Topic: %s\r\n", topic);
   TRACE_INFO("  Message (%" PRIuSIZE " bytes):\r\n", length);
   TRACE_INFO_ARRAY("    ", message, length);
   TRACE_INFO("  Mqtt Event: %d\r\n", context->packetType);
   //Check topic name
   if(!strcmp(topic, "board/leds/1"))
   {

   }
   else if(!strcmp(topic, "board/leds/2"))
   {
       
   }
   else if (!strcmp(topic, "board/leds/3")) /*测试topic，服务器像设备推送消息server publish*/
   {
       TRACE_INFO("server publish message to device.\r\n");
   }
}

/**
 * @brief Establish MQTT connection
 **/

error_t mqttConnect(void)
{
   error_t error;
   IpAddr ipAddr;
   uint32_t trng = 0;
   char server_url[128 + 1] = {0};
   
   //Debug message
   TRACE_INFO("\r\n\r\nResolving server name...\r\n");

   //Resolve MQTT server name
   sprintf(server_url,"%s.%s",EXAMPLE_PRODUCT_KEY,TEST_SERVER);
   error = getHostByName(NULL, server_url, &ipAddr, 0);
   //Any error to report?
   if(error)
      return error;

#if (APP_SERVER_PORT == 8080 || APP_SERVER_PORT == 8081)
   //Register RNG callback
   webSocketRegisterRandCallback(webSocketRngCallback);
#endif

   //Set the MQTT version to be used
   mqttClientSetVersion(&mqttClientContext, MQTT_VERSION_3_1_1);

#if (APP_SERVER_PORT == 1883)
   //MQTT over TCP
   mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TCP);
#elif (APP_SERVER_PORT == TEST_PORT || APP_SERVER_PORT == 8884)
   //MQTT over TLS
   mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_TLS);
   //Register TLS initialization callback
   mqttClientRegisterTlsInitCallback(&mqttClientContext, mqttTestTlsInitCallback);
#elif (APP_SERVER_PORT == 8080)
   //MQTT over WebSocket
   mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_WS);
#elif (APP_SERVER_PORT == 8081)
   //MQTT over secure WebSocket
   mqttClientSetTransportProtocol(&mqttClientContext, MQTT_TRANSPORT_PROTOCOL_WSS);
   //Register TLS initialization callback
   mqttClientRegisterTlsInitCallback(&mqttClientContext, mqttTestTlsInitCallback);
#endif

   //Register publish callback function
   mqttClientRegisterPublishCallback(&mqttClientContext, mqttTestPublishCallback);

   //Set communication timeout
   mqttClientSetTimeout(&mqttClientContext, 20000);
   //Set keep-alive value
   mqttClientSetKeepAlive(&mqttClientContext, 100);

#if (APP_SERVER_PORT == 8080 || APP_SERVER_PORT == 8081)
   //Set the hostname of the resource being requested
   mqttClientSetHost(&mqttClientContext, APP_SERVER_NAME);
   //Set the name of the resource being requested
   mqttClientSetUri(&mqttClientContext, APP_SERVER_URI);
#endif

   //Set client identifier
   //mqttClientSetIdentifier(&mqttClientContext, "client12345678");

   //Set user name and password
   //mqttClientSetAuthInfo(&mqttClientContext, "username", "password");

   //Set Will message
//   mqttClientSetWillMessage(&mqttClientContext, "board/status",
//      "offline", 7, MQTT_QOS_LEVEL_0, FALSE);

   //Debug message
   TRACE_INFO("Connecting to MQTT server %s...\r\n", ipAddrToString(&ipAddr, NULL));
   
   HAL_RNG_GenerateRandomNumber(&RNG_Handle, &trng);
   trng %= 10000;
   TRACE_INFO("ali mqtt RNG:%d\r\n",trng);
   connect_aliyun_data_init(&mqttClientContext,EXAMPLE_DEVICE_NAME,EXAMPLE_DEVICE_SECRET,EXAMPLE_PRODUCT_KEY,trng);
   //Start of exception handling block
   do
   {
      //Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext,
         &ipAddr, APP_SERVER_PORT, TRUE);
      //Any error to report?
      if(error)
         break;
       
//      sprintf(topic_name, "/sys/df/%s/%s/response", TEST_PRODUCT_KEY, TEST_DEVICE_NAME);
//      //Subscribe to the desired topics
//      error = mqttClientSubscribe(&mqttClientContext,
//         topic_name, MQTT_QOS_LEVEL_1, NULL);
      //Any error to report?
      if(error)
         break;
      //End of exception handling block
   } while(0);

   //Check status code
   if(error)
   {
      //Close connection
      mqttClientClose(&mqttClientContext);
   }

   //Return status code
   return error;
}
