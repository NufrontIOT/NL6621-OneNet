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
        /* 获取一个完成的EDP包 */
        if ((pkg = GetEdpPacket(recvbuf)) == 0)
        {
            log_info("need more bytes...\n");
            break;
        }

        /* 获取这个EDP包的消息类型，根据这个EDP包的消息类型, 分别做EDP包解析 */
        mtype = EdpPacketType(pkg);
        switch(mtype)
        {
            case CONNRESP:
                /* 解析EDP包 - 连接响应 */
                rtn = UnpackConnectResp(pkg);
                log_info("Receive connect response, rtn: %d\n", rtn);
			
				/* 启动Sensor Task任务 */
				SensorTaskFlag = 1;
				cloud_conn_status.conn_status = CLOUD_CONN_DONE;
				
                break;
            case PINGRESP:
                /* 解析EDP包 - 心跳响应 */
                UnpackPingResp(pkg); 
                log_info("recv ping resp\n");
                break;
            case PUSHDATA:
                /* 解析EDP包 - 数据转发 */
                break;
            case SAVEDATA:
            case SAVEACK:
            case CMDREQ:
            default:
                /* 未知消息类型 */
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
	/* 向Onenet服务器发送EDP连接报文 */
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
        /* 发送连接次数超过3次则退出连接 */
        if (failedcnt > 3) {
            Socket_TCPClientClose();
            return -1;
        }
        OSTimeDly(50);
		goto AGAIN;
	} else {
		log_notice("Send connect data success.\n");
	}

	/* 主进程用于接收云端服务器的数据 */
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
	/* 初始化设备端cloud资源 */
	dev_cloud_init();

	/* 分配接收数据和发送数据的内存块 */
	recv_buf = malloc(CLOUD_RECV_BUFFER_SIZE);
	send_buf = malloc(CLOUD_SEND_BUFFER_SIZE);
}


