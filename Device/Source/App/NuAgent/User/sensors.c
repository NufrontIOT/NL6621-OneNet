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
#include "Onenet_comm.h"
#include "EdpKit.h"

enum DATATYPE {
    DATATYPE_INT,
    DATATYPE_DOUBLE,
    DATATYPE_STRING,
};


unsigned char SensorTaskFlag = 0;
extern CLOUD_CONN_VAL_G cloud_conn_status;


int GetTHData(double *temp, double *humi)
{
	*temp = make_rand(22, 30);
	*humi = make_rand(60, 70);

	log_debug("Temp:%lf, Humi:%lf\n", *temp, *humi);
	
	return 0;
}

int GetVCData(double *voltage, double *current, double *power)
{
	*voltage = make_rand(190, 230);
	*current = make_rand(3, 6);
	*power = (*voltage) * (*current);

	log_debug("Voltage:%lf, current:%lf, power:%lf\n", *voltage, *current, *power);
	
	return 0;
}

int GetUPSData(int *energy, int *status_cnt, int *error_cnt)
{
	*energy = make_rand(68, 75);
	*status_cnt = make_rand(0, 2);
	*error_cnt = make_rand(0, 2);

	log_debug("Energy:%d\n", *energy);
	
	return 0;
}

int SendSensorData(char *dataPoint, void* value, unsigned char datatype)
{
	int ret = 0;
    EdpPacket* update_pkg;
    
    switch (datatype) {
        default:
        case DATATYPE_INT:
            update_pkg = PacketSavedataInt(kTypeFullJson, SRC_DEVID, dataPoint, *(int*)value, 0, NULL); 
            break;
        case DATATYPE_DOUBLE:
            update_pkg = PacketSavedataDouble(kTypeFullJson, SRC_DEVID, dataPoint, *(double*)value, 0, NULL); 
            break;
        case DATATYPE_STRING:
            update_pkg = PacketSavedataString(kTypeFullJson, SRC_DEVID, dataPoint, (const char*)value, 0, NULL); 
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
	OSTimeDly(10);

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
	/* 温湿度 */
	double temp = 33;
	double humi = 67;

	/* 电源监控 */
	double voltage = 220;
	double current = 5;
	double power = 1000;

	/* UPS监控 */
	int ups_energy = 75;
	int status_cnt = 0;
	int fault_cnt = 0;
	char *ups_status[3] = {"Work on normal", "Work on charging", "Work on discharging"};
	char *ups_fault[3] = {"Normal", "Outage", "RunOut"};
	char *intrusion_detect = "Unmanned";
	char *smoke_detect = "Smokeless";

	/* 阻塞等待与Onnet连接成功 */
	while (SensorTaskFlag == 0) { 
		OSTimeDly(100);
	}

	while (1) {
		/* 温湿度数据更新上传 */
		GetTHData(&temp, &humi);
        SendSensorData("temperature", &temp, DATATYPE_DOUBLE);
        OSTimeDly(30);
		SendSensorData("humidity", &humi, DATATYPE_DOUBLE);
		OSTimeDly(30);

		GetVCData(&voltage, &current, &power);
        SendSensorData("voltage", &voltage, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SendSensorData("current", &current, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SendSensorData("power", &power, DATATYPE_DOUBLE);
		OSTimeDly(30);

		GetUPSData(&ups_energy, &status_cnt, &fault_cnt);
		OSTimeDly(30);
        SendSensorData("ups-energy", &ups_energy, DATATYPE_INT);

		OSTimeDly(30);
        SendSensorData("ups-status", ups_status[status_cnt], DATATYPE_STRING);
		OSTimeDly(30);
        SendSensorData("ups-fault", ups_fault[fault_cnt], DATATYPE_STRING);

#if 1
		/* 更新入侵监测 */
		OSTimeDly(30);
        SendSensorData("intrusion-detect", intrusion_detect, DATATYPE_STRING);
		OSTimeDly(30);
        SendSensorData("smoke-detect", smoke_detect, DATATYPE_STRING);
#endif		
		PrintGMTTime();
		OSTimeDly(700);
	}
}

void HeartBeatThread(void *arg)
{
	while (1)
	{
		cloud_heartbeat_timer_handle(NULL, NULL);
		OSTimeDly(CLOUD_HEARTBEAT_NUM/10);
	}
}
