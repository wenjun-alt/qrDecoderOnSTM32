该二维码&条码识别LIB功能:
1.支持QR二维码识别.
2.支持CODE128.CODE39.I25.EAN13等四种编码方式的条码识别.
3.支持UTF8-OEM转换输出(需要自己提供转换码表).
	
本LIB移植步骤:	
1.实现atk_qrdecode.c里面的所有函数. 
2.堆栈(Stack_Size)设置为0X1000或以上.
 	   
本LIB使用步骤:					   
1.调用atk_qr_init函数.初始化识别程序.返回值为ATK_QR_OK.则初始化成功.
2.调用atk_qr_decode函数.给定参数.对图像进行识别.
3.如果需要不停的识别.则重复第2个步骤即可.
4.调用atk_qr_destroy函数.结束识别.释放所有内存.结束识别.