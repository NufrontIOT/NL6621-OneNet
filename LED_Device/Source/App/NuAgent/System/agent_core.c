/*
 * =====================================================================================
 *
 *       Filename:  agent_core.c
 *
 *    Description:  Core code of Agent
 *
 *        Version:  0.0.1
 *        Created:  2015/6/26 10:38:05
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


Onenet_config   OneNetdata;
Agent_config_t Agent_config_data;
extern struct sys_status_t sys_status;

/*********************************************/

void onenet_ConfigData(Onenet_config *pConfig)
{
   unsigned char length = 0;

   if(pConfig->onenet_save == 0)
   {
       pConfig->onenet_save = 1;
	   length =  strlen(SRC_DEVID);
	   memcpy((unsigned char *)pConfig->src_devid,(unsigned char *)SRC_DEVID,length);
	   pConfig->src_devid[length] = 0;
	
	   length =  strlen(DEST_DEVID);
	   memcpy((unsigned char *)pConfig->dest_devid,(unsigned char *)DEST_DEVID,length);
	   pConfig->dest_devid[length] = 0;
	
	   length =  strlen(CLOUD_API_KEY);
	   memcpy((unsigned char *)pConfig->api_key,(unsigned char *)CLOUD_API_KEY,length);
	   pConfig->api_key[length] = 0;
	   onenet_SaveConfigData(pConfig);   
   }
}

//id:623622key:2EqccrTz=8Cnt6jril1fm1v6N=Q=
void onenet_setConfigData(char *buffer,unsigned char length)
{
    int src_len = 0,api_len = 0;

    OneNetdata.onenet_save = 1;

	src_len = Search_str(buffer,"key:");

    memcpy((unsigned char *)OneNetdata.src_devid,(unsigned char *)&buffer[3],src_len-7);
    OneNetdata.src_devid[length-7] = 0;


	api_len = length - src_len;
    memcpy((unsigned char *)OneNetdata.api_key,(unsigned char *)&buffer[src_len],api_len);
    OneNetdata.api_key[api_len] = 0;

   	log_info("GET UDP DATA SRC_DEVID:%s CLOUD_API_KEY:%s\r\n",OneNetdata.src_devid,OneNetdata.api_key);
    onenet_SaveConfigData(&OneNetdata); 

}

/****************************************************************/
/****************** NuAgent LED indicater interface *************/
/****************************************************************/

