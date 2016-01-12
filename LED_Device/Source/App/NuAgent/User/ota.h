#ifndef __OTA_H__
#define __OTA_H__

#include "common.h"


#define NL6621_OTA

#ifdef NL6621_OTA

#define UDP_CTRL_PORT  7200
#define TCP_DATA_PORT 8200

#define OTA_CTRL_BUF_SIZE    256

#define WIFI_DEV  0x01

#define FW_UPGRADE_CMD 0x20
#define DEV_ACK_CMD  0xA1

#define UPGRADE_START   0x01
#define UPGRADE_END     0x02
#define UPGRADE_DOING   0x04
#define UPGRADE_OKFILE  0x08
#define UPGRADE_DONE    0x10

typedef struct 
{
    INT32 local_fd;
    UINT32 ip;
    UINT16 port;
}SOCKET_INFO;

typedef struct
{
    UINT16 pkt_len;       //报文长度
    UINT8 dev_type;       //设备类型
    UINT8  cmd;            //命令ID  
}__attribute__((packed)) PKT_HEADER;

typedef struct 
{
    PKT_HEADER     header;         //报文头
    UINT8  ack_cmd_id;     //确认命令ID
    UINT8  ack_status;     //确认状态
    UINT8  reserve1;       //备用1
    UINT8  reserve2;       //备用2
}__attribute__((packed)) GEN_ACK_PKT;

typedef struct 
{
    PKT_HEADER      header;         //报文头
    UINT8   flag;
    UINT8   unused1;  
    UINT32  fileSize;
    UINT8   unused2;
    UINT8   unused3;
}__attribute__((packed)) FW_UPGRADE_PKT;

#define UPP_SCAN_DATA_LEN_MAX                    (10)
#define UDP_SCAN_BROADCAST_PORT                  (7878)
#define UDP_SCAN_BROADCAST_RESPONSE_PORT         (8787)
#define UDP_SCAN_RESPONSE_TIME                   (5) 
#define UDP_SCAN_STRING                          "scan"                   
#define UDP_UPGRADE_RESPONSE_TIME                (3)



#define CALIBRATION_ADDR        (0x32000)
#define BOOT_PARAM_DATA_ADDR	(0x33000)
#define SDK1FW_IMG_START_ADDR	(0x2000)
#define SDK2FW_IMG_START_ADDR	(0x34000)
#define SDKFW_IMG_SIZE_MAX 		(0x30000 - 0x1000)

/* Boot Flag */
enum BOOT_FLAG {
	BOOT_SDK1_FLAG = 1,
	BOOT_SDK2_FLAG = 2,
	BOOT_SDK_OTHER_FLAG = 3,
};

/* Boot status */
enum BOOT_STATUS {
	BOOT_STATUS_NONE = 0,
	BOOT_STATUS_SDK1 = 1,
	BOOT_STATUS_SDK2 = 2,
	BOOT_STATUS_OTHER = 3,
};

/* Bootloader data structure */
typedef struct boot_param_t {
	unsigned int   boot_addr;
	unsigned short boot_flag;
	unsigned short boot_status;
	unsigned int   sdk1_cnt;
	unsigned int   sdk2_cnt;
} boot_param_t;


VOID DevDiscoverThread(VOID * arg);
void OtaCtrlThread(void *arg);
void OtaDataThread(void *arg);

void write_boot_flag(unsigned char cFlag);

#endif


#endif  





