#if !defined( COORDINATOR_H)
#define COORDINATOR_H
#endif

#include "zComDef.h"

#define GENERICAPP_ENDPOINT           10

#define GENERICAPP_PROFID             0x0F04
#define GENERICAPP_DEVICEID           0x0001
#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0
#define GENERICAPP_MAX_CLUSTERS       1
#define GENERICAPP_CLUSTERID          1

typedef union h
{
  uint8 databuf[26];
  struct RFRXBUF
  {
    unsigned char Head;
    unsigned char type[4];
    unsigned char myNWK[4];
    unsigned char pNWK[4];
    unsigned char vvalue[4];
    unsigned char tvalue[2];
    unsigned char tempvalue[2];
    unsigned char humivalue[2];
    unsigned char humidity[2];
    unsigned char Tail;
  }BUF;
}RFTX;

extern void GenericApp_Init( byte task_id );
extern UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events );