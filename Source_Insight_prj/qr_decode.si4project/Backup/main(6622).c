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
//��ά��������ʶ��  							  
////////////////////////////////////////////////////////////////////////////////// 

u16 qr_image_width;						//����ʶ��ͼ��Ŀ�ȣ�����=��ȣ�
u8 	readok=0;							//�ɼ���һ֡���ݱ�ʶ
u32 *dcmi_line_buf[2];					//����ͷ����һ��һ�ж�ȡ,�����л���  
u16 *rgb_data_buf;						//RGB565֡����buf 
u16 dcmi_curline=0;						//����ͷ�������,��ǰ�б��

//����ͷ����DMA��������жϻص�����
void qr_dcmi_rx_callback(void)
{  
	u32 *pbuf;
	u16 i;
	pbuf=(u32*)(rgb_data_buf+dcmi_curline*qr_image_width);//��rgb_data_buf��ַƫ�Ƹ�ֵ��pbuf
	
	if(DMA2_Stream1->CR&(1<<19))//DMAʹ��buf1,��ȡbuf0
	{ 
		for(i=0;i<qr_image_width/2;i++)
		{
			pbuf[i]=dcmi_line_buf[0][i];
		} 
	}else 										//DMAʹ��buf0,��ȡbuf1
	{
		for(i=0;i<qr_image_width/2;i++)
		{
			pbuf[i]=dcmi_line_buf[1][i];
		} 
	} 
	dcmi_curline++;
}

//imagewidth:<=240;����240ʱ,��240��������
//imagebuf:RGBͼ�����ݻ�����
//���뺯��
void qr_decode(u16 imagewidth,u16 *imagebuf)
{
	static u8 bartype=0; //ʶ���ά��,��bartype��0
	u8 *bmp;
	u8 *result=NULL;
	u16 Color;
	u16 i,j;	
	u16 qr_img_width=0;						//����ʶ������ͼ����,��󲻳���240!
	u8 qr_img_scale=0;						//ѹ����������
	
	if(imagewidth > 240)
	{
		if(imagewidth % 240)
			return ;	//����240�ı���,ֱ���˳�
		qr_img_width = 240;
		qr_img_scale = imagewidth / qr_img_width;
	}
	else
	{
		/*ͼ����     < 240 */
		qr_img_width = imagewidth;
		qr_img_scale = 1;
	}  
	result = mymalloc(SRAMIN, 1536); //���ڲ��ڴ��������ʶ��������ڴ�
	bmp = mymalloc(SRAMCCM, qr_img_width*qr_img_width);//CCM�����ڴ�Ϊ60K��������������240*240=56K 
	mymemset(bmp, 0, qr_img_width*qr_img_width);//����bmp���ڹ����ڴ�
	for(i=0; i < qr_img_width; i++)		
	{
		for(j=0;j<qr_img_width;j++)		//��RGB565ͼƬת�ɻҶ�
		{	
			Color=*(imagebuf+((i*imagewidth)+j)*qr_img_scale); //����qr_img_scaleѹ����240*240
			*(bmp+i*qr_img_width+j)=(((Color&0xF800)>> 8)*76+((Color&0x7E0)>>3)*150+((Color&0x001F)<<3)*30)>>8; //ȡR,G,B��ͨ��,���Ҹ��ݹ�ʽ����ת��Ϊ�Ҷ�ͼ
		}		
	}
	
	/**/
	atk_qr_decode(qr_img_width,qr_img_width,bmp,bartype,result);//ʶ��qr�Ҷ�ͼƬ����¼�����κ�ʱԼ0.2S��
	
	if(result[0] == 0)//û��ʶ�����
	{
		bartype++;
		if(bartype>=5)bartype=0; 
	}
	else if(result[0]!=0)//ʶ������ˣ���ʾ���
	{	
		BEEP=1;//�򿪷�����
		delay_ms(100);
		BEEP=0;
		POINT_COLOR=BLUE; 
		LCD_Fill(0,(lcddev.height+qr_image_width)/2+20,lcddev.width,lcddev.height,BLACK);
		Show_Str(0,(lcddev.height+qr_image_width)/2+20,lcddev.width,
					(lcddev.height-qr_image_width)/2-20,(u8*)result,16,0);//LCD��ʾʶ����
		printf("\r\nresult:\r\n%s\r\n",result);//���ڴ�ӡʶ���� 		
	}
	myfree(SRAMCCM,bmp);		//�ͷŻҶ�ͼbmp�ڴ�
	myfree(SRAMIN,result);	    //�ͷ�ʶ����	
} 

