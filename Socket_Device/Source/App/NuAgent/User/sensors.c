/*
 * =====================================================================================
 *
 *       Filename:  sensors.c
 *
 *    Description:  Eviroment sensor process interface
 *
 *        Version:  0.0.1
 *        Created:  2015/8/10 14:12:55
 *       Revision:  none
 *
 *         Author:  Lin Hui (Link), linhui.568@163.com
 *   Organization:  Nufront
 *
 *--------------------------------------------------------------------------------------          
 *       ChangLog:
 *        version    Author      Date          Purpose
 *        0.0.1      Lin Hui    2015/8/10      
 *
 * =====================================================================================
 */

#include "common.h"
#include "sensors.h"
#include "EdpKit.h"
#include "socket_drv.h"

enum DATATYPE {
    DATATYPE_INT,
    DATATYPE_DOUBLE,
    DATATYPE_STRING,
};




extern CLOUD_CONN_VAL_G cloud_conn_status;
extern unsigned char TCPClient_reset_flag;
extern struct sys_status_t sys_status;
extern Onenet_config   OneNetdata;




int SensorData_Upload(char *dataPoint, void* value, unsigned char datatype)
{
	int ret = 0;
    EdpPacket* update_pkg;
		   
    switch (datatype) {
        default:
        case DATATYPE_INT:
            update_pkg = PacketSavedataInt(kTypeFullJson, OneNetdata.src_devid, dataPoint, *(int*)value, 0, NULL); 
            break;
        case DATATYPE_DOUBLE:
            update_pkg = PacketSavedataDouble(kTypeFullJson, OneNetdata.src_devid, dataPoint, *(double*)value, 0, NULL); 
            break;
        case DATATYPE_STRING:
            update_pkg = PacketSavedataString(kTypeFullJson, OneNetdata.src_devid, dataPoint, (const char*)value, 0, NULL); 
            break;
    }
	OSTimeDly(10);

    /* 发送转存数据到OneNet并转发到Dev2设备 */
    ret = Socket_TCPClientSendData((char*)update_pkg->_data, update_pkg->_write_pos);
	
    if (ret <= 0) {
        log_info("Save and transmit full json data failed(ret:%d).\n", ret);
    } else {
        log_notice("Save and transmit full json data success.\n");
    }	
    DeleteBuffer(&update_pkg);

    return 0;
}

/*
 * 向OneNet云端发送心跳包
 * */
void cloud_heartbeat_timer_handle(void *ptmr, void *parg)
{
	int ret = 0;
	EdpPacket* send_pkg = NULL;

	if (cloud_conn_status.conn_status == CLOUD_CONN_DONE)
	{
		/* 封装心跳包 */
		send_pkg = PacketPing();
	
		/* 发送心跳包 */
		ret = Socket_TCPClientSendData((char*)send_pkg->_data, send_pkg->_write_pos);
		if (ret < 0) {
			log_info("Send Heartbeat data failed(ret:%d).\n", ret);
		} else {
			log_notice("Send Heartbeat data success.\n");
		}
		DeleteBuffer(&send_pkg);		
	}	    
}


/* 传感器数据采集线程 */
void SenserTaskThread(void *arg)
{
//    log_info("Create SenserTaskThread task success.\n");   
//
//	while (sys_status.status != SYS_STATUS_WIFI_STA_LOGIN)
//			OSTimeDly(100);
//
//	upload_led_data(0xff);
//
//	while (1) {
//
//		if(sys_status.status != SYS_STATUS_WIFI_STA_LOGIN) {
//		   OSTimeDly(30);
//	       continue;
//		}
//
//        upload_led_data(Attr_flags);
//		Attr_flags = 0;
//        OSTimeDly(100);			
//	}
}




//tcp 发送心跳给ONENET保持链接。
void HeartBeatThread(void *arg)
{
	while (1)
	{  
   	    PrintGMTTime();
		if(sys_status.status == SYS_STATUS_WIFI_STA_LOGIN) {
		    cloud_heartbeat_timer_handle(NULL, NULL);
	     	OSTimeDly(CLOUD_HEARTBEAT_NUM/10);
		} else {
		   	OSTimeDly(500);
		}
	}
}
