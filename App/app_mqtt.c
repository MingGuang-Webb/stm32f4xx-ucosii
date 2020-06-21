#include "app_mqtt.h"
#include "stdint.h"
#include "stdio.h"
#include "infra_sha256.h"
#include "hash/sha256.h"
#include "stm32f4xx.h"
#include "mqtt/mqtt_client.h"

//Global variables
extern RNG_HandleTypeDef hrng;
static MqttClientContext mqttClientContext;
static mqtt_net_sta_t mqtt_net_sta = TBOX_MQTT_STATE_IDLE;
static char mqtt_publish_test[64 + 1] = "Ali Alink CycloneTCP MQTT Test Success.";

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
   for (i = 0; i < Hexlen; i++)
   {
      HexToChar(Hex[i], &(Str[i * 2]));
   }
   Str[i * 2] = '\0';
   return 0;
}

void connect_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, uint32_t t)
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
   TRACE_INFO("buf:%s\r\n", buf);
   md5Compute((unsigned char *)buf, strlen(buf), (uint8_t *)pass);
   HexToStr((uint8_t *)pass, pass_str, 16);
   strncpy(connect_data->settings.password, pass_str, strlen(pass_str));
}

void connect_aliyun_data_init(MqttClientContext *connect_data, char *device_name, char *device_secret, char *product_key, uint32_t t)
{
    char buf[100] = {0};
    char pass[32] = {0};
    char pass_str[33] = {0};   
    char clientid[32 + 1] = {0};

   sprintf(clientid, "%s.%s", product_key, device_name);
   TRACE_INFO("\r\n\r\n*************START***************\r\n\r\n");
   sprintf(connect_data->settings.clientId, "%s|securemode=3,signmethod=hmacsha256,timestamp=%ld|", clientid, t); //clientID
   TRACE_INFO("clientId:%s\r\n", connect_data->settings.clientId);
   sprintf(connect_data->settings.username, "%s&%s", device_name, product_key); //usernanme
   TRACE_INFO("username:%s\r\n", connect_data->settings.username);
   sprintf(buf, "clientId%sdeviceName%sproductKey%stimestamp%d", clientid, device_name, product_key, t);
   TRACE_INFO("buf:%s\r\n", buf);
   TRACE_INFO("\r\n\r\n**************END**************\r\n\r\n");

   utils_hmac_sha256((uint8_t *)buf, strlen(buf), (uint8_t *)device_secret,
                     strlen(device_secret), (uint8_t *)pass);

   HexToStr((uint8_t *)pass, pass_str, 32u);
   TRACE_INFO("pass_str:%s\r\n", pass_str);
   strcpy(connect_data->settings.password, pass_str);
}

static error_t mqtt_publish(MqttClientContext *context,char *fmt ,uint8_t *pload,int pload_len,MqttQosLevel qos)
{
   error_t error;
   char topic[64 + 1] = {0};

   sprintf(topic, fmt, EXAMPLE_PRODUCT_KEY, EXAMPLE_DEVICE_NAME);
   error = mqttClientPublish(context, topic,
                             pload, pload_len, MQTT_QOS_LEVEL_0, TRUE, NULL);
   return error;
}

