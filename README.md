# qrDecoderOnSTM32
关键技术：STM32+OV2640+ESP8266+YOLOv3 + 卷积神经网络 + 多目标检测定位

1. 项目背景及简述

   1.1项目背景

   当前人工智能技术正逐步融入传统行业，同时“互联网+”带来的线上销售日益挤压线下实体店面。实体店面，尤其是大型购物超市不得不融入“互联网+”的浪潮，以通过各种人工智能技术，提升超市服务质量、提高客户满意度，才可能在激烈的竞争中得以生存。分析当下的所有大型购物超市，其常被客户诟病的弊端之一就是收银台的大量排队问题，尽管通过条形码扫码识别、通过二维码支付、甚至是刷脸支付等现代手段已经极大改善了该问题，但核心的收银系统并没有太多改善，仍然拘泥于传统的信息化收银系统。为了进一步提升客户在超市结算时的购物体验，本系统实现了单次批量条形码的快速定位解码，从而使得单人单次结算时间可以在数秒内完成。同时系统实现了基于用户历史数据的商品和上架位置关联，促进商品销售的同时，进一步提升了客户体验。
   智慧收银云系统是基于“AI+万物互联”背景下建立的一个“互联网+智慧收银云”项目。

   1.2 项目简述

   项目针对当前大型超市智能化程度低、收银台效率低下带来的客户结账存在大量排队的问题，首先构建“收银云”，使用深度学习目标定位技术，结合运动目标识别方法，快速完成批量商品条形码定位，随后利用边缘计算思想，结合多条形码解码终端，快速完成批量条形码解码，并对商品售价进行查询，从而在数秒内完成收银台结算过程。同时系统根据用户购买商品的历史习惯数据，采用机器学习算法，完成对商品和超市上架位置的相互关联，进而促进商品销售。  

2. 项目总体方案

![](D:\DevelopFiles\STM32\qrDecoderOnSTM32\img\img.png)

3. 项目总体流程描述

   通过基于 STM32 的批量二维码/条形码扫码器，即嵌入式端完成对商品的条形码拍照并上传至“收银云”服务器， 服务器端完成图片接收，并借助运动目标检测技术、多目标识别与定位技术，训练相应的神经网络框架模型，实现对批量二维码/条形码的定位工作， 并将定位得到的位置信息回传给嵌入式端，嵌入式端根据位置信息完成条形码解码并从服务器获取商品价格并完成结算。  

4. 嵌入式端实验说明

   4.1硬件环境：STM32F407
   
   4.2 ATK_QR.lib功能:（该解码库经过移植后，不提供源码）
   	1.支持QR二维码识别。（包括常见的UTF8编码格式和特殊的GBK编码格式）。
   	2.支持EAN码、39码、交叉25码、UPC码、128码等编码方式的条码识别。
   	3.支持UTF8-OEM转换输出(需要客户自己提供转换码表，OEM一般指GBK编码)。
   	4.内存占用：6K（解码所需算法内存）+ bmp_heigh* bmp_width(Byte--8位灰度图像内存）。	
   
   4.3 硬件资源
   	1.LED
   	2.BEEP
   	3.KEY
   	4.LCD
   	5.OV2640摄像头
   	6.外部SRAM
   	7.外部FLASH	
   
   4.4 实验现象
   
   编译下载代码，LCD显示当前识别图像。将含有多个二维码、条形码的图像放在摄像头面前，手动旋转调节OV2640模块镜头直至图像清晰，则系统会扫面图像并识别，如果识别到了则蜂鸣器会“滴”一声，并在LCD显示识别结果及发送给串口。通过按钮（KEY2）结束识别。同时DS1闪烁表示OV2640图像帧中断，DS0闪烁速度慢表示识别图像模式，闪烁快则表示已结束识别。 
   
   4.5 注意事项
   
   ​	1.移植ATK_QR.lib需将堆栈(Stack_Size)设置为0X1000或以上；
   
   ​	2.LCD识别结果可打开串口工具查看结果。 

