#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "usart.h"
#include "sram.h"   
#include "malloc.h"   
#include "string.h"	
#include "dcmi.h"	
#include "ov2640.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "sdio_sdcard.h"  
#include "ff.h"  
#include "w25qxx.h"    
#include "exfuns.h"    
#include "fontupd.h"
#include "text.h"	
#include "atk_qrdecode.h"

//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK STM32F407
//二维码条形码识别  							  
////////////////////////////////////////////////////////////////////////////////// 

u16 qr_image_width;						//输入识别图像的宽度（长度=宽度）
u8 	readok=0;							//采集完一帧数据标识
u32 *dcmi_line_buf[2];					//摄像头采用一行一行读取,定义行缓存  
u16 *rgb_data_buf;						//RGB565帧缓存buf 
u16 dcmi_curline=0;						//摄像头输出数据,当前行编号

//摄像头数据DMA接收完成中断回调函数
//设置DMA传输玩一行图像数据则触发中,并调用此回调函数
void qr_dcmi_rx_callback(void)
{  
	u32 *pbuf;
	u16 i;
	pbuf=(u32*)(rgb_data_buf+dcmi_curline*qr_image_width);//将rgb_data_buf地址偏移赋值给pbuf
	
	/* 当前目标存储器为存储器1 */
	
	if(DMA2_Stream1->CR&(1<<19))//DMA使用buf1,读取buf0
	{ 
		for(i=0;i<qr_image_width/2;i++)
		{
			pbuf[i]=dcmi_line_buf[0][i];
		} 
	}
	/* 当前目标存储器为存储器0 */
	else 										//DMA使用buf0,读取buf1
	{
		for(i=0;i<qr_image_width/2;i++)
		{
			pbuf[i]=dcmi_line_buf[1][i];
		} 
	} 
	dcmi_curline++;
}

//imagewidth:<=240;大于240时,是240的整数倍
//imagebuf:RGB图像数据缓冲区
//解码函数
void qr_decode(u16 imagewidth,u16 *imagebuf)
{
	static u8 bartype=0; //识别二维码,将bartype置0
	u8 *bmp;
	u8 *result=NULL;
	u16 Color;
	u16 i,j;	
	u16 qr_img_width=0;						//输入识别器的图像宽度,最大不超过240!
	u8 qr_img_scale=0;						//压缩比例因子
	
	if(imagewidth > 240)
	{
		if(imagewidth % 240)
			return ;	//不是240的倍数,直接退出
		qr_img_width = 240;
		qr_img_scale = imagewidth / qr_img_width;
	}
	else
	{
		/*图像宽度     < 240 */
		qr_img_width = imagewidth;
		qr_img_scale = 1;
	}  
	result = mymalloc(SRAMIN, 1536); //在内部内存池中申请识别结果存放内存
	bmp = mymalloc(SRAMCCM, qr_img_width*qr_img_width);//CCM管理内存为60K，这里最大可申请240*240=56K 
	mymemset(bmp, 0, qr_img_width*qr_img_width);//清零bmp所在管理内存
	for(i=0; i < qr_img_width; i++)		
	{
		for(j=0;j<qr_img_width;j++)		//将RGB565图片转成灰度
		{	
			Color=*(imagebuf+((i*imagewidth)+j)*qr_img_scale); //按照qr_img_scale压缩成240*240
			*(bmp+i*qr_img_width+j)=(((Color&0xF800)>> 8)*76+((Color&0x7E0)>>3)*150+((Color&0x001F)<<3)*30)>>8; //取R,G,B三通道,并且根据公式将其转变为灰度图
		}		
	}
	
	/**/
	atk_qr_decode(qr_img_width,qr_img_width,bmp,bartype,result);//识别qr灰度图片（记录：单次耗时约0.2S）
	
	if(result[0] == 0)//没有识别出来
	{
		bartype++;
		if(bartype>=5)bartype=0; 
	}
	else if(result[0]!=0)//识别出来了，显示结果
	{	
		BEEP=1;//打开蜂鸣器
		delay_ms(100);
		BEEP=0;
		POINT_COLOR=BLUE; 
		LCD_Fill(0,(lcddev.height+qr_image_width)/2+20,lcddev.width,lcddev.height,BLACK);
		Show_Str(0,(lcddev.height+qr_image_width)/2+20,lcddev.width,
					(lcddev.height-qr_image_width)/2-20,(u8*)result,16,0);//LCD显示识别结果
		printf("\r\nresult:\r\n%s\r\n",result);//串口打印识别结果 		
	}
	myfree(SRAMCCM,bmp);		//释放灰度图bmp内存
	myfree(SRAMIN,result);	    //释放识别结果	
} 

