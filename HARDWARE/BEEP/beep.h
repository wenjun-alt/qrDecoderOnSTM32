#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//实验环境：ALIENTEK STM32F407开发板
//蜂鸣器驱动代码	   				  
////////////////////////////////////////////////////////////////////////////////// 

//LED端口定义
#define BEEP PFout(8)	// 蜂鸣器控制IO 

void BEEP_Init(void);//初始化		 				    
#endif

















