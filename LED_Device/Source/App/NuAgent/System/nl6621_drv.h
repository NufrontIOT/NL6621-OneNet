#ifndef __NL6621_DRV_H__
#define __NL6621_DRV_H__

#include "led_drv.h"

typedef struct {
	 unsigned int boot_addr;		
	 unsigned short boot_flag;			
	 unsigned short boot_status;
	 unsigned int fw1_times;
	 unsigned int fw2_times;
}__attribute__((packed))BOOT_PARAM;


#define DEVID_MAXLEN    24
#define API_KEY_MAXLEN  64

typedef struct ONENET_CONFIG {
   char src_devid[DEVID_MAXLEN];	      //设备ID
   char dest_devid[DEVID_MAXLEN];		  //透传，目的ID
   char api_key[API_KEY_MAXLEN];		  //API-KEY

   unsigned char onenet_save; //onenet save保存，1为已保存，0为没有保存
}Onenet_config;


//onenet data
int onenet_GetConfigData(Onenet_config *pConfig);
int onenet_SaveConfigData(Onenet_config *pConfig);

VOID ReadFlash(UINT8* pBuf, UINT32 DataLen, UINT32 ReadStartPos);
VOID WriteFlash(UINT8* pData, UINT32 DataLen, UINT32 BurnStartPos);

int nl6621_GetConfigData(Agent_config_t *pConfig);
int nl6621_SaveConfigData(Agent_config_t *pConfig);
void nl6621_GetWiFiMacAddress(unsigned char *pMacAddress);

void Agent_SaveSSIDAndKey(char *pSSID, char *pKey);

unsigned int nl6621_GetTime_MS(void);
unsigned int nl6621_GetTime_S(void);
void nl6621_Reset(void);
int make_rand(int start, int end);


int nl6621_GetSmartLedData(led_status_flash *pConfig);
int nl6621_SaveSmartLedData(led_status_flash *pConfig);




#endif  /* __nl6621_DRV_H__ */
