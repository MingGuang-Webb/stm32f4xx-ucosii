/*
 * @Author: your name
 * @Date: 2020-06-07 11:16:22
 * @LastEditTime: 2020-06-23 09:54:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f767-uscoii\App\app_main.c
 */
#include "app_main.h"
#include "os_debug.h"
#include "gpio.h"
#include "core/net.h"
#include "drivers/mac/stm32f4xx_eth_driver.h"
#include "drivers/phy/lan8720_driver.h"
#include "dhcp/dhcp_client.h"
#include "ipv6/slaac.h"
#include "rng/yarrow.h"
#include "debug.h"
#include "app_mqtt.h"
#include "easyflash.h"

extern RNG_HandleTypeDef hrng;

//Ethernet interface configuration
#define APP_IF_NAME "eth0"
#define APP_HOST_NAME "mqtt-client-ali"
#define APP_MAC_ADDR "00-AB-CD-EF-07-67"

#define APP_USE_DHCP_CLIENT ENABLED
#define APP_IPV4_HOST_ADDR "192.168.42.66"
#define APP_IPV4_SUBNET_MASK "255.255.255.0"
#define APP_IPV4_DEFAULT_GATEWAY "192.168.43.1"
#define APP_IPV4_PRIMARY_DNS "223.5.5.5"
#define APP_IPV4_SECONDARY_DNS "223.6.6.6"

#define APP_USE_SLAAC ENABLED
#define APP_IPV6_LINK_LOCAL_ADDR "fe80::767"
#define APP_IPV6_PREFIX "2001:db8::"
#define APP_IPV6_PREFIX_LENGTH 64
#define APP_IPV6_GLOBAL_ADDR "2001:db8::767"
#define APP_IPV6_ROUTER "fe80::1"
#define APP_IPV6_PRIMARY_DNS "2001:4860:4860::8888"
#define APP_IPV6_SECONDARY_DNS "2001:4860:4860::8844"

//#define APP_SERVER_PORT TEST_PORT //MQTT over TLS
//#define APP_SERVER_PORT 8884 //MQTT over TLS (mutual authentication)
//#define APP_SERVER_PORT 8080 //MQTT over WebSocket
//#define APP_SERVER_PORT 8081 //MQTT over secure WebSocket

//URI (for MQTT over WebSocket only)
#define APP_SERVER_URI "/ws"

//Client's certificate
const char_t clientCert[] =
    "-----BEGIN CERTIFICATE-----"
    "MIICGjCCAYOgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix"
    "FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE"
    "CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y"
    "ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0xOTEwMjIwOTIw"
    "NDFaFw0yMDAxMjAwOTIwNDFaMEAxCzAJBgNVBAYTAkZSMRYwFAYDVQQKDA1Pcnl4"
    "IEVtYmVkZGVkMRkwFwYDVQQDDBBtcXR0LWNsaWVudC1kZW1vMFkwEwYHKoZIzj0C"
    "AQYIKoZIzj0DAQcDQgAEWT/enOkLuY+9NzUQPOuNVFARl5Y3bc4lLt3TyVwWG0Ez"
    "IIk8Wll5Ljjrv+buPSKBVQtOwF9VgyW4QuQ1uYSAIaMaMBgwCQYDVR0TBAIwADAL"
    "BgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADgYEAWoiW35xauLH2mTzMtE6Pdavi"
    "MyFgnBNeve9USawRLdrbgDJ0CwFw4ssduvrCe8wSfvkqEsJXsNMBP7cXEXbWz2nr"
    "AN1UDxZGoF9kaQgYboDwzL83B9jo/F/JHJRDDUxYW1vNvvkc1mobkejW/3Zz4aYt"
    "Hs4tpUyogRO/5N57X/w="
    "-----END CERTIFICATE-----";

//Client's private key
const char_t clientKey[] =
    "-----BEGIN EC PRIVATE KEY-----"
    "MHcCAQEEICYULY0KQ6nDAXFl5tgK9ljqAZyb14JQmI3iT7tdScDloAoGCCqGSM49"
    "AwEHoUQDQgAEWT/enOkLuY+9NzUQPOuNVFARl5Y3bc4lLt3TyVwWG0EzIIk8Wll5"
    "Ljjrv+buPSKBVQtOwF9VgyW4QuQ1uYSAIQ=="
    "-----END EC PRIVATE KEY-----";

