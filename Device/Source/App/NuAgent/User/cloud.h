/*
 * =====================================================================================
 *
 *       Filename:  cloud.h
 *
 *    Description:  Head file of NuAgent cloud network
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
#ifndef __CLOUD_H__
#define __CLOUD_H__

#define CLOUD_HEARTBEAT_NUM  		(10 * 1000)

#define CLOUD_RECV_BUFFER_SIZE		(1500)
#define CLOUD_SEND_BUFFER_SIZE		(1500)


/* 保存云端连接使用的全局变量和状态 */
typedef struct cloud_conn_value_g {
	enum CONN_STATUS {
		SOCK_NONE = 1, 
		SOCK_ERROR,
		SOCK_DONE, 
		CLOUD_CONN_NONE,
		CLOUD_CONN_ERROR,
		CLOUD_CONN_DONE,
	} conn_status;
	unsigned char cloud_conn_flag;
}CLOUD_CONN_VAL_G;

int Agent_cloud_process(void);

#endif

