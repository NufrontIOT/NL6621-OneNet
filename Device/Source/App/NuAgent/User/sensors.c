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

enum DATATYPE {
    DATATYPE_INT,
    DATATYPE_DOUBLE,
    DATATYPE_STRING,
};

#ifdef  DEVID2 /* Dev2 */
unsigned char MyMacID[6]={0x61,0x34,0x55,0x55,0x66,0x66};
#else  /* Dev1 */
unsigned char MyMacID[6]={0x61,0x34,0x44,0x44,0x55,0x55};
#endif

unsigned char SensorTaskFlag = 0;
extern CLOUD_CONN_VAL_G cloud_conn_status;
extern unsigned char TCPClient_reset_flag;


int GetTHData(double *temp, double *humi)
{
	*temp = make_rand(22, 30);
	*humi = make_rand(60, 70);

	log_debug("Temp:%f, Humi:%f\n", *temp, *humi);
	
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
//
//EDPЭ��ʵ�������ϴ�
//
#ifdef  Data_Upload
	int SensorData_Upload(char *dataPoint, void* value, unsigned char datatype)
	{
		int ret = 0;
	    EdpPacket* update_pkg;
			   
	    switch (datatype) {
	        default:
	        case DATATYPE_INT:
	            update_pkg = PacketSavedataInt(kTypeFullJson, SRC_DEVID, dataPoint, *(int*)value, 0, EDP_TOKEN); 
	            break;
	        case DATATYPE_DOUBLE:
	            update_pkg = PacketSavedataDouble(kTypeFullJson, SRC_DEVID, dataPoint, *(double*)value, 0, EDP_TOKEN); 
	            break;
	        case DATATYPE_STRING:
	            update_pkg = PacketSavedataString(kTypeFullJson, SRC_DEVID, dataPoint, (const char*)value, 0, EDP_TOKEN); 
	            break;
	    }
		OSTimeDly(10);
	
	    /* ����ת�����ݵ�OneNet��ת����Dev2�豸 */
	    ret = Socket_TCPClientSendData((char*)update_pkg->_data, update_pkg->_write_pos);
		
	    if (ret <= 0) {
	        log_info("Save and transmit full json data failed(ret:%d).\n", ret);
	    } else {
	        log_notice("Save and transmit full json data success.\n");
	    }	
	    DeleteBuffer(&update_pkg);
	
	    return 0;
	}
#endif
//  
//EDPЭ��ʵ���豸�����ݵ�ת��
//
#ifdef  Devid_PassThrough
	int SensorData_PassThrough(char *dest, void* data, unsigned char datalen)
	{
		int ret = 0;
	    EdpPacket* update_pkg;
	 
	    //͸�����ݣ��豸1�����豸2
		update_pkg = PacketPushdata(dest, data, datalen); 
		//dump_hex(update_pkg->_data, update_pkg->_write_pos);
	    /* ����ת�����ݵ�OneNet��ת����Dev2�豸 */
	    ret = Socket_TCPClientSendData((char*)update_pkg->_data, update_pkg->_write_pos);
	
	    if (ret <= 0) {
	        log_info("Pass through and transmit full json data failed(ret:%d).\n", ret);
	    } else {
	        log_notice("Pass through and transmit full json data success.\n");
	    }	
	    DeleteBuffer(&update_pkg);
	
	    return 0;
	}
#endif
//
//�����ƶ�++�豸֮��͸��
//
#ifdef   Devid_ALL
	int SensorData_UploadPass(char *dataPoint, void* value, unsigned char datatype)
	{
		int ret = 0;
	    EdpPacket* update_pkg;
			   
	    switch (datatype) {
	        default:
	        case DATATYPE_INT:
	            update_pkg = PacketSavedataInt(kTypeFullJson, DEST_DEVID, dataPoint, *(int*)value, 0, EDP_TOKEN); 
	            break;
	        case DATATYPE_DOUBLE:
	            update_pkg = PacketSavedataDouble(kTypeFullJson, DEST_DEVID, dataPoint, *(double*)value, 0, EDP_TOKEN); 
	            break;
	        case DATATYPE_STRING:
	            update_pkg = PacketSavedataString(kTypeFullJson, DEST_DEVID, dataPoint, (const char*)value, 0, EDP_TOKEN); 
	            break;
	    }
		OSTimeDly(10);
	
	    /* ����ת�����ݵ�OneNet��ת����Dev2�豸 */
	    ret = Socket_TCPClientSendData((char*)update_pkg->_data, update_pkg->_write_pos);
		
	    if (ret <= 0) {
	        log_info("Save and transmit full json data failed(ret:%d).\n", ret);
	    } else {
	        log_notice("Save and transmit full json data success.\n");
	    }	
	    DeleteBuffer(&update_pkg);
	
	    return 0;
	}
#endif



/*
 * ��OneNet�ƶ˷���������
 * */
void cloud_heartbeat_timer_handle(void *ptmr, void *parg)
{
	int ret = 0;
	EdpPacket* send_pkg = NULL;

	if (cloud_conn_status.conn_status == CLOUD_CONN_DONE)
	{
		/* ��װ������ */
		send_pkg = PacketPing();
	
		/* ���������� */
		ret = Socket_TCPClientSendData((char*)send_pkg->_data, send_pkg->_write_pos);
		if (ret < 0) {
			log_info("Send Heartbeat data failed(ret:%d).\n", ret);
		} else {
			log_notice("Send Heartbeat data success.\n");
		}
		DeleteBuffer(&send_pkg);		
	}	    
}


/* ���������ݲɼ��߳� */
void SenserTaskThread(void *arg)
{
	/* ��ʪ�� */
	double temp = 33;
	double humi = 67;

	/* ��Դ��� */
	double voltage = 220;
	double current = 5;
	double power = 1000;

	/* UPS��� */
	int ups_energy = 75;
	int status_cnt = 0;
	int fault_cnt = 0;
	char *ups_status[3] = {"Work on normal", "Work on charging", "Work on discharging"};
	char *ups_fault[3] = {"Normal", "Outage", "RunOut"};
	char *intrusion_detect = "Unmanned";
	char *smoke_detect = "Smokeless";

	#ifndef  Devid_RestFul //restful��ʽ
		/* �����ȴ���Onnet���ӳɹ� */
		while (SensorTaskFlag == 0) { 
			OSTimeDly(100);
		}
	#endif


	while (1) {
		/* ��ʪ�����ݸ����ϴ� */
#ifdef  Data_Upload	//�豸�ϴ�����

		GetTHData(&temp, &humi);
        SensorData_Upload("temperature", &temp, DATATYPE_DOUBLE);
        OSTimeDly(30);
		SensorData_Upload("humidity", &humi, DATATYPE_DOUBLE);
		OSTimeDly(30);

		GetVCData(&voltage, &current, &power);
        SensorData_Upload("voltage", &voltage, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SensorData_Upload("current", &current, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SensorData_Upload("power", &power, DATATYPE_DOUBLE);
		OSTimeDly(30);

		GetUPSData(&ups_energy, &status_cnt, &fault_cnt);
		OSTimeDly(30);
        SensorData_Upload("ups-energy", &ups_energy, DATATYPE_INT);

		OSTimeDly(30);
        SensorData_Upload("ups-status", ups_status[status_cnt], DATATYPE_STRING);
		OSTimeDly(30);
        SensorData_Upload("ups-fault", ups_fault[fault_cnt], DATATYPE_STRING);

		/* �������ּ�� */
		OSTimeDly(30);
        SensorData_Upload("intrusion-detect", intrusion_detect, DATATYPE_STRING);
		OSTimeDly(30);
        SensorData_Upload("smoke-detect", smoke_detect, DATATYPE_STRING);
		OSTimeDly(200);

#endif 

#ifdef SensorData_UploadPass //�豸�ϴ���͸�����ݸ��ƶ�

	    GetTHData(&temp, &humi);
        SensorData_UploadPass("temperature", &temp, DATATYPE_DOUBLE);
        OSTimeDly(30);
		SensorData_UploadPass("humidity", &humi, DATATYPE_DOUBLE);
		OSTimeDly(30);

		GetVCData(&voltage, &current, &power);
        SensorData_UploadPass("voltage", &voltage, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SensorData_UploadPass("current", &current, DATATYPE_DOUBLE);
		OSTimeDly(30);
        SensorData_UploadPass("power", &power, DATATYPE_DOUBLE);
		OSTimeDly(30);

#endif


#ifdef  Devid_PassThrough //�豸ת������

		SensorData_PassThrough(DEST_DEVID,SendData,sizeof(SendData));
	    OSTimeDly(200);

#endif

#ifdef Devid_RestFul //http devid restful��ʽ

	   if(TCPClient_reset_flag == 1)
	       RestFul_Run();

#endif

		PrintGMTTime();
		OSTimeDly(200);				
	}
}


//tcp ����������ONENET�������ӡ�
void HeartBeatThread(void *arg)
{
	while (1)
	{
		cloud_heartbeat_timer_handle(NULL, NULL);
		OSTimeDly(CLOUD_HEARTBEAT_NUM/10);
	}
}