const char_t trustedCaList[] = "\
-----BEGIN CERTIFICATE-----\r\n\
MIIDUDCCAjgCCQD8SgoeHeWPKjANBgkqhkiG9w0BAQsFADBpMQswCQYDVQQGEwJD\r\n\
TjEQMA4GA1UECAwHYmVpamluZzEQMA4GA1UEBwwHYmVpamluZzEXMBUGA1UECgwO\r\n\
c2Vnd2F5LW5pbmVib3QxHTAbBgNVBAsMFHNlZ3dheS1uaW5lYm90IG1xdHRzMCAX\r\n\
DTIwMDIyMDExNDYzMFoYDzIwNjAwMjEwMTE0NjMwWjBpMQswCQYDVQQGEwJDTjEQ\r\n\
MA4GA1UECAwHYmVpamluZzEQMA4GA1UEBwwHYmVpamluZzEXMBUGA1UECgwOc2Vn\r\n\
d2F5LW5pbmVib3QxHTAbBgNVBAsMFHNlZ3dheS1uaW5lYm90IG1xdHRzMIIBIjAN\r\n\
BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0n6sI5rmlSDGCpzqlFFo9PCiL9ki\r\n\
vrHDtp9w43ORWC0vQsD//wBZsymSo0D32DK1EubFYepRZ3eTlToE/QbmAsvklF6a\r\n\
TFJNK+9XLOfuctWecWcBgLR1mkPQWTnBpg6Ux6MT0R5jiW6/NZoRcxxs2JbkLSKr\r\n\
x1AYruQcubh/xZMgcmK6Z+gA4j9ktq4vB1x+HfSqdy4KSgYfSYZIATOuX/yFMOQ/\r\n\
wv/RhpxK6Ehz2kIjbLgupiLHDLMsDSoQi/aQZjr29qeRqXvjT9KnQ4IZNNrjTuC1\r\n\
+fyrWgG2uQvFTT2cBH+xWKv0PIS7m7zNGv+1ql7k3/jFv14Il1cHSDet4QIDAQAB\r\n\
MA0GCSqGSIb3DQEBCwUAA4IBAQBrzLzLjLU4vk+pNJy8xVT0G3eEbkVmPZaw3vQ0\r\n\
lvLJ3spCC0dZ0s0ZVPuGCLMJcsEwfgHqY0CpESAhSWBIKtjmjqKHCtYCuLQ9hKVB\r\n\
ewAbX0cAye+tM3IQVKClhpaVeLtPatywaX0HEDdZGdGvVl6sscusvmPHe4JToifD\r\n\
dTe9X575KbS+/XDr44fNQM0Hu8hXZLJJY0YTMjRLwtls42zbQco1lYn7Lf6HrwNs\r\n\
0fWdiJ+vbnsORIPjUZL2Gr1qe/c8qfXObw6PPNWz2MtfTOxpAn39Z6Ml2g7iFa6E\r\n\
r9yenqwMukrfzHIEBb9AxSv9BDdxa0hvaRXXsdPjRTMF+Bz+\r\n\
-----END CERTIFICATE-----\r\n\
";

DhcpClientSettings dhcpClientSettings;
DhcpClientContext dhcpClientContext;
SlaacSettings slaacSettings;
SlaacContext slaacContext;
YarrowContext yarrowContext;
uint8_t seed[32];

/**
 * @description: ucosii的优先级必须唯一，后面可以靠这个去删除任务和挂起任务操作
 * @param {type} 
 * @return: 
 */
#define LED_STAKE_SIZE 128
OS_STK led_blink_stake[LED_STAKE_SIZE];
#define LED_TASK_PRO 3

#define UART_STAKE_SIZE 128
OS_STK uart_stake[UART_STAKE_SIZE];
#define UART_TASK_PRO 2

