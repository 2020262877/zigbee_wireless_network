#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"
#include "Sensor.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

#define SEND_DATA_EVENT 0x01

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
  0,                                //  byte  AppNumInClusters;
  (cId_t *)NULL,                    //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
};

endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;   // Task ID for internal task/event processing
byte GenericApp_TransID;  // This is the unique message ID (counter)
devStates_t GenericApp_NwkState;

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GenericApp_SendTheMessage( void );
int8 readTemp(void);
uint16 ReadHumi(void);
void To_string(uint8 *dest,uint8 *src,uint8 l);

void GenericApp_Init( byte task_id )
{
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;
  afRegister( &GenericApp_epDesc );
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
        case ZDO_STATE_CHANGE:
          GenericApp_NwkState=(devStates_t)(MSGpkt->hdr.status);
          if(GenericApp_NwkState==DEV_END_DEVICE||GenericApp_NwkState==DEV_ROUTER)
          {
            osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT);
          }
          break;
        default:
          break;
      }
      osal_msg_deallocate( (uint8 *)MSGpkt );
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    }
    return (events ^ SYS_EVENT_MSG);
  }
  
  if(events & SEND_DATA_EVENT)
  {
    GenericApp_SendTheMessage();
//    osal_start_timerEx(GenericApp_TaskID,SEND_DATA_EVENT,1000);
    osal_start_timerEx( GenericApp_TaskID, SEND_DATA_EVENT,(3000 + (osal_rand() & 0x00FF)) );

    return (events ^ SEND_DATA_EVENT);
  }
  return 0;
}

void GenericApp_SendTheMessage( void )
{
  uint8 tvalue;  
  uint16 vvalue; 
  RFTX rftx;
  uint16 nwk;
  uint16 humidity;
  
  tvalue=readTemp();     //获取土壤温度
  DHT11();               //获取空气温湿度
  humidity =ReadHumi(); //读取土壤湿度传感器引脚上的ad转换值
  vvalue=GetVol()*69/256;//获取节点电压
  rftx.BUF.Head='&';
  
  rftx.BUF.tvalue[0]=tvalue/10+'0';
  rftx.BUF.tvalue[1]=tvalue%10+'0';
 // rftx.BUF.tvalue[2]='*';
 // rftx.BUF.tvalue[3]='C';
  
  rftx.BUF.vvalue[0]=vvalue/10+'0';
  rftx.BUF.vvalue[1]='.';
  rftx.BUF.vvalue[2]=vvalue%10+'0';
  rftx.BUF.vvalue[3]='V';
  
  rftx.BUF.tempvalue[0]=wendu_shi+'0';
  rftx.BUF.tempvalue[1]=wendu_ge+'0';
  //rftx.BUF.tempvalue[2]='*';
  //rftx.BUF.tempvalue[3]='C';
  
  rftx.BUF.humivalue[0]=shidu_shi+'0';
  rftx.BUF.humivalue[1]=shidu_shi+'0';
 // rftx.BUF.humivalue[2]='%';
  
  rftx.BUF.humidity[0]=humidity/10+'0';
  rftx.BUF.humidity[1]=humidity%10+'0';
 // rftx.BUF.humidity[2]='%';
  
  
  //因为DHT11只有两个，zigbee3节（ROU1）点发送的是CC2530内置温度传感器采集的数据emmm
  rftx.BUF.tempvalue[0]= rftx.BUF.tvalue[0];
  rftx.BUF.tempvalue[1]= rftx.BUF.tvalue[1];
  
  rftx.BUF.humivalue[0]=rftx.BUF.humidity[0];
  rftx.BUF.humivalue[1]=rftx.BUF.humidity[1];
  
  
  if(GenericApp_NwkState==DEV_END_DEVICE)
    osal_memcpy(rftx.BUF.type,"END3",4);
  if(GenericApp_NwkState==DEV_ROUTER)
    osal_memcpy(rftx.BUF.type,"ROU1",4);
  
  nwk=NLME_GetShortAddr();
  To_string(rftx.BUF.myNWK,(uint8 *)&nwk,2);
  
  nwk=NLME_GetCoordShortAddr();
  To_string(rftx.BUF.pNWK,(uint8 *)&nwk,2);
  rftx.BUF.Tail='&';

  afAddrType_t my_DstAddr;
  my_DstAddr.addrMode=(afAddrMode_t)Addr16Bit;
  my_DstAddr.endPoint=GENERICAPP_ENDPOINT;
  my_DstAddr.addr.shortAddr=0x0000;
  AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       sizeof(rftx),
                       rftx.databuf,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS);
}

void To_string(uint8 *dest,uint8 *src,uint8 l)
{
  uint8 *xad;
  uint8 i=0;
  uint8 ch;
  xad=src+l-1;
  for(i=0;i<l;i++,xad--)
  {
    ch=(*xad >>4)&0x0F;
    dest[i<<1]=ch+((ch<10) ? '0' : '7');
    ch=*xad&0x0F;
    dest[(i<<1)+1]=ch+((ch<10) ? '0' : '7');
  }
}