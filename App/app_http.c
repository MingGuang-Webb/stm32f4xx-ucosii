/*
 * @Author: your name
 * @Date: 2020-06-23 18:32:51
 * @LastEditTime: 2020-06-29 17:50:40
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\App\app_http.c
 */ 
#include "app_http.h"
#include "http/http_client.h"
#include "ucos_ii.h"
#include "debug.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "yarrow.h"

static enum __HTTP_STATE http_state = HTTP_INIT_STATE;
static HttpClientContext httpClientContext;
static YarrowContext yarrowContext;

#define APP_HTTP_URI "/anything"
#define APP_HTTP_SERVER_PORT 443
#define APP_HTTP_SERVER_NAME "www.httpbin.org"
#define APP_SET_CIPHER_SUITES DISABLED
#define APP_SET_SERVER_NAME DISABLED
#define APP_SET_TRUSTED_CA_LIST DISABLED

error_t httpClientTlsInitCallback(HttpClientContext *context,
   TlsContext *tlsContext)
{
   error_t error;

   //Debug message
   TRACE_INFO("HTTP Client: TLS initialization callback\r\n");

   //Set the PRNG algorithm to be used
   error = tlsSetPrng(tlsContext, YARROW_PRNG_ALGO, &yarrowContext);
   //Any error to report?
   if(error)
      return error;

#if (APP_SET_CIPHER_SUITES == ENABLED)
   //Preferred cipher suite list
   error = tlsSetCipherSuites(tlsContext, cipherSuites, arraysize(cipherSuites));
   //Any error to report?
   if(error)
      return error;
#endif

#if (APP_SET_SERVER_NAME == ENABLED)
   //Set the fully qualified domain name of the server
   error = tlsSetServerName(tlsContext, APP_HTTP_SERVER_NAME);
   //Any error to report?
   if(error)
      return error;
#endif

#if (APP_SET_TRUSTED_CA_LIST == ENABLED)
   //Import the list of trusted CA certificates
   error = tlsSetTrustedCaList(tlsContext, trustedCaList, trustedCaListLength);
   //Any error to report?
   if(error)
      return error;
#endif

   //Successful processing
   return NO_ERROR;
}

