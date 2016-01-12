
#include "ota.h"


#ifdef NL6621_OTA

UINT8 *OtaCtrlBuf;
UINT8 *OtaDataBuf; 

UINT32 WritePtr;
UINT32 ReadPtr;

UINT32 upgradeFlag = 0;
UINT32 fileSize = 0;
UINT32 recvFileSize = 0;
UINT32 upgradeSector = 0;
SOCKET_INFO upgrade_socket_info;




//设置启动参数
void write_boot_flag(unsigned char cFlag)
{
    int flag = 0,timer = 0;
	boot_param_t 	 param;

	if(cFlag == BOOT_SDK1_FLAG) {
	   param.boot_addr = SDK1FW_IMG_START_ADDR;
	   param.boot_flag = BOOT_SDK1_FLAG;
	   param.boot_status  = BOOT_STATUS_SDK1;
	} else if(cFlag == BOOT_SDK2_FLAG) {
	   param.boot_addr = SDK2FW_IMG_START_ADDR;
	   param.boot_flag = BOOT_SDK2_FLAG;
	   param.boot_status  = BOOT_STATUS_SDK2;	
	}
	do 
	{
	  	flag = QSpiWriteOneSector(BOOT_PARAM_DATA_ADDR,&param);
		if(timer++ > 20) printf("flash write boot_param fail.\r\n");  
	} while(flag != 0);

    printf("*************** Boot_param Firmware(%d) update success!! *************\r\n",param.boot_flag);
}



