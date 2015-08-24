/*
 * =====================================================================================
 *
 *       Filename:  cloud.c
 *
 *    Description:  cloud network 
 *
 *        Version:  0.0.1
 *        Created:  2015/7/1 14:12:55
 *       Revision:  none
 *
 *         Author:  Lin Hui (Link), linhui.568@163.com
 *   Organization:  Nufront
 *
 *--------------------------------------------------------------------------------------          
 *       ChangLog:
 *        version    Author      Date          Purpose
 *        0.0.1      Lin Hui    2015/7/1      
 *
 * =====================================================================================
 */
#include "common.h"
#include "cloud.h"
#include "Onenet_comm.h"
#include "EdpKit.h"

char * recv_buf = NULL;
char * send_buf = NULL;
CLOUD_CONN_VAL_G cloud_conn_status;

extern unsigned char SensorTaskFlag;
extern void dev_cloud_init(void);


void recv_data_process(char * buffer, int len)
{
    int rtn;
    uint8 mtype;
    RecvBuffer* recvbuf = NULL;
    EdpPacket* pkg;

	log_info("Recv data(len:%d).\n", len);

	recvbuf = NewBuffer();
    WriteBytes(recvbuf, buffer, len);

    while (1)
    {
        /* ��ȡһ����ɵ�EDP�� */
        if ((pkg = GetEdpPacket(recvbuf)) == 0)
        {
            log_info("need more bytes...\n");
            break;
        }

        /* ��ȡ���EDP������Ϣ���ͣ��������EDP������Ϣ����, �ֱ���EDP������ */
        mtype = EdpPacketType(pkg);
        switch(mtype)
        {
            case CONNRESP:
                /* ����EDP�� - ������Ӧ */
                rtn = UnpackConnectResp(pkg);
                log_info("Receive connect response, rtn: %d\n", rtn);
			
				/* ����Sensor Task���� */
				SensorTaskFlag = 1;
				cloud_conn_status.conn_status = CLOUD_CONN_DONE;
				
                break;
            case PINGRESP:
                /* ����EDP�� - ������Ӧ */
                UnpackPingResp(pkg); 
                log_info("recv ping resp\n");
                break;
            case PUSHDATA:
                /* ����EDP�� - ����ת�� */
                break;
            case SAVEDATA:
            case SAVEACK:
            case CMDREQ:
            default:
                /* δ֪��Ϣ���� */
//                log_info("recv failed...\n");
                break;
        }
        DeleteBuffer(&pkg);
    }
    DeleteBuffer(&recvbuf);
}

int Agent_cloud_process(void)
{
	EdpPacket* connect_pkg = NULL;
    int failedcnt = 0;
	int ret, errorcnt = 0;

	cloud_conn_status.conn_status = SOCK_DONE;

AGAIN:    
	/* ��Onenet����������EDP���ӱ��� */
	connect_pkg = PacketConnect1((char*)SRC_DEVID, (char*)CLOUD_API_KEY);

	/* dump package data */
	log_info("Packet connect data(len:%d):\n", connect_pkg->_write_pos);
	dump_hex(connect_pkg->_data, connect_pkg->_write_pos);

	ret = Socket_TCPClientSendData((char *)connect_pkg->_data, connect_pkg->_write_pos);
	DeleteBuffer(&connect_pkg);
	if (ret < 0) {
		cloud_conn_status.conn_status = CLOUD_CONN_ERROR;
		log_info("Send connect data failed(ret:%d).\n", ret);
        failedcnt++;
        /* �������Ӵ�������3�����˳����� */
        if (failedcnt > 3) {
            Socket_TCPClientClose();
            return -1;
        }
        OSTimeDly(50);
		goto AGAIN;
	} else {
		log_notice("Send connect data success.\n");
	}

	/* ���������ڽ����ƶ˷����������� */
	while (1)
	{
		ret = Socket_TCPClientRecvData(recv_buf, CLOUD_RECV_BUFFER_SIZE);
		if (ret > 0) {
            recv_data_process(recv_buf, ret);
		} else if (ret < 0) {
			log_err("Recv tcp client data error(%d)\n", ret);
			errorcnt++;
			if (errorcnt > 5) {
				Socket_TCPClientClose();
				return -1;
			}
		}
	}
}

void Agent_cloud_init(void)
{
	/* ��ʼ���豸��cloud��Դ */
	dev_cloud_init();

	/* ����������ݺͷ������ݵ��ڴ�� */
	recv_buf = malloc(CLOUD_RECV_BUFFER_SIZE);
	send_buf = malloc(CLOUD_SEND_BUFFER_SIZE);
}


