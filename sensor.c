#include "Sensor.h"
#include <ioCC2530.h>
#include "OnBoard.h"

#define HAL_ADC_REF_115V 0x00
#define HAL_ADC_DEC_256  0x20

#define HAL_ADC_CHN_TEMP 0x0e
typedef unsigned int  uint;

#define DATA_PIN P0_7

void Delay_us(void);
void Delay_10us(void);
void Delay_ms(uint Time);
void COM(void);
void DHT11(void);

//温湿度定义
uchar ucharFLAG,uchartemp;
uchar shidu_shi,shidu_ge,wendu_shi,wendu_ge=4;
uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;

//延时函数
void Delay_us(void) //1 us延时
{
    MicroWait(1);   
}

void Delay_10us(void) //10 us延时
{
   MicroWait(10);
}

void Delay_ms(uint Time)//n ms延时
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
     Delay_10us();
  }
}

//温湿度传感
void COM(void)    // 温湿写入
{     
    uchar i;         
    for(i=0;i<8;i++)    
    {
        ucharFLAG=2; 
        while((!DATA_PIN)&&ucharFLAG++);
        Delay_10us();
        Delay_10us();
        Delay_10us();
        uchartemp=0;
        if(DATA_PIN)uchartemp=1;
        ucharFLAG=2;
        while((DATA_PIN)&&ucharFLAG++);   
        if(ucharFLAG==1)break;    
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp; 
    }    
}

void DHT11(void)   //温湿传感启动
{
    DATA_PIN=0;
    Delay_ms(19);  //>18MS
    DATA_PIN=1; 
    P0DIR &= ~0x80; //重新配置IO口方向
    Delay_10us();
    Delay_10us();                        
    Delay_10us();
    Delay_10us();  
    if(!DATA_PIN) 
    {
        ucharFLAG=2; 
        while((!DATA_PIN)&&ucharFLAG++);
        ucharFLAG=2;
        while((DATA_PIN)&&ucharFLAG++); 
        COM();
        ucharRH_data_H_temp=ucharcomdata;
        COM();
        ucharRH_data_L_temp=ucharcomdata;
        COM();
        ucharT_data_H_temp=ucharcomdata;
        COM();
        ucharT_data_L_temp=ucharcomdata;
        COM();
        ucharcheckdata_temp=ucharcomdata;
        DATA_PIN=1; 
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        if(uchartemp==ucharcheckdata_temp)
        {
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
        }
        wendu_shi=ucharT_data_H/10; 
        wendu_ge=ucharT_data_H%10;
        
        shidu_shi=ucharRH_data_H/10; 
        shidu_ge=ucharRH_data_H%10;        
    } 
    else //没用成功读取，返回0
    {
        wendu_shi=0; 
        wendu_ge=0;
        
        shidu_shi=0; 
        shidu_ge=0;  
    } 
    
    P0DIR |= 0x80; //IO口需要重新配置 
}

//__________________________________________________

uint16 ReadHumi( void )
{
  uint16 reading = 0;
  ADCCFG |= 0x40;
  ADCCON3 = 0x86;
  while (!(ADCCON1 & 0x80));
  ADCCFG &= (0x40 ^ 0xFF);
  reading = ADCL;
  reading |= (int16) (ADCH << 8); 
  reading >>= 8;
  return (reading);
}
//_________________________________________
int8 readTemp(void)
{
  static uint16 reference_voltage;
  static uint8 bCalibrate=TRUE;
  uint16 value;
  int8 temp;
  
  ATEST=0x01;
  TR0|=0x01;
  ADCIF=0;
  ADCCON3=(HAL_ADC_REF_115V|HAL_ADC_DEC_256|HAL_ADC_CHN_TEMP);
  while(!ADCIF);
  ADCIF=0;
  value=ADCL;
  value|=((uint16)ADCH)<<8;
  value>>=4;
  
  if(bCalibrate)
  { reference_voltage=value;
    bCalibrate=FALSE;
  }
  temp=22+((value-reference_voltage)/4);
  return temp;
}

unsigned int GetVol(void)
{
  unsigned int value;
  uint8 tmpADCCON3=ADCCON3;

  ADCIF=0; 
  ADCCON3=(HAL_ADC_REF_115V|HAL_ADC_DEC_256|HAL_ADC_CHN_TEMP);
  while(!ADCIF);
  
  value=ADCH;
  ADCCON3=tmpADCCON3;
  return value;
}