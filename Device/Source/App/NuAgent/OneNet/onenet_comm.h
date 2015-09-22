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
！！！！！！！！！！！！！设备之间的透传和上传数据给云端宏定义只能开一个。！！！！！！！！！！！！
*/
#define    Devid_Mode   2

//数字代表以下功能。
typedef enum{
  DEVID_PASSTHROUGH = 1,//设备透传
  DEVID_UPLOAD = 2,	    //设备上传
  DEVID_ALL = 3,        //设备上传和透传
  DEVID_RESTFUL = 4,    //restful协议
  DEVID_NULL,	        //无效
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
1、设备之间的透传时，必须把宏定义Devid_PassThrough打开。 
设备1，宏定义DEVID1，
设备2，屏蔽宏定义DEVID1
*/
//#define    Devid_Mode   1

#ifdef  Devid_PassThrough
  //#define DEVICE1
  #ifdef  DEVICE1 /* DEVICE1 */
	#define SRC_DEVID					"151713"  	  
	#define DEST_DEVID					"154423"  
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"   //设备APIKey
	#define SendData                    "Hi,Im Device1 ID: 151713!!!\r\n"
  #else           /* DEVICE2 */
	#define SRC_DEVID			 		"154423"
	#define DEST_DEVID					"151713"
	#define CLOUD_API_KEY				"fngEM6w3X56lBeYP4u4HaFuB8WoA" //设备APIKey
	#define SendData                    "Hi,Im Device1 ID: 154423!!!\r\n"	
  #endif
#endif 



/*
2、上传数据给云端需要储存确认，把宏定义Storage_Token打开,默认不打开。
*/

//#define   Storage_Token

#ifdef 	Storage_Token
   #define  EDP_TOKEN     "123456789"
#else
   #define  EDP_TOKEN     NULL
#endif


/*
3、只上传数据给云端，使能Data_Upload宏定义
*/
//#define    Devid_Mode   2

#ifdef   Data_Upload
	#define SRC_DEVID					"151713"
	#define DEST_DEVID					"154423"
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"	
#endif


/*
3、只上传数据给云端，并透传给设备，使能Devid_ALL宏定义,需要在同个项目，不同设备ID
*/
//#define    Devid_Mode  3 
#ifdef   Devid_ALL

  //#define DEVICE1

  #ifdef  DEVICE1 /* DEVICE1 仓库环境监控系统*/
	#define SRC_DEVID					"151713"  	  
	#define DEST_DEVID					"154423"  
	#define CLOUD_API_KEY				"mDx9p0nK88Bxsr7BvdX881Nt9yQA"   //设备APIKey
	#define SendData                    "Hi,Im Device1 ID: 151713!!!\r\n"
  #else           /* DEVICE2 设备2，透传*/
	#define SRC_DEVID			 		"154423"
	#define DEST_DEVID					"151713"
	#define CLOUD_API_KEY				"fngEM6w3X56lBeYP4u4HaFuB8WoA" //设备APIKey
	#define SendData                    "Hi,Im Device1 ID: 154423!!!\r\n"	
  #endif

#endif


/*
4、是否使能RestFul测试，使能Devid_RestFul宏定义
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