int http_connect(void)
{
    int rc = 0;
    error_t error;
    size_t length;
    uint_t status;
    const char_t *value;
    IpAddr ipAddr;
    char_t buffer[128];

    switch (http_state)
    {
    case /* constant-expression */ HTTP_INIT_STATE:
        /* code */
        if (netGetLinkState(&netInterface[0]))
        {
            //Initialize HTTP client context
            httpClientInit(&httpClientContext);
            http_state = HTTP_DNS_STATE;
        }
        break;
    case HTTP_DNS_STATE:
        //Debug message
        TRACE_INFO("\r\n\r\nResolving server name...\r\n");

        //Resolve HTTP server name
        error = getHostByName(NULL, APP_HTTP_SERVER_NAME, &ipAddr, 0);
        //Any error to report?
        if (error)
        {
            //Debug message
            TRACE_INFO("Failed to resolve server name!\r\n");
            break;
        }
        http_state = HTTP_OPEN_STATE;
        break;
    case HTTP_OPEN_STATE:
        http_state = HTTP_CONNECT_STATE;
        break;
    case HTTP_CONNECT_STATE:
        http_state = HTTP_SET_STATE;
#if (APP_HTTP_SERVER_PORT == 443)
        //Register TLS initialization callback
        error = httpClientRegisterTlsInitCallback(&httpClientContext,
                                                  httpClientTlsInitCallback);
        //Any error to report?
        if (error)
            break;
#endif
        //Select HTTP protocol version
        error = httpClientSetVersion(&httpClientContext, HTTP_VERSION_1_1);
        //Any error to report?
        if (error)
        {
            break;
        }
        //Set timeout value for blocking operations
        error = httpClientSetTimeout(&httpClientContext, 20000);
        //Any error to report?
        if (error)
        {
            
            break;
        }
        //Debug message
        TRACE_INFO("Connecting to HTTP server %s...\r\n",
                   ipAddrToString(&ipAddr, NULL));

        //Connect to the HTTP server
        error = httpClientConnect(&httpClientContext, &ipAddr,
                                  APP_HTTP_SERVER_PORT);
        //Any error to report?
        if (error)
        {
            http_state = HTTP_CLOSE_STATE;
            //Debug message
            TRACE_INFO("Failed to connect to HTTP server!,[%d]\r\n",error);
            break;
        }
        break;
    case HTTP_SET_STATE:
        http_state = HTTP_RECV_RESP_STATE;
        //Create an HTTP request
        httpClientCreateRequest(&httpClientContext);
        httpClientSetMethod(&httpClientContext, "POST");
        httpClientSetUri(&httpClientContext, APP_HTTP_URI);

        //Set the hostname and port number of the resource being requested
        httpClientSetHost(&httpClientContext, APP_HTTP_SERVER_NAME,
                          APP_HTTP_SERVER_PORT);

        //Set query string
        //httpClientAddQueryParam(&httpClientContext, "param1", "value1");
        //httpClientAddQueryParam(&httpClientContext, "param2", "value2");

        //Add HTTP header fields
        httpClientAddHeaderField(&httpClientContext, "User-Agent", "Mozilla/5.0"); /*服务器判断客户端浏览器类别*/
        httpClientAddHeaderField(&httpClientContext, "Content-Type", "text/plain"); /*http请求时提交的内容类型*/
        httpClientAddHeaderField(&httpClientContext, "Transfer-Encoding", "chunked"); /*传输过程中的编码类型*/

        //Send HTTP request header
        error = httpClientWriteHeader(&httpClientContext);
        //Any error to report?
        if (error)
        {
            http_state = HTTP_CLOSE_STATE;
            //Debug message
            TRACE_INFO("Failed to write HTTP request header!\r\n");
            break;
        }

        //Send HTTP request body
        error = httpClientWriteBody(&httpClientContext, "Hello World!", 12,
                                    NULL, 0);
        //Any error to report?
        if (error)
        {
            http_state = HTTP_CLOSE_STATE;
            //Debug message
            TRACE_INFO("Failed to write HTTP request body!\r\n");
            break;
        }

        //Receive HTTP response header
        error = httpClientReadHeader(&httpClientContext);
        //Any error to report?
        if (error)
        {
            http_state = HTTP_CLOSE_STATE;
            //Debug message
            TRACE_INFO("Failed to read HTTP response header!\r\n");
            break;
        }

        //Retrieve HTTP status code
        status = httpClientGetStatus(&httpClientContext);
        //Debug message
        TRACE_INFO("HTTP status code: %u\r\n", status);

        //Retrieve the value of the Content-Type header field
        value = httpClientGetHeaderField(&httpClientContext, "Content-Type");

        //Header field found?
        if (value != NULL)
        {
            //Debug message
            TRACE_INFO("Content-Type header field value: %s\r\n", value);
        }
        else
        {
            //Debug message
            TRACE_INFO("Content-Type header field not found!\r\n");
        }
        break;
    case HTTP_RECV_RESP_STATE:
        //Read data
        error = httpClientReadBody(&httpClientContext, buffer,
                                   sizeof(buffer) - 1, &length, 0);

        //Check status code
        if (!error)
        {
            //Properly terminate the string with a NULL character
            buffer[length] = '\0';
            //Dump HTTP response body
            TRACE_INFO("%s", buffer);
        }
        break;
    case HTTP_READ_STATE:
        break;
    case HTTP_CLOSE_STATE:
        http_state = HTTP_INIT_STATE;
        //Gracefully disconnect from the HTTP server
        httpClientDisconnect(&httpClientContext);
        //Release HTTP client context
        httpClientDeinit(&httpClientContext);
        OSTimeDly(2000);
        break;
    default:
        break;
    }
    return rc;
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void http_task(void *arg)
{
   for (;;)
   {
      http_connect();
      OSTimeDly(10);
   }
}