int mqtt_net_fsm(void)
{
   error_t error;
   error_t rc = NO_ERROR;
   static int count = 0;
    
   switch (mqtt_net_sta)
   {
   case TBOX_MQTT_STATE_IDLE:
   case TBOX_MQTT_STATE_INIT:
      //Make sure the link is up
      if (netGetLinkState(&netInterface[0]))
      {
         memset(&mqttClientContext,0x00,sizeof(MqttClientContext));
         //Initialize MQTT client context
         mqttClientInit(&mqttClientContext);
         mqtt_net_sta = TBOX_MQTT_STATE_CONNECT;
      }
      break;
   case TBOX_MQTT_STATE_CONNECT:
      error = mqttConnect();
      if (!error)
      {
         mqtt_net_sta = TBOX_MQTT_STATE_SUB;
      }
      else
      {
         mqtt_net_sta = TBOX_MQTT_STATE_CLOSE;
      }
      break;
   case TBOX_MQTT_STATE_SUB:
      mqtt_net_sta = TBOX_MQTT_STATE_ALIVE;
      break;
   case TBOX_MQTT_STATE_ALIVE:
      count++;
      if (0 == count % 200)
      {
         rc = mqtt_publish(&mqttClientContext, "/%s/%s/user/update", (uint8_t *)mqtt_publish_test, strlen(mqtt_publish_test), MQTT_QOS_LEVEL_0);
         if (rc != NO_ERROR)
         {
            mqtt_net_sta = TBOX_MQTT_STATE_CLOSE;
         }
      }

      break;
   case TBOX_MQTT_STATE_CLOSE:
      mqtt_net_sta = TBOX_MQTT_STATE_IDLE;
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
   if (!strcmp(topic, "board/leds/1"))
   {
   }
   else if (!strcmp(topic, "board/leds/2"))
   {
   }
   else if (!strcmp(topic, "board/leds/3")) /*测试topic，服务器像设备推送消息server publish*/
   {
      TRACE_INFO("server publish message to device.\r\n");
   }
}

/**
 * @description: 限定随机数范围
 * @param {type} 
 * @return: 
 */
static uint32_t RNG_Get_RandomRange(uint32_t min, uint32_t max)
{
   return HAL_RNG_GetRandomNumber(&hrng) % (max - min + 1) + min;
}

/**
 * @brief Establish MQTT connection
 **/

error_t mqttConnect(void)
{
   error_t error = NO_ERROR;
   IpAddr ipAddr;
   uint32_t trng = 0;
   char server_url[128 + 1] = {0};

   //Debug message
   TRACE_INFO("\r\n\r\nResolving server name...\r\n");

   //Any error to report?
   while (OS_TRUE)
   {
      static int rety = 0;

      //Resolve MQTT server name
      sprintf(server_url, "%s.%s", EXAMPLE_PRODUCT_KEY, TEST_SERVER);
      error = getHostByName(NULL, server_url, &ipAddr, 0);
      if (error == NO_ERROR)
      {
         break;
      }
      else
      {
         rety++;
         if (rety > 3)
         {
            return error;
         }
         OSTimeDly(2000);
      }
   }

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
   mqttClientSetKeepAlive(&mqttClientContext, 120);

#if (APP_SERVER_PORT == 8080 || APP_SERVER_PORT == 8081)
   //Set the hostname of the resource being requested
   mqttClientSetHost(&mqttClientContext, APP_SERVER_NAME);
   //Set the name of the resource being requested
   mqttClientSetUri(&mqttClientContext, APP_SERVER_URI);
#endif

   //Debug message
   TRACE_INFO("Connecting to MQTT server %s...\r\n", ipAddrToString(&ipAddr,NULL));

   //Set Will message
   mqttClientSetWillMessage(&mqttClientContext, "board/status",
                            "offline", 7, MQTT_QOS_LEVEL_0, FALSE);

   trng = RNG_Get_RandomRange(0, 10000);
   TRACE_INFO("ali mqtt RNG:%u\r\n", trng);
   connect_aliyun_data_init(&mqttClientContext, EXAMPLE_DEVICE_NAME, EXAMPLE_DEVICE_SECRET, EXAMPLE_PRODUCT_KEY, trng);
   //Start of exception handling block
   do
   {
      //Establish connection with the MQTT server
      error = mqttClientConnect(&mqttClientContext,
                                &ipAddr, APP_SERVER_PORT, OS_TRUE);
      TRACE_INFO("mqtt connect error:%d\r\n", error);
      //Any error to report?
      if (error)
         break;
      //End of exception handling block
   } while (0);

   //Check status code
   if (error)
   {
      //Close connection
      mqttClientClose(&mqttClientContext);
   }

   //Return status code
   return error;
}
