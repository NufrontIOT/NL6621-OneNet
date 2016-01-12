#include "common.h"
#include "socket_drv.h"
#include "uart.h"



/**********************************************/
/***   ²å×ù³ÌÐò   ***/
int socket_state = 0;

extern  struct sys_status_t sys_status;
extern 	int SensorData_Upload(char *dataPoint, void* value, unsigned char datatype);



void check_socket(void)
{
	UINT8 gpio10_val;
	UINT8 gpio30_val;
    static unsigned char upload_first = 1;

	if(upload_first && sys_status.status == SYS_STATUS_WIFI_STA_LOGIN ) {
	    upload_first = 0; 
	    SensorData_Upload("socket", &socket_state, 0);		
	} else if(sys_status.status != SYS_STATUS_WIFI_STA_LOGIN) {
	    upload_first = 1; 
	} 

	gpio10_val = BSP_GPIOGetValue(USER_GPIO_DIRECTCONFIG);
	if (gpio10_val == 0 ) {
		OSTimeDly(5);		/* delay 20ms filter button shake */
    	gpio10_val = BSP_GPIOGetValue(USER_GPIO_DIRECTCONFIG);
		if(gpio10_val == 0)
		{
			gpio30_val = ((NST_RD_GPIO_REG(SWPORTA_DR) & (1 << STRIP_GPIO_NUM_ONE))==0) ? 0 : 1;
			if(gpio30_val == 0) {
				BSP_GPIOSetValue(STRIP_GPIO_NUM_ONE, 1);
				BSP_GPIOSetValue(USER_GPIO_IDX_LED, 0);
				socket_state = 1;

			} else if(gpio30_val == 1) {
				BSP_GPIOSetValue(STRIP_GPIO_NUM_ONE, 0);
				BSP_GPIOSetValue(USER_GPIO_IDX_LED, 1);	
				socket_state = 0;			
			}
			OSTimeDly(2);
			/* Response the device status to cloud server */
			if(sys_status.status == SYS_STATUS_WIFI_STA_LOGIN)
		      SensorData_Upload("socket", &socket_state, 0);
		}
   }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  strip_stop
 *  Description:  Stop strip device control 
 *         Note:
 * =====================================================================================
 */
void strip_stop(void)
{
    BSP_GPIOSetValue(STRIP_GPIO_NUM_ONE, 0);
	BSP_GPIOSetValue(USER_GPIO_IDX_LED, 1);	
	socket_state = 0;
}		/* -----  end of function strip_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  strip_init
 *  Description:  Init smart strip 
 *         Note:
 * =====================================================================================
 */
void strip_init(void)
{
	/* Initialize the strip gpio with reg/green/blue/while */
	BSP_GPIOPinMux(STRIP_GPIO_NUM_ONE);		        /* Smart Socket */
	BSP_GPIOSetDir(STRIP_GPIO_NUM_ONE, 1);			/* output */
	BSP_GPIOSetValue(STRIP_GPIO_NUM_ONE, 0);	    /* low level */
	socket_state = 0;
}	

void Set_Strip(unsigned char state)
{
    BSP_GPIOSetValue(STRIP_GPIO_NUM_ONE, state);
	socket_state = state; //×´Ì¬
	if(socket_state == 0)
    	BSP_GPIOSetValue(USER_GPIO_IDX_LED, 1);	
	else 
	    BSP_GPIOSetValue(USER_GPIO_IDX_LED, 0);		    	   
}


void print_data(char *buf , UINT8 length)
{
    UINT8 i = 0;
	for(i=0; i<length; i++)
	{
	   BSP_UartPutcPolled(buf[i]);
	}
}