void update_led_indicator(unsigned int low_time, unsigned int high_time)
{
	BSP_GPIOSetValue(USER_GPIO_IDX_LED, GPIO_LOW_LEVEL);
	OSTimeDly(low_time);	
	BSP_GPIOSetValue(USER_GPIO_IDX_LED, GPIO_HIGH_LEVEL);
	OSTimeDly(high_time);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SysIdxLedThread
 *  Description:  Output indicator LED based on system status. 
 *         Note:
 * =====================================================================================
 */
void SysIdxLedThread(void *arg)
{
	log_info("Create system LED indicator task success.\n");

	/* initialize LED indicator gpio */
	BSP_GPIOPinMux(USER_GPIO_IDX_LED);		    /* led indicator */
	BSP_GPIOSetDir(USER_GPIO_IDX_LED, 1);		/* output */
	BSP_GPIOSetValue(USER_GPIO_IDX_LED, 0);	    /* low level */

	while (1) {
		switch (sys_status.status) {
			case SYS_STATUS_WIFI_STOP:
				while (sys_status.status == SYS_STATUS_WIFI_STOP) {
					BSP_GPIOSetValue(USER_GPIO_IDX_LED, GPIO_LOW_LEVEL);
					OSTimeDly(200);
				}
				break;

			case SYS_STATUS_WIFI_SOFTAP:
				update_led_indicator(300, 100);
				break;

			case SYS_STATUS_WIFI_DIRECTCONFING:
				update_led_indicator(10, 10);
				break;

			case SYS_STATUS_WIFI_STA_CONNECTED:
				update_led_indicator(100, 100);
				//BSP_GPIOSetValue(USER_GPIO_IDX_LED, GPIO_HIGH_LEVEL);
				break;

			case SYS_STATUS_WIFI_STA_LOGIN:
				while (sys_status.status == SYS_STATUS_WIFI_STA_LOGIN) {
					//BSP_GPIOSetValue(USER_GPIO_IDX_LED, GPIO_HIGH_LEVEL);
					OSTimeDly(200);
				}
				break;

			default:
				break;
		}
		//OSTimeDly(1);		
	}
}


/****************************************************************/
/******************  Agent reset system interface  **************/
/****************************************************************/
#define USER_RESET_BUTTON_PRESS				(0)		/* 1:for qfn88; 0:for NF-210S */
#define USER_RESET_BUTTON_UP				(0)
#define USER_RESET_PRESS_TIMEOUT			(7)		/* when press 4s, reset system */

static unsigned int system_reset_flag = 0;

void Agent_reset_init(void)
{
	/* Reset gpio is valied when set to low level */
	BSP_GPIOPinMux(USER_GPIO_RESET_BUTTON);  
	BSP_GPIOSetDir(USER_GPIO_RESET_BUTTON, GPIO_DIRECTION_INPUT);

	/* Set GAgent reset flag */
	system_reset_flag = 0x12345678;
	log_info("Create NuAgent reset task success.\n");
}

void SysResetThread(void *arg)
{
	UINT8 gpio_val;
	int i;
	Agent_config_t  pConfigData;

 	log_info("Create SysResetThread task success.\n");
	while (1) 
	{
		if (system_reset_flag == 0x12345678) 
		{
			gpio_val = BSP_GPIOGetValue(USER_GPIO_RESET_BUTTON);
			if (gpio_val == USER_RESET_BUTTON_PRESS) {
				for (i = 0; i < USER_RESET_PRESS_TIMEOUT; i++) {
					OSTimeDly(50);		/* delay 500ms filter button shake */
					gpio_val = BSP_GPIOGetValue(USER_GPIO_RESET_BUTTON);  
					if (gpio_val != USER_RESET_BUTTON_PRESS) {
						break;
					}
				}
				
				if (i == USER_RESET_PRESS_TIMEOUT) {
					
					memcpy(&pConfigData, &Agent_config_data, sizeof(Agent_config_t));
					pConfigData.flag = 0;
					memset(pConfigData.wifi_ssid, 0x0, sizeof(pConfigData.wifi_ssid));
					memset(pConfigData.wifi_key, 0x0, sizeof(pConfigData.wifi_key));
				    
					//SDK1启动程序
	            	//write_boot_flag(BOOT_SDK1_FLAG);
					//OSTimeDly(20); // wait for upgrade finish ack being sent out 

					nl6621_SaveConfigData(&pConfigData);
					/* reset module */
					log_warn("RESET button press more than 4 seconds, reseting System.\n");
					OSTimeDly(100);
					nl6621_Reset();
				} 
			}				
		}
		OSTimeDly(100);
	}
}

void Agent_Init(void)
{
	/* Initialize WIFI base resource */
	Agent_wifi_init();

	/* Initialize NL6621 network */
	Agent_network_init();

	/* Initialize network adapter resource */
	network_adapter_init();

	/* initialize Domain semaphare */
	InitDomainSem();
    
    return;
}


/**********************************************************************/
/************** Network task thread for local or remote ***************/
/**********************************************************************/
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  UdpServerThread
 *  Description:  Task thread for UDP server would be used to process device scan,
 *				register and bind.
 *         Note:
 * =====================================================================================
 */
void UdpServerThread(void *arg)
{
	int ret = 0;
  	log_info("Create UdpServerThread task success.\n");
	while (1) {
	    /* Create UDP server socket */
		while (1) {
			OSTimeDly(50);
		    ret = Socket_CreateUDPServer();
			if (ret < 0) {
				log_info("Create UDP server socket error.\n");
			} else {
				log_info("Create UDP server socket success.\n");
				break;
			} 
		}    

	    Socket_UDPServerRecvData();
	}
}		/* -----  end of function UdpServerThread  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  TcpServerThread
 *  Description:  Task thread for TCP server which can be used to receive MQTT and Gserver
 *              communication data.
 *         Note:
 * =====================================================================================
 */
void TcpServerThread(void *arg)
{   
	int ret = 0;

	while (1) {

		while (sys_status.status != SYS_STATUS_WIFI_STA_CONNECTED)
			OSTimeDly(50);	

	    /* Create TCP server socket */
		while (1) {
	        ret = Socket_CreateTCPServer();
			if (ret < 0) {
				OSTimeDly(50);
				log_info("Create TCP server socket error.\n");
			} else {
				log_info("Create TCP server socket success.\n");
				break;
			} 
		}

		Socket_TCPServerRecvData();
	}
}		/* -----  end of function TcpServerThread  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  TcpCloudThread
 *  Description:  Task thread for TCP login MQTT cloud server 
 *         Note:
 * =====================================================================================
 */


void TcpCloudThread(void *arg)
{
	int ret;   

    log_info("Create TcpCloudThread task success.\n");   
    while (1) {
		while (sys_status.status != SYS_STATUS_WIFI_STA_CONNECTED)
			OSTimeDly(50);

	    onenet_GetConfigData(&OneNetdata);
        onenet_ConfigData(&OneNetdata); //赋值到初始值
	   	log_info("FLASH SRC_DEVID:%s CLOUD_API_KEY:%s\r\n",OneNetdata.src_devid,OneNetdata.api_key);
		while (1) {
	        ret = Socket_CreateTCPClient();
			if (ret < 0) {
				OSTimeDly(50);
				log_info("Create TCP cloud client socket error.\n");
			} else {
				log_info("Create TCP cloud client socket success.\n");
				break;
			} 
		}

	    ret = Agent_cloud_process();
	
    }
}		/* -----  end of function TcpCloudThread  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OtherTaskThread
 *  Description:  Finish user's private task, like NTP, OTA 
 *         Note:
 * =====================================================================================
 */
void OtherTaskThread(void *arg)
{
	int count = 0; 
	int ntp_ret = 1;
	log_info("Create Other Task thread success.\n");

	while (sys_status.status != SYS_STATUS_WIFI_STA_CONNECTED)
		OSTimeDly(50);

	while (1) 
	{
		if ((count <= 50) && (ntp_ret != 0)) 
		{																	   
			OSTimeDly(200);
			ntp_ret = start_ntp();
			//log_info("\tTry get NTP data(count:%d; ret:%d).\n", count, ntp_ret);
			count++;
		} else {
			/* When NTP setup finish, Add OTA functionality here. */

			OSTimeDly(1000);
		}		
	}    
}		/* -----  end of function RTCThread  ----- */


