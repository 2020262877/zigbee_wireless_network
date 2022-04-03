#ifndef SENSOR_H
#define SENSOR_H
#include <hal_types.h>

#define uchar unsigned char
extern void Delay_ms(unsigned int xms);	//��ʱ����
extern void COM(void);                  // ��ʪд��
extern void DHT11(void);                //��ʪ��������

extern uchar temp[2]; 
extern uchar temp1[5];
extern uchar humidity[2];
extern uchar humidity1[9];
extern uchar shidu_shi,shidu_ge,wendu_shi,wendu_ge;

extern int8 readTemp(void);
extern unsigned int GetVol(void);
extern uint16 ReadHumi(void);

#endif 