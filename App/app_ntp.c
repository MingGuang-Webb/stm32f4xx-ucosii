#include "app_ntp.h"
#include "app_rtc.h"
#include "core/net.h"
#include "debug.h"
#include "time.h"

#define NTP_SERVER      "ntp1.aliyun.com"   
#define NTP_SERVER_IP   "120.25.115.20"
#define NTP_SERVER_PORT 123

Socket *ntp_socket = NULL;

int ntp_init(void)
{   
    ntp_socket = socketOpen(SOCKET_TYPE_DGRAM,SOCKET_IP_PROTO_UDP);
    if (NULL == ntp_socket)
    {
        return -1;
    }
    return 0;
}

int send_sync_ntp_packet(uint8_t version)
{
    int  error;
    ntp_packet packet = {0};
    IpAddr ipAddr;
    time_t txTm;
    size_t reclen = 0;
    uint16_t port = NTP_SERVER_PORT;
    
    getHostByName(NULL, NTP_SERVER_IP, &ipAddr, 0);
    *( ( char * ) &packet + 0 ) = version; // Represents 27 in base 10 or 00011011 in base 2.
    error = socketSendTo(ntp_socket, &ipAddr,port,(uint8_t *)&packet,sizeof(ntp_packet),&reclen,0);
    if (!error)
    {
        error = socketReceiveFrom(ntp_socket,&ipAddr,&port,(uint8_t *)&packet,sizeof(ntp_packet),&reclen,0);
        if (!error)
        {
            packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
            packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.
            txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );
            TRACE_INFO("Time: %s\r\n", ctime( ( const time_t* ) &txTm ) );
        }
        else
        {
            goto __exit;
        }
    }
    else
    {
        goto __exit;
    }
   
    __exit:    
    return error;
}

