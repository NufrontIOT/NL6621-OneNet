#ifndef __ONENET_COMM_H__
#define __ONENET_COMM_H__

/*---------------------------------------------------------------------------*/
/* Type Definition Macros                                                    */
/*---------------------------------------------------------------------------*/
typedef unsigned char   uint8;
typedef char            int8;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned int    uint32;
typedef int             int32;


/*
���������������������������豸֮���͸�����ϴ����ݸ��ƶ˺궨��ֻ�ܿ�һ����������������������������
*/
#define    Devid_Mode   2

//���ִ������¹��ܡ�
typedef enum{
  DEVID_PASSTHROUGH = 1,//�豸͸��
  DEVID_UPLOAD = 2,	    //�豸�ϴ�
  DEVID_ALL = 3,        //�豸�ϴ���͸��
  DEVID_RESTFUL = 4,    //restfulЭ��
  DEVID_NULL,	        //��Ч
}devid_mode;

#if	  (Devid_Mode==1)	  
  #define   Devid_PassThrough
#elif (Devid_Mode==2) 
  #define   Data_Upload 
#elif (Devid_Mode==3)
  #define  	Devid_ALL
#elif (Devid_Mode==4)
  #define   Devid_RestFul 
#endif

/*
1���豸֮���͸��ʱ������Ѻ궨��Devid_PassThrough�򿪡� 
�豸1���궨��DEVID1��
�豸2�����κ궨��DEVID1
*/
//#define    Devid_Mode   1

#ifdef  Devid_PassThrough
  //#define DEVICE1
  #ifdef  DEVICE1 /* DEVICE1 */
	#define SRC_DEVID					"151713"  	  
	#define DEST_DEVID					"154423"  
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"   //�豸APIKey
	#define SendData                    "Hi,Im Device1 ID: 151713!!!\r\n"
  #else           /* DEVICE2 */
	#define SRC_DEVID			 		"154423"
	#define DEST_DEVID					"151713"
	#define CLOUD_API_KEY				"fngEM6w3X56lBeYP4u4HaFuB8WoA" //�豸APIKey
	#define SendData                    "Hi,Im Device1 ID: 154423!!!\r\n"	
  #endif
#endif 



/*
2���ϴ����ݸ��ƶ���Ҫ����ȷ�ϣ��Ѻ궨��Storage_Token��,Ĭ�ϲ��򿪡�
*/

//#define   Storage_Token

#ifdef 	Storage_Token
   #define  EDP_TOKEN     "123456789"
#else
   #define  EDP_TOKEN     NULL
#endif


/*
3��ֻ�ϴ����ݸ��ƶˣ�ʹ��Data_Upload�궨��
*/
//#define    Devid_Mode   2

#ifdef   Data_Upload
	#define SRC_DEVID					"151713"
	#define DEST_DEVID					"154423"
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"	
#endif


/*
3��ֻ�ϴ����ݸ��ƶˣ���͸�����豸��ʹ��Devid_ALL�궨��,��Ҫ��ͬ����Ŀ����ͬ�豸ID
*/
//#define    Devid_Mode  3 
#ifdef   Devid_ALL

  //#define DEVICE1

  #ifdef  DEVICE1 /* DEVICE1 �ֿ⻷�����ϵͳ*/
	#define SRC_DEVID					"151713"  	  
	#define DEST_DEVID					"154423"  
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"   //�豸APIKey
	#define SendData                    "Hi,Im Device1 ID: 151713!!!\r\n"
  #else           /* DEVICE2 �豸2��͸��*/
	#define SRC_DEVID			 		"154423"
	#define DEST_DEVID					"151713"
	#define CLOUD_API_KEY				"fngEM6w3X56lBeYP4u4HaFuB8WoA" //�豸APIKey
	#define SendData                    "Hi,Im Device1 ID: 154423!!!\r\n"	
  #endif

#endif


/*
4���Ƿ�ʹ��RestFul���ԣ�ʹ��Devid_RestFul�궨��
*/
//#define    Devid_Mode   4

#ifdef   Devid_RestFul
	#define RESTFUL_SERVER_DOMAIN	    "api.heclouds.com"
	#define RESTFUL_SERVER_PORT_NUM		(80)
	#define SRC_DEVID					"151713"
	#define DEST_DEVID					"154423"	
	#define HTTP_API_KEY                "eafOBKUzn3Ox8Zo3UUA8EyVhjd4A"
#endif



#endif /* __ONENET_COMM_H__ */