int main(void)
{						 
 	u8 key;						   
	u8 i;	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  			//��ʼ����ʱ����
	uart_init(115200); //��ʼ������1
	LED_Init();
	BEEP_Init();
	KEY_Init();
	LCD_Init();
	FSMC_SRAM_Init();			  //��ʼ���ⲿSRAM.
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMEX);		//��ʼ���ڲ��ڴ��  
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	W25QXX_Init();				  //��ʼ���ⲿflash(W25Q128)
	POINT_COLOR=RED;        //��������Ϊ��ɫ 
	LCD_Clear(BLACK);	
	while(font_init()) 			//����ֿ�
	{	    
		LCD_ShowString(60,50,lcddev.width,16,16,(u8*)"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,lcddev.width,66,WHITE);//�����ʾ
		delay_ms(200);
	}
	Show_Str_Mid(0,0,(u8*)"̽����F407������",16,lcddev.width);	 			    	 
	Show_Str_Mid(0,20,(u8*)"��ά��/������ʶ�����",16,lcddev.width);	
	while(OV2640_Init())		//��ʼ��OV2640
	{
		LCD_ShowString(60,50,lcddev.width,16,16,(u8*)"OV2640 Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,lcddev.width,66,WHITE);//�����ʾ
		delay_ms(200);
	}	
	OV2640_Special_Effects(0);		//����
	OV2640_RGB565_Mode();			//RGB565ģʽ
	My_DCMI_Init();					//DCMI����
	
	qr_image_width=lcddev.width;
	if(qr_image_width>480)qr_image_width=480;//����qr_image_width����Ϊ240�ı���
	else if(qr_image_width>=240)qr_image_width=240;
	Show_Str(0,(lcddev.height+qr_image_width)/2+4,240,16,(u8*)"ʶ������",16,1);
	
	dcmi_line_buf[0]=mymalloc(SRAMIN,qr_image_width*2);						//Ϊ�л�����������ڴ�	
	dcmi_line_buf[1]=mymalloc(SRAMIN,qr_image_width*2);						//Ϊ�л�����������ڴ�
	rgb_data_buf=mymalloc(SRAMEX,qr_image_width*qr_image_width*2);//Ϊrgb֡���������ڴ�
	
	dcmi_rx_callback=qr_dcmi_rx_callback;//DMA���ݽ����жϻص�����
	DCMI_DMA_Init(
									(u32)dcmi_line_buf[0],(u32)dcmi_line_buf[1],	\
									qr_image_width/2,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable											
							 );//DCMI DMA����
	
	OV2640_OutSize_Set(qr_image_width,qr_image_width); 
	DCMI_Start();
	printf("SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	
	atk_qr_init();//��ʼ��ʶ��⣬Ϊ�㷨�����ڴ�
	
	printf("1SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("1SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("1SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	while(1)
	{	
		key=KEY_Scan(0);//��֧������
		if(key)
		{ 
			if(key==KEY2_PRES)break;//��KEY2����ʶ��
		} 
		if(readok==1)			//�ɼ�����һ֡ͼ��
		{		
			readok=0;
			LCD_Color_Fill( (lcddev.width-qr_image_width)/2,(lcddev.height-qr_image_width)/2,	\
											(lcddev.width+qr_image_width)/2-1,(lcddev.height+qr_image_width)/2-1,
											rgb_data_buf );//��ʾͼ��
			
			qr_decode(qr_image_width,rgb_data_buf);//ʶ��ͼ��
			
		}
		i++;
		if(i==20)//DS0��˸.
		{
			i=0;
			LED0=!LED0;
 		}
	}
	atk_qr_destroy();//�ͷ��㷨�ڴ�
	printf("3SRAM IN:%d\r\n",my_mem_perused(SRAMIN));
	printf("3SRAM EX:%d\r\n",my_mem_perused(SRAMEX));
	printf("3SRAM CCM:%d\r\n",my_mem_perused(SRAMCCM)); 
	while(1)
	{
		LED0=!LED0;
		delay_ms(200);
	}
}









