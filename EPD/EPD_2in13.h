#ifndef _EPD_2IN13_H_
#define _EPD_2IN13_H_

#include "DEV_Config.h"

// wf0213t50cz16 / GDEW0213T5: 104 x 212
#define EPD_2IN13_WIDTH       104
#define EPD_2IN13_HEIGHT      212

void EPD_2IN13_Init(void);
void EPD_2IN13_Clear(void);
void EPD_2IN13_Display(UBYTE *Image);
void EPD_2IN13_UpdateDisplay(void);
void EPD_2IN13_Sleep(void);
void EPD_2IN13_SendCommand(UBYTE Reg);
void EPD_2IN13_SendData(UBYTE Data);

#endif
