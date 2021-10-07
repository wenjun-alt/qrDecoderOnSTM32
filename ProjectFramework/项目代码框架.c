程序框架？
{
	1.main函数
	{
		1.初始化所有外设，配置好中断等一系列初始化工作
		比如：初始化LCD,FSMC,USART,LED,BEEP,KEY,字库等,
		配置OV2640图像格式/特效设置/输入图像尺寸-->RGB565模式/普通模式/根据LCD分辨率设置,
		初始化片外FLASH,片内/片外SRAM内存池,
		初始化DCMI,DMA等
		初始化ESP8266,建立与Server端TCP连接 [待改进]
		
		2.申请堆内存，配合中断完成数据传输
		为图像接收开了两个行缓存buffer->放在堆上->片上SRAM,
		还有一个帧缓存rgb_data_buf->片外SRAM(1MB,8080并口时序,FSMC控制器管理)
		由DMA配合DCMI接口以双缓冲机制进行触发DMA中断完成中断回调函数,
		在中断回调中完成一帧图像数据的搬运(片内SRAM->片外SRAM)
		并且在DMA配合DCMI完成一帧数据的搬运后,触发DCMI帧中断,标记readok=1,
		完成ISR与主函数的通信(就是告诉main函数我在ISR中已经完成了一帧数据的传输了,你可以做图像处理并解码了)
		
		[待改进]
		3.将完整一帧图像数据通过WIFI上传到Server端,并在串口中断完成对回传的位置信息进行接收 
		根据接收到的多个qr码的位置信息进行字符串解析,提取出每个qr码的位置信息
		根据解析后的每个位置信息进行图像分割,(假设有一张图片上有5个qr码,就要分割出5张图片)	
			
		4.对单个qr码解码前的图像处理、完整一帧输入图像的显示以及解码操作
		当采集到一帧完整图像后,利用LCD进行显示该帧图像
		根据分割出来的单张图像大小进行图像压缩(具体做法:按照比例等间隔取像素点)
		将rgb565格式的图像进行灰度处理,申请CCM内存
		调用ZBar库的解码函数接口,挨个循环进行解码
		将所有解码结果通过LCD和串口进行显示，解码成功后利用BEEP提示
	}
	2.ISR
	{
		该系统一共涉及有三个中断,并且是中断嵌套的关系,DCMI帧中断 > DMA双缓冲中断 > 串口接收中断
		1.第一个中断是DMA配合DCMI接口的DMA双缓冲中断
		//该DMA中断是为了通过中断回调完成从行缓存数据搬运到外存中从而完成一帧完整图像数据存储
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);　　　　	//开启传输完成中断	
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1; //抢占优先级1 --> 决定是否可以中断嵌套
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0	  --> 决定中断响应顺序
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	     					//根据指定的参数初始化NVIC寄存器
				
		2.第二个中断是DCMI帧中断
		//在第一个中断“完成多次”,即成功传输完一帧图像数据后就会触发帧中断,
		//设置readok=1等标志变量,表示已经完成一帧图像数据的搬运
		DCMI_ITConfig(DCMI_IT_FRAME,ENABLE);　　				//开启帧中断 
		DCMI_Cmd(ENABLE);	　　　　　　　　　　				//DCMI使能
		NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	　　　					//根据指定的参数初始化NVIC寄存器
		
		[待改进]
		3.第三个中断是串口接收中断
		//接收Server回传的qr码的位置信息，标记接收完成变量，并在主函数中完成位置信息的解析
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启相关中断
		//USART1 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//串口1中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3; //抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化NVIC寄存器
	}
}
