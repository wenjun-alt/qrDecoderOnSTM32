#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//实验环境：ALIENTEK STM32F407开发板
//SPI 驱动代码	   								  
////////////////////////////////////////////////////////////////////////////////// 	

 
						  	    													  
void SPI1_Init(void);			 //初始化SPI1口
void SPI1_SetSpeed(u8 SpeedSet); //设置SPI1速度   
u8 SPI1_ReadWriteByte(u8 TxData);//SPI1总线读写一个字节
		 
#endif

