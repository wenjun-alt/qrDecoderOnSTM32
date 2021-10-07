#ifndef __ATK_QRDECODE_H
#define __ATK_QRDECODE_H
#include <stdint.h>
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//
//����ά��&����ʶ��LIB��ALIENTEK�ṩ,������ʹ��.
//�ö�ά��&����ʶ��LIB����:
//1,֧��QR��ά��ʶ��.
//2,֧��CODE128,CODE39,I25,EAN13�����ֱ��뷽ʽ������ʶ��.
//3,֧��UTF8-OEMת�����(��Ҫ�ͻ��Լ��ṩת�����).

//����Ҫ֧��UTF82OEMת�������ʱ��:
//1,��ATK_QR_UTF82OEM_SUPPORT���ֵ��Ϊ1(Ĭ����1)
//2,ʵ��atk_qr_convert����.�ú�����unicode����ת��ΪOEM����.
//����,������,����OEM������ַ���(һ�����GBK����),�����ڵ�Ƭ��ϵͳ������ʾ.
//�������Ҫ֧��OEMת��,��:��ATK_QR_UTF82OEM_SUPPORT���ֵ��Ϊ0����,��������������ԭʼ����
//���ַ���(UTF8/GBK).���������,�ڵ�Ƭ��ϵͳ����,�������޷�ֱ����ʾ��,�����Ӣ��,�����ֱ����ʾ.

//����Ҫ֧��GBK����Ķ�ά��ʱ:��ATK_QR_GBK_SUPPORT���ֵ��Ϊ1(Ĭ����1)����.

//��LIB��ֲ����:	
//1,ʵ��atk_qrdecode.c��������к���. 
//2,��ջ(Stack_Size)����Ϊ0X1000������.

//��LIBʹ�ò���:					   
//1,����atk_qr_init����,��ʼ��ʶ�����,����ֵΪATK_QR_OK,���ʼ���ɹ�.
//2,����atk_qr_decode����,��������,��ͼ�����ʶ��.
//3,�����Ҫ��ͣ��ʶ��,���ظ���2�����輴��.
//4,����atk_qr_destroy����,����ʶ��,�ͷ������ڴ�.����ʶ��.

//u8 atk_qr_decode(u16 bmp_width,u16 bmp_heigh,u8 *bmp,u8 btype,u8* result)��������˵��:
//img_width,img_heigh:����ͼ��Ŀ�Ⱥ͸߶�
//imgbuf:ͼ�񻺴���(8λ�Ҷ�ͼ��,����RGB565!!!!)
//btype:0,ʶ���ά��
//      1,ʶ��CODE128����
//      2,ʶ��CODE39����
//      3,ʶ��I25����
//      4,ʶ��EAN13����
//result:ʶ����������.���result[0]==0,��˵��δʶ���κ�����,�������ʶ�𵽵�����(�ַ���)
//����ֵ:ATK_QR_OK,ʶ�����
//            ����,�������
//�����Ҫ������֧�ֵı������ʶ��,����������btypeΪ0~4����ʵ��.
 	  				      
//�汾:V1.4
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��˾��ַ:www.alientek.com
//Copyright(C) ������������ӿƼ����޹�˾ 2016-2026
//All rights reserved	    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//ATK_QR_UTF82OEM_SUPPORT:�����Ƿ�֧�ֽ�UTF8����ת��ΪOEM����(����һ����ָGBK����),����Ϊ:
//1,��������ΪGBK���뷽ʽ���ַ���;����Ϊ0,���������ԭʼ�ַ���(δ������ת��,������UTF8,Ҳ
//������GBK,ȡ���ڶ�ά��ı��뷽ʽ).
#define ATK_QR_UTF82OEM_SUPPORT		1	

//ATK_QR_GBK_SUPPORT:������ATK_QR_UTF82OEM_SUPPORT==1��ʱ��,�Ƿ�֧��ʶ��GBK����Ķ�ά��,��
//������Ϊ1,���������ж���UTF8����GBK,����ʶ����,����ת��.�������Ϊ0,��ֻʶ��UTF8���뷽
//ʽ�Ķ�ά��.��ʶ��GBK����Ķ�ά��.
//ע��:
//1,��ATK_QR_UTF82OEM_SUPPORT==0��ʱ��,�ú궨�岻������.
//2,��������ʶ�����(UTF8ʶ���GBK��)��ʱ��,������ATK_QR_GBK_SUPPORT==0,�Ը��õ�֧��UTF8
//3,�����GBK����������GBK��֧��,��֧��GB2312�����ʶ��.
#define ATK_QR_GBK_SUPPORT			1	


//����ֵ����
#define ATK_QR_OK					0					//����
#define ATK_QR_MEM_ERR				1					//�ڴ����
#define ATK_QR_OTHER_ERR			2					//��������
#define ATK_QR_RECO_ERR				3					//�޷�ʶ��


u8 atk_qr_init(void);									//��ʼ��ʶ���	
u8 atk_qr_decode(u16 bmp_width,u16 bmp_heigh,u8 *bmp,u8 btype,u8* result);//QR���뺯��
void atk_qr_destroy(void);								//����ʶ��	
void atk_qr_memset(void *p,u8 c,u32 len);				//�ڴ����ú��� 
void *atk_qr_malloc(u32 size);							//��̬�����ڴ�
void *atk_qr_realloc(void *ptr,u32 size);				//�ڴ������뺯��  
void atk_qr_free(void *ptr);							//��̬�ͷ��ڴ�
void atk_qr_memcpy(void *des,void *src,u32 n);			//�ڴ渴�ƺ��� 
u16 atk_qr_convert(u16 unicode);						//unicodeתoem
#endif