int main(void)
{						 
 	u8 key;						   
	u8 i;	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  			//初始化延时函数
	uart_init(115200); //初始化串口1
	LED_Init();
	BEEP_Init();
	KEY_Init();
	LCD_Init();
	FSMC_SRAM_Init();			  //初始化外部SRAM(1MB)
	my_mem_init(SRAMIN);		//初始化内部内存池(128KB) 
	my_mem_init(SRAMEX);		//初始化外部内存池(1MB)  
	my_mem_init(SRAMCCM);		//初始化CCM内存池(64K)
	W25QXX_Init();				//初始化外部flash(W25Q128)
	POINT_COLOR=RED;        //设置字体为红色 
	LCD_Clear(BLACK);	
	while(font_init()) 			//检查字库
	{	    
		LCD_ShowString(60,50,lcddev.width,16,16,(u8*)"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,lcddev.width,66,WHITE);//清除显示
		delay_ms(200);
	}
	Show_Str_Mid(0,0,(u8*)"探索者F407开发板",16,lcddev.width);	 			    	 
	Show_Str_Mid(0,20,(u8*)"二维码/条形码识别测试",16,lcddev.width);	
	while(OV2640_Init())		//初始化OV2640
	{
		LCD_ShowString(60,50,lcddev.width,16,16,(u8*)"OV2640 Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,lcddev.width,66,WHITE);//清除显示
		delay_ms(200);
	}	
	OV2640_Special_Effects(0);		//正常
	OV2640_RGB565_Mode();			//RGB565模式
	My_DCMI_Init();					//DCMI配置
	
	qr_image_width=lcddev.width;//根据屏幕分辨率设置OV2640的图像尺寸
	if(qr_image_width>480)
	{
		qr_image_width=480;//这里qr_image_width设置为240的倍数
	}
		
	else if(qr_image_width>=240)
	{
		qr_image_width=240;
	}
	Show_Str(0,(lcddev.height+qr_image_width)/2+4,240,16,(u8*)"识别结果：",16,1);
	
	dcmi_line_buf[0]=mymalloc(SRAMIN,qr_image_width*2);						//为行缓存接收申请内存	
	dcmi_line_buf[1]=mymalloc(SRAMIN,qr_image_width*2);						//为行缓存接收申请内存
	rgb_data_buf=mymalloc(SRAMEX,qr_image_width*qr_image_width*2);			//为rgb帧缓存申请内存(外部SRAM),用于存放一帧RGB565图像数据
	
	dcmi_rx_callback=qr_dcmi_rx_callback;//DMA数据接收中断回调函数
	DCMI_DMA_Init(
									(u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],	\
									qr_image_width/2,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable											
							 );//DCMI DMA配置
	
	OV2640_OutSize_Set(qr_image_width,qr_image_width); 
	DCMI_Start();
	printf("SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	
	atk_qr_init();//初始化识别库，为算法申请内存(6K)
	
	printf("1SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("1SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("1SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	while(1)
	{	
		key=KEY_Scan(0);//不支持连按
		if(key)
		{ 
			if(key==KEY2_PRES)break;//按KEY2结束识别
		} 
		if(readok==1)			//采集到了一帧图像
		{		
			readok=0;
			LCD_Color_Fill( (lcddev.width-qr_image_width)/2,(lcddev.height-qr_image_width)/2,	\
											(lcddev.width+qr_image_width)/2-1,(lcddev.height+qr_image_width)/2-1,
											rgb_data_buf );//显示图像
			
			qr_decode(qr_image_width,rgb_data_buf);//识别图像
			
		}
		i++;
		if(i==20)//DS0闪烁.
		{
			i=0;
			LED0=!LED0;
 		}
	}
	atk_qr_destroy();//释放算法内存
	printf("3SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("3SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("3SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	while(1)
	{
		LED0=!LED0;
		delay_ms(200);
	}
}









