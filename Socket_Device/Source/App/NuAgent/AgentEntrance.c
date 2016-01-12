/*
 * =====================================================================================
 *
 *       Filename:  AgentEntrance.c
 *
 *    Description:  Entry of NuAgent
 *
 *        Version:  0.0.1
 *        Created:  2015/6/26 10:18:05
 *       Revision:  none
 *
 *         Author:  Lin Hui (Link), linhui.568@163.com
 *   Organization:  Nufront
 *
 *--------------------------------------------------------------------------------------          
 *       ChangLog:
 *        version    Author      Date          Purpose
 *        0.0.1      Lin Hui    2015/06/26     Create and Initialize
 * =====================================================================================
 */
#include "common.h"
#include "ota.h"

extern const INT8U FwVerNum[3];
const INT8U NuAgentVerNum[3] = {
	0x01,  /* Main version */ 
	0x02,  /* Sub version */
	0x01   /* Internal version */
};

/* system status */
struct sys_status_t sys_status;
extern void check_socket(void);



void print_version(void)
{
				/* SDK Version;   	 NuAgent Version;   	Compile Time; */
	log_notice("Sver:<%x.%02x.%02x>; NuAver:<%x.%02x.%02x>; Time:<%s-%s><%s>;\n", 
		FwVerNum[0], FwVerNum[1], FwVerNum[2],
		NuAgentVerNum[0], NuAgentVerNum[1], NuAgentVerNum[2],
        __DATE__, __TIME__,
		#if DEBUG_ON
		"dbg"
		#else
		"rel"
		#endif
	);
	log_notice("===================================================================\n\n");
}



void nl6621_main_entry(void * pParam)
{
	unsigned char prioUser = TCPIP_THREAD_PRIO + 1;

	/* Print NuAgent version */			
	print_version();	
				   
	/* Create system indicator LED task thread */
	sys_status.status = SYS_STATUS_WIFI_STOP;
	sys_status.onboarding = SYS_STATUS_ONBOARDING_FAILED;		
	sys_thread_new("SysIdxLedThread", SysIdxLedThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 8));
	sys_thread_new("SysResetThread", SysResetThread, NULL, NST_TEST_APP_TASK_STK_SIZE, prioUser);

	/* initialize GAgent resource */
    Agent_Init();

	/* Create network task thread. Include UDP server, UDP Broadcast, TCP server, TCP
	 * login cloud. */
	sys_thread_new("UdpServerThread", UdpServerThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 1));

	//---------------- OTA ----------------//
	#ifdef NL6621_OTA
    sys_thread_new("DevDiscoverThread", DevDiscoverThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 2));
    sys_thread_new("OtaCtrlThread",     OtaCtrlThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 3));
	sys_thread_new("TcpServerThread", OtaDataThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 4));	
  	//---------------- OTA ----------------//
	#endif

	sys_thread_new("TcpCloudThread",  TcpCloudThread, NULL, NST_TEST_APP_TASK_STK_SIZE , (prioUser + 5));

	/* Support user's private task, like NTP*/
	sys_thread_new("OtherTaskThread",  OtherTaskThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 6));

	sys_thread_new("HeartBeatThread", HeartBeatThread, NULL, NST_TEST_APP_TASK_STK_SIZE, (prioUser + 7));	

    while (1) {
		check_socket();	
		OSTimeDly(10);	
    }
}
