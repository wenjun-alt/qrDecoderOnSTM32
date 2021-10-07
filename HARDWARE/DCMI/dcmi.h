#ifndef _DCMI_H
#define _DCMI_H
#include "sys.h"    									
//////////////////////////////////////////////////////////////////////////////////	 
//实验环境：ALIENTEK STM32F407开发板
//DCMI 驱动代码	   	  
////////////////////////////////////////////////////////////////////////////////// 	
extern void (*dcmi_rx_callback)(void);//DCMI DMA接收回调函数
void My_DCMI_Init(void);
void DCMI_DMA_Init(u32 mem0addr,u32 mem1addr,u16 memsize,u32 memblen,u32 meminc);
void DCMI_Start(void);
void DCMI_Stop(void);

void DCMI_Set_Window(u16 sx,u16 sy,u16 width,u16 height);
void DCMI_CR_Set(u8 pclk,u8 hsync,u8 vsync);

#endif





