#define TCP_STAKE_SIZE 1024 /* IMU_STAKE_SIZE*2byte */
OS_STK tcp_stake[TCP_STAKE_SIZE];
#define TCP_TASK_PRO 1

static OS_FLAG_GRP *event; /* 创建事件标志组 */
static void *pq_group[10]; /* ucosii的队列只能传输指针,不能想freeRTOS那种复制操作 */
OS_EVENT *pq_event;

void led_blink_task(void *arg);
void uart_task(void *arg);
void tcp_task(void *arg);

void ucos_show_version(void)
{
   ucos_kprintf("\n \\ |  /\n");
   ucos_kprintf("- UCOS -     Ucosii Operating System\n");
   ucos_kprintf(" / |  \\     Version:V%d.%d.%d build %s\n",
                OS_VERSION / 10000, OS_VERSION % 10000 / 100, OS_VERSION % 100, __DATE__);
   ucos_kprintf(" i - n Copyright by Micrium RTOS.\n");
}

void create_app_task(void)
{
   ucos_show_version();

   ucos_kprintf("create ucosii app task.\r\n");
   OSTaskCreate(led_blink_task, (void *)0, (OS_STK *)&led_blink_stake[LED_STAKE_SIZE - 1], LED_TASK_PRO);
   OSTaskCreate(uart_task, (void *)0, (OS_STK *)&uart_stake[UART_STAKE_SIZE - 1], UART_TASK_PRO);
   OSTaskCreate(tcp_task, (void *)0, (OS_STK *)&tcp_stake[TCP_STAKE_SIZE - 1], TCP_TASK_PRO);
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void led_blink_task(void *arg)
{
   INT8U event_err;
   event = OSFlagCreate(0, &event_err);
   if (OS_ERR_NONE == event_err)
   {
      ucos_kprintf("create OS event success.\r\n");
   }
   for (;;)
   {
      /* code */
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
      OSTimeDly(100); /*正常的延时函数*/
   }
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void uart_task(void *arg)
{
   pq_event = OSQCreate(pq_group, 10);
   if (OS_ASCII_NUL != pq_event)
   {
      ucos_kprintf("Queue Create Success.\r\n");
   }
   for (;;)
   {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      /* code */
      OSTimeDlyHMSM(0, 0, 0, 500); /*按照时分秒的方式进行延时*/
   }
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void tcp_task(void *arg)
{
   error_t error;
   uint32_t value = 0;
   NetInterface *interface;
   MacAddr macAddr;

#if (APP_USE_DHCP_CLIENT == DISABLED)
   Ipv4Addr ipv4Addr;
#endif
#if (APP_USE_SLAAC == DISABLED)
   Ipv6Addr ipv6Addr;
#endif

   //Start-up message
   TRACE_INFO("\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("*** CycloneTCP MQTT Client Demo ***\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("Copyright: 2010-2019 Oryx Embedded SARL\r\n");
   TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
   TRACE_INFO("Target: STM32F407ZG\r\n");
   TRACE_INFO("\r\n");

   //Generate a random seed
   for (int i = 0; i < 32; i += 4)
   {
      //Get 32-bit random value
      HAL_RNG_GenerateRandomNumber(&hrng, &value);

      //Copy random value
      seed[i] = value & 0xFF;
      seed[i + 1] = (value >> 8) & 0xFF;
      seed[i + 2] = (value >> 16) & 0xFF;
      seed[i + 3] = (value >> 24) & 0xFF;
   }

   //PRNG initialization
   error = yarrowInit(&yarrowContext);
   //Any error to report?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize PRNG!\r\n");
   }    

   //Properly seed the PRNG
   error = yarrowSeed(&yarrowContext, seed, sizeof(seed));
   //Any error to report?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to seed PRNG!\r\n");
   }

   //TCP/IP stack initialization
   error = netInit();
   //Any error to report?
   if (error)
   {
      ucos_kprintf("tcp/ip init error:%d\r\n", error);
      //Debug message
      TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
   }

   //Configure the first Ethernet interface
   interface = &netInterface[0];

   //Set interface name
   netSetInterfaceName(interface, APP_IF_NAME);
   //Set host name
   netSetHostname(interface, APP_HOST_NAME);
   //Set host MAC address
   macStringToAddr(APP_MAC_ADDR, &macAddr);
   netSetMacAddr(interface, &macAddr);
   //Select the relevant network adapter
   netSetDriver(interface, &stm32f4xxEthDriver);
   netSetPhyDriver(interface, &lan8720PhyDriver);

   //Initialize network interface
   error = netConfigInterface(interface);
   //Any error to report?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);
   }

#if (IPV4_SUPPORT == ENABLED)
#if (APP_USE_DHCP_CLIENT == ENABLED)
   //Get default settings
   dhcpClientGetDefaultSettings(&dhcpClientSettings);
   //Set the network interface to be configured by DHCP
   dhcpClientSettings.interface = interface;
   //Disable rapid commit option
   dhcpClientSettings.rapidCommit = FALSE;

   //DHCP client initialization
   error = dhcpClientInit(&dhcpClientContext, &dhcpClientSettings);
   //Failed to initialize DHCP client?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize DHCP client!\r\n");
   }

   //Start DHCP client
   error = dhcpClientStart(&dhcpClientContext);
   //Failed to start DHCP client?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to start DHCP client!\r\n");
   }
   else
   {
      TRACE_PRINTF("dhcp ipaddr:%d,%d\r\n", dhcpClientContext.serverIpAddr, dhcpClientContext.requestedIpAddr);
   }