VOID DevDiscoverThread(VOID * arg)
{
    int udp_sock,count;
    struct sockaddr_in server_addr, client_addr,addrto;
    socklen_t addr_len = sizeof(struct sockaddr);
    char buffer[UPP_SCAN_DATA_LEN_MAX];
    unsigned char ip_buf[20];
	char *sendbuf = "ok"; 
 
    log_info("Create DevDiscoverThread task success.\n");
DEL_TASK:	
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        DBGPRINT(DEBUG_ERROR, "UDP Broadcast Socket error\n");
        goto DEL_TASK;
    }
  
    memset(&server_addr, 0, sizeof(struct sockaddr_in));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = htonl(IPADDR_ANY); 
    server_addr.sin_port=htons(UDP_SCAN_BROADCAST_PORT); /* port:7878 */ 

    if (bind(udp_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
    {
        DBGPRINT_LWIP(DEBUG_ERROR, "bind error\n");
        goto DEL_TASK;
    }

	OtaDataBuf = (UINT8 *)malloc(FLASH_SECTOR_SIZE);
	if(OtaDataBuf == NULL)
	   	  DBGPRINT_LWIP(DEBUG_ERROR, "OtaDataBuf GET MEM FAIL\n");
	OtaCtrlBuf = (UINT8 *)malloc(OTA_CTRL_BUF_SIZE);
  	if(OtaCtrlBuf == NULL)
	   	  DBGPRINT_LWIP(DEBUG_ERROR, "OtaCtrlBuf GET MEM FAIL\n");
    while (1)
    {
        if (upgradeFlag != 0)
        {
            fcntl(udp_sock, F_SETFL, O_NONBLOCK);
            recvfrom(udp_sock, buffer, UPP_SCAN_DATA_LEN_MAX, 0,
                           (struct sockaddr *)&client_addr, &addr_len);
            OSTimeDly(1);
            continue;
        }
        fcntl(udp_sock, F_SETFL, 0);
        
        DBGPRINT(DEBUG_TRACE, "Waiting for device scan on port %d....\n", UDP_SCAN_BROADCAST_PORT);
        
        memset(buffer, '\0', sizeof(buffer));
        memset(ip_buf, '\0', sizeof(ip_buf));	
        DBGPRINT(DEBUG_WARN, "Poll to read UDP data..........\n");
        
        recvfrom(udp_sock, buffer, UPP_SCAN_DATA_LEN_MAX, 0,
                       (struct sockaddr *)&client_addr, &addr_len);
        
        DBGPRINT(DEBUG_WARN, "Receive APP(IP:%s) discover device broadcast message.......\n", inet_ntoa(client_addr.sin_addr));

        if (strcmp(buffer, UDP_SCAN_STRING) != 0) 
        {
            DBGPRINT(DEBUG_TRACE, "Receive device scan data error.\n");
            continue;
        } 
        else 
        {

            memcpy(ip_buf, (INT8U *)inet_ntoa(client_addr.sin_addr), 
            strlen(inet_ntoa(client_addr.sin_addr)));		
            //DevDiscoverResponse((const char *)ip_buf);
		//	client_addr.sin_port = htons(UDP_SCAN_BROADCAST_RESPONSE_PORT);
		//	printf("udp receive port:%d\n",client_addr.sin_port);
			memset(&addrto, 0, sizeof(struct sockaddr_in));  
    		addrto.sin_family = AF_INET;  
    		addrto.sin_addr.s_addr = inet_addr((const char *)ip_buf); 
    		addrto.sin_port = htons(UDP_SCAN_BROADCAST_RESPONSE_PORT); /* port:8787 */
		   	NVIC_DisableIRQ(TMR1_IRQn);	 
         	*Tmr1Ctl = TMR_INT_MASK; 
			for(count = 0; count < UDP_SCAN_RESPONSE_TIME; count++)
			{
			  	sendto(udp_sock,sendbuf,strlen(sendbuf), 0, (struct sockaddr*)&addrto, sizeof(addrto));
				OSTimeDly(1);
			}  
        }
    }
}

//UDP发送
int DevSendCtrlMsg(UINT8 * buf, int len, SOCKET_INFO* sock_info)
{
    struct sockaddr_in PeerAddr;    // peer address information
    int snd;
    
    PeerAddr.sin_family = AF_INET;         
    PeerAddr.sin_port = sock_info->port;    
    PeerAddr.sin_addr.s_addr = sock_info->ip; 
    memset(PeerAddr.sin_zero, '\0', sizeof(PeerAddr.sin_zero));
    
    snd = sendto(sock_info->local_fd, buf, len, 0,
                        (struct sockaddr *) &PeerAddr, 
                        sizeof(struct sockaddr));

    if (snd < 0) 
    {
        printf("DevSendCtrlMsg failed\n");
        return -1;
    }
    return 0;
}

static void FwUpgradeFinishAck(void)
{
	int ret;
	int count = 0;
    GEN_ACK_PKT upgrade_resp;

    upgrade_resp.header.cmd = DEV_ACK_CMD; 
    upgrade_resp.header.dev_type = WIFI_DEV;
    upgrade_resp.ack_cmd_id = FW_UPGRADE_CMD;
    upgrade_resp.ack_status = 1;
    upgrade_resp.reserve1 = 0;
    upgrade_resp.reserve2 = 0;
    upgrade_resp.header.pkt_len = htons(sizeof(upgrade_resp) - 2); //2个字节的报文长度字段。 
	
    while (1) 
    {	
	    ret = DevSendCtrlMsg((UINT8 *)&upgrade_resp, sizeof(upgrade_resp), &upgrade_socket_info);
	    if (ret < 0) 
	    {
	        printf("Failed to send upgrade data finish ack\n");
	    }													
        
		if (count < UDP_UPGRADE_RESPONSE_TIME+3) 
            OSTimeDly(1);
        else
            break;
        count++; 	
    }
}

static void FwUpgradeProcess(UINT8 * cmd_pkt, unsigned short cmd_len, SOCKET_INFO* sock_info)
{
    GEN_ACK_PKT upgrade_resp;
    FW_UPGRADE_PKT *pkt = (FW_UPGRADE_PKT *)cmd_pkt;
    UINT8 ret = 0;
        
    printf("upgrade start\n");
    upgradeFlag |= UPGRADE_START;
    fileSize = htonl(pkt->fileSize);
    
    upgrade_resp.header.cmd = DEV_ACK_CMD; 
    upgrade_resp.header.dev_type = WIFI_DEV;
    upgrade_resp.ack_cmd_id = FW_UPGRADE_CMD;
    upgrade_resp.ack_status = (ret == 0) ? 1 : 0;
    upgrade_resp.reserve1 = 0;
    upgrade_resp.reserve2 = 0;
    upgrade_resp.header.pkt_len = htons(sizeof(upgrade_resp) - 2); //2个字节的报文长度字段。

    if (DevSendCtrlMsg((UINT8 *)&upgrade_resp, sizeof(upgrade_resp), sock_info) != 0)
    {
        upgradeFlag = 0;
        printf("Failed to send upgrade cmd ack\n");
    }

    return;
}

static void OtaPktProcess(UINT8 * pkt, int pkt_len, SOCKET_INFO* sock_info)
{
    FW_UPGRADE_PKT* pkt_data = (FW_UPGRADE_PKT*)pkt;
    
    UINT16 length;

    printf("UDP control info: pkt_len:%d, device type:%u, cmd_id:%u, flag:%u, fileSize:%u; data package:%u.\n", 
            ntohs(pkt_data->header.pkt_len), 
            pkt_data->header.dev_type, 
            pkt_data->header.cmd,
            pkt_data->flag,
            htonl(pkt_data->fileSize),
            pkt_len);
    
    length = ntohs(pkt_data->header.pkt_len);

    if (pkt_len == (length+2))
    {
        switch (pkt_data->header.cmd) 
        {     
            case FW_UPGRADE_CMD:
                FwUpgradeProcess(pkt, length, sock_info);
                break;
            default:
                break;
        }
    }

    return;
}

void OtaCtrlThread(void *arg)
{
    int sock;
    int bytes_read;
 
    u32_t addr_len;
    struct sockaddr_in server_addr, client_addr;
    SOCKET_INFO client_socket_info;

    log_info("Create OtaCtrlThread task success.\n");
DEL_TASK:	 
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        DBGPRINT_LWIP(DEBUG_ERROR, "Socket error\n");
        goto DEL_TASK;
    }
 
    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_CTRL_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero),0, sizeof(server_addr.sin_zero));
 
    /* 绑定socket到服务端地址 */
    if (bind(sock,(struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        /* 绑定地址失败 */
        DBGPRINT_LWIP(DEBUG_ERROR, "Bind error\n");
        goto DEL_TASK;
    }
 
    addr_len = sizeof(struct sockaddr);
 
    while (1)
    {
        /* 从sock中收取最大600字节数据 */
        bytes_read = recvfrom(sock, OtaCtrlBuf, OTA_CTRL_BUF_SIZE, 0,
                              (struct sockaddr *)&client_addr, &addr_len); 
 
        client_socket_info.local_fd = sock; 
        client_socket_info.ip = client_addr.sin_addr.s_addr;
        client_socket_info.port = client_addr.sin_port;

        upgrade_socket_info = client_socket_info;

        OtaPktProcess(OtaCtrlBuf, bytes_read, &client_socket_info);
    }

}

/*
******************************************************************************
**                        void OtaCtrlThread(void)
**
** Description  : 处理TCP报文，UDP为Process_Pkts  
** Arguments    : 
                  
                  
** Returns      : 无
** Author       :                                   
** Date         : 
**
******************************************************************************
*/

void OtaDataThread(void *arg)
{
   u32_t sin_size;
   int sock, connected, bytes_received;
   struct sockaddr_in server_addr, client_addr;
   BOOL_T stop = NST_FALSE; /* 停止标志 */
   static unsigned char flashstart_flag = 1;

   log_info("Create OtaDataThread task success.\n");    
DEL_TASK:	
   /* 一个socket在使用前，需要预先创建出来，指定SOCK_STREAM为TCP的socket */
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
        /* 创建失败的错误处理 */
        DBGPRINT_LWIP(DEBUG_ERROR, "Socket error\n");
        goto DEL_TASK;
   }

   /* 初始化服务端地址 */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(TCP_DATA_PORT); /* 服务端工作的端口 */
   server_addr.sin_addr.s_addr = INADDR_ANY;
   memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

   /* 绑定socket到服务端地址 */
   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
        /* 绑定失败 */
        DBGPRINT_LWIP(DEBUG_ERROR, "Unable to bind\n");
        goto DEL_TASK;
   }

   /* 在socket上进行监听 */
   if (listen(sock, 5) == -1)
   {
        DBGPRINT_LWIP(DEBUG_ERROR, "Listen error\n");
        goto DEL_TASK;
   }
   
   sin_size = sizeof(struct sockaddr_in);

   DBGPRINT_LWIP(DEBUG_TRACE ,"\nTCPServer Waiting for client on port %d...\n", TCP_DATA_PORT);
   while (stop == NST_FALSE)
   {
        /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* 返回的是连接成功的socket */
        if (connected < 0)
        {
            DBGPRINT_LWIP(DEBUG_ERROR, "accept socket error!");
            continue;
        }

       /* 接受返回的client_addr指向了客户端的地址信息 */
        DBGPRINT_LWIP(DEBUG_TRACE, "I got a connection from (%s , %d)\n",
              inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

	   	NVIC_DisableIRQ(TMR1_IRQn);	 
        *Tmr1Ctl = TMR_INT_MASK; 
        while (1)
        {
            /*B:handle fw upgrade*/
            if (upgradeFlag & UPGRADE_START)
            {
                
                if (!(upgradeFlag & UPGRADE_DOING))
                {
                    WritePtr = 0;
                    recvFileSize = 0;
                    upgradeFlag |= UPGRADE_DOING;    
                }

                /* 从connected socket中接收数据，接收buffer是1024大小，但并不一定能够收到1024大小的数据 */
                bytes_received = recv(connected, &OtaDataBuf[WritePtr], FLASH_SECTOR_SIZE - WritePtr, 0);
                if (bytes_received <= 0)
                {
                    if (bytes_received == 0)
                        DBGPRINT_LWIP(DEBUG_ERROR, "client close connection.\n");
                    else
                        DBGPRINT_LWIP(DEBUG_ERROR, "received failed, server close connection.\n");
                    
                    /* 接收失败，关闭这个connected socket */
                    lwip_close(connected);
                    upgradeFlag = 0;    
                    break;
                }

			  	//V3工具去掉头256byte
				if(flashstart_flag && bytes_received >= 256 && recvFileSize < 4096) {
				     if(memcmp(OtaDataBuf,"Nu_link",6) == 0) {			
						 memcpy(OtaDataBuf,&OtaDataBuf[256],(bytes_received-256));
						 bytes_received -= 256;
						 flashstart_flag = 0;
						 fileSize -= 256; //总长度减去256
					 } else {
					     lwip_close(connected);
                         upgradeFlag = 0;
                         break;
					 }			
				}
			   	//V3工具去掉头256byte

                recvFileSize += bytes_received;
                WritePtr += bytes_received;

                if (recvFileSize == fileSize)
                {
                    upgradeFlag |= UPGRADE_OKFILE;
                    /*写最后一个sector*/
                    if (QSpiWriteOneSector(SDK2FW_IMG_START_ADDR + upgradeSector * FLASH_SECTOR_SIZE, OtaDataBuf) != 0)
                    {
                        lwip_close(connected);
                        upgradeFlag = 0;
                        break;
                    }
                    
                    upgradeSector++;
                    WritePtr = 0; 
				    
					write_boot_flag(BOOT_SDK2_FLAG);
					OSTimeDly(100); // wait for upgrade finish ack being sent out

                    upgradeFlag |= UPGRADE_DONE;
                    upgradeSector = 0;
                    recvFileSize = 0;
                    printf("Firmware update success\n");

                    FwUpgradeFinishAck();
                    OSTimeDly(100); // wait for upgrade finish ack being sent out
                    lwip_close(connected);
                    upgradeFlag = 0;
                    break;
                }
                else if (WritePtr == FLASH_SECTOR_SIZE)
                {				
                    /*写一个sector*/
                    if (QSpiWriteOneSector(SDK2FW_IMG_START_ADDR + upgradeSector * FLASH_SECTOR_SIZE, OtaDataBuf) != 0)
                    {
                        lwip_close(connected);
                        upgradeFlag = 0;   
                        break;
                    }
                    
                    upgradeSector++;
                    WritePtr = 0;
                }
                
                printf("use sector:%d, tot:%d, WritePtr:%d, bytes_received:%d\n", 
						upgradeSector, recvFileSize, WritePtr, bytes_received);
            }
            else
            {
                lwip_close(connected);
                break;
            }
        }
		NVIC_EnableIRQ(TMR1_IRQn);
  		BSP_Timer1Init(4);
    }

}


#endif

