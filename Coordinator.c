#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "math.h"
#include "stdlib.h"
#include "Coordinator.h"
#include "DebugTrace.h"
#if !defined( WIN32 )
#include "OnBoard.h"
#endif
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  0,                                //  byte  AppNumInClusters;
  (cId_t *)NULL                     //  byte *pAppInClusterList;
};

endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;   // Task ID for internal task/event processing
byte GenericApp_TransID;  // This is the unique message ID (counter)

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GenericApp_SendTheMessage( void );

void GenericApp_Init( byte task_id )
{
  halUARTCfg_t uartConfig;
  GenericApp_TaskID = task_id;
  GenericApp_TransID = 0;
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;
  afRegister( &GenericApp_epDesc );
  
  uartConfig.configured       =TRUE;
  uartConfig.baudRate         =HAL_UART_BR_115200;
  uartConfig.flowControl      =FALSE;
  uartConfig.callBackFunc     =NULL;
  HalUARTOpen (0, &uartConfig);
}

UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case AF_INCOMING_MSG_CMD:
          GenericApp_MessageMSGCB( MSGpkt );
          break;
        default:
          break;
      }
      osal_msg_deallocate( (uint8 *)MSGpkt );
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    }
    return (events ^ SYS_EVENT_MSG);
  }
  return 0;
}

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  RFTX rftx;
  unsigned char buffer[2]={13,10};
  int len=(int)pow(10,(abs(pkt->rssi)/10))*100;
  unsigned char len1[2]={len/10,len%10};
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      osal_memcpy(&rftx,pkt->cmd.Data,sizeof(rftx));
      
//      HalUARTWrite(0,"type:",5);
      HalUARTWrite(0,rftx.BUF.type,sizeof(rftx.BUF.type));
      
      HalUARTWrite(0," MYNWK:",7);
      HalUARTWrite(0,rftx.BUF.myNWK,sizeof(rftx.BUF.myNWK));
      
      HalUARTWrite(0," PNWK:",6);
      HalUARTWrite(0,rftx.BUF.pNWK,sizeof(rftx.BUF.pNWK));
      
      HalUARTWrite(0," Vol:",5);
      HalUARTWrite(0,rftx.BUF.vvalue,sizeof(rftx.BUF.vvalue));
     
      HalUARTWrite(0," temperture:",12);
      HalUARTWrite(0,rftx.BUF.tempvalue,sizeof(rftx.BUF.tempvalue));
  
      HalUARTWrite(0," humidity:",10);
      HalUARTWrite(0,rftx.BUF.humivalue,sizeof(rftx.BUF.humivalue));
      
      /*
      HalUARTWrite(0," temperture:",12);
      HalUARTWrite(0,rftx.BUF.tvalue,sizeof(rftx.BUF.tvalue));
      
      HalUARTWrite(0," humidity:",10);
      HalUARTWrite(0,rftx.BUF.humidity,sizeof(rftx.BUF.humidity));*/
      
//     HalUARTWrite(0,buffer,2); 
      HalUARTWrite(0,buffer,2);
      break;
  }
}