#else
   //Set IPv4 host address
   ipv4StringToAddr(APP_IPV4_HOST_ADDR, &ipv4Addr);
   ipv4SetHostAddr(interface, ipv4Addr);

   //Set subnet mask
   ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &ipv4Addr);
   ipv4SetSubnetMask(interface, ipv4Addr);

   //Set default gateway
   ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &ipv4Addr);
   ipv4SetDefaultGateway(interface, ipv4Addr);

   //Set primary and secondary DNS servers
   ipv4StringToAddr(APP_IPV4_PRIMARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 0, ipv4Addr);
   ipv4StringToAddr(APP_IPV4_SECONDARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 1, ipv4Addr);
#endif
#endif

#if (IPV6_SUPPORT == ENABLED)
#if (APP_USE_SLAAC == ENABLED)
   //Get default settings
   slaacGetDefaultSettings(&slaacSettings);
   //Set the network interface to be configured
   slaacSettings.interface = interface;

   //SLAAC initialization
   error = slaacInit(&slaacContext, &slaacSettings);
   //Failed to initialize SLAAC?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize SLAAC!\r\n");
   }

   //Start IPv6 address autoconfiguration process
   error = slaacStart(&slaacContext);
   //Failed to start SLAAC process?
   if (error)
   {
      //Debug message
      TRACE_ERROR("Failed to start SLAAC!\r\n");
   }
#else
   //Set link-local address
   ipv6StringToAddr(APP_IPV6_LINK_LOCAL_ADDR, &ipv6Addr);
   ipv6SetLinkLocalAddr(interface, &ipv6Addr);

   //Set IPv6 prefix
   ipv6StringToAddr(APP_IPV6_PREFIX, &ipv6Addr);
   ipv6SetPrefix(interface, 0, &ipv6Addr, APP_IPV6_PREFIX_LENGTH);

   //Set global address
   ipv6StringToAddr(APP_IPV6_GLOBAL_ADDR, &ipv6Addr);
   ipv6SetGlobalAddr(interface, 0, &ipv6Addr);

   //Set default router
   ipv6StringToAddr(APP_IPV6_ROUTER, &ipv6Addr);
   ipv6SetDefaultRouter(interface, 0, &ipv6Addr);

   //Set primary and secondary DNS servers
   ipv6StringToAddr(APP_IPV6_PRIMARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 0, &ipv6Addr);
   ipv6StringToAddr(APP_IPV6_SECONDARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 1, &ipv6Addr);
#endif
#endif

   easyflash_init();
   for (;;)
   {
      mqtt_net_fsm();
      osDelayTask(10u);
   }
}
