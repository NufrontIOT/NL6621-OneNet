#include "common.h"
#include "led_drv.h"
#include "uart.h"



/**********************************************/
/***   LED³ÌÐò   ***/

struct led_pwm_data led_data;


extern Agent_config_t Agent_config_data;


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  light_timer_irq_handle
 *  Description:  The handle of timer interrupt for light pwm output 
 *         Note:  Just called by TMR1_IRQHandler.
 * =====================================================================================
 */
void light_timer_irq_handle(void)
{
	int i;

    if (led_data.status_flag == 1) {
        /* restart timer1 */
        //BSP_Timer1Init(4);

        /* Check red/green/blue/white led */
		for (i = 0; i < LED_MAX_NUM; i++)
		{
			if (led_data.led_data.led_table[i].led_pwm > LED_MIN_BRIGHTNESS &&
	            led_data.led_data.led_table[i].led_pwm <= LED_MAX_BRIGHTNESS) 
			{
				if (led_data.timer_cnt == led_data.led_data.led_table[i].led_pwm &&
					led_data.timer_cnt != LED_MAX_BRIGHTNESS) 
				{
					BSP_GPIOSetValue(led_data.led_data.led_table[i].led_gpio, 0);
				}
				else if(led_data.timer_cnt == 0)
				{
					BSP_GPIOSetValue(led_data.led_data.led_table[i].led_gpio, 1);
				}	        
	        }
			else if(led_data.led_data.led_table[i].led_pwm == LED_MIN_BRIGHTNESS)
			{
				if(led_data.timer_cnt == 0)
				{
					BSP_GPIOSetValue(led_data.led_data.led_table[i].led_gpio, 0);
				}
			}	
		}

		if (led_data.timer_cnt >= LED_MAX_BRIGHTNESS)
		{
	    	led_data.timer_cnt = -1;
	    }
        
		led_data.timer_cnt++;
    }
}		/* -----  end of function light_timer_irq_handle  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_light_gpio_status
 *  Description:  Get the PWM value of the gpio 
 *         Note:
 * =====================================================================================
 */
unsigned char get_light_gpio_status(unsigned char gpio)
{
    if (gpio == LED_RED_GPIO_NUM) {
        return led_data.led_data.led_table[0].led_pwm;
    
    } else if (gpio == LED_GREEN_GPIO_NUM) {
        return led_data.led_data.led_table[1].led_pwm;
    
    } else if (gpio == LED_BLUE_GPIO_NUM) {
        return led_data.led_data.led_table[2].led_pwm;
    
    } else if (gpio == LED_WHITE_GPIO_NUM) {
        return led_data.led_data.led_table[3].led_pwm;
    }

    return 0;
}		/* -----  end of function get_light_gpio_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  light_data_parse
 *  Description:  Parse P0 data come from cloud server, if the bit is not set, reload the
 *  			current data to struct led_status structure.
 *   Parameters:
 *         Note:
 * =====================================================================================
 */
int light_data_parse(
		unsigned char action, 
		unsigned char attr_flags, 
        unsigned char *ctrl_data)
{
	led_status_flash led_Flash;
	Agent_config_t  pConfigData;

	NVIC_DisableIRQ(TMR1_IRQn);	 
   	*Tmr1Ctl = TMR_INT_MASK; 

    /* get the valid data in ctrl_data buffer */
	if ((attr_flags & 0x01) == 0x01) {        
        led_data.Switch = ctrl_data[0]& 0x01;
	} 

	if((attr_flags & 0x02) == 0x02){
	
		log_info("Factory reset.......\n");

		memcpy(&pConfigData, &Agent_config_data, sizeof(Agent_config_t));
		pConfigData.flag = 0;
		memset(pConfigData.wifi_ssid, 0x0, sizeof(pConfigData.wifi_ssid));
		memset(pConfigData.wifi_key, 0x0, sizeof(pConfigData.wifi_key));

		nl6621_SaveConfigData(&pConfigData);

		memset(&led_Flash, 0x0, sizeof(led_Flash));
	   	nl6621_SaveSmartLedData(&led_Flash);
		/* reset module */
		log_info("RESET button press more than 4 seconds, reseting System.\n");
		OSTimeDly(100);
		nl6621_Reset();
	}



	if ((attr_flags & 0x10) == 0x10) {        
        led_data.led_data.led_table[3].led_pwm = ctrl_data[1];
		log_info("Set WHITE: %d\n", led_data.led_data.led_table[3].led_pwm);
    }
    if ((attr_flags & 0x20) == 0x20) {         
        led_data.led_data.led_table[0].led_pwm = ctrl_data[2];
		log_info("Set RED: %d\n", led_data.led_data.led_table[0].led_pwm);
    }
    
    if ((attr_flags & 0x40) == 0x40) {        
        led_data.led_data.led_table[1].led_pwm = ctrl_data[3];
		log_info("Set GREEN: %d\n", led_data.led_data.led_table[1].led_pwm);
    }

    if ((attr_flags & 0x80) == 0x80) {        
        led_data.led_data.led_table[2].led_pwm = ctrl_data[4];
		log_info("Set BLUE: %d\n", led_data.led_data.led_table[2].led_pwm);
    }

 	led_Flash.led_r = get_light_gpio_status(LED_RED_GPIO_NUM);
 	led_Flash.led_g = get_light_gpio_status(LED_GREEN_GPIO_NUM);
 	led_Flash.led_b = get_light_gpio_status(LED_BLUE_GPIO_NUM);
 	led_Flash.led_w = get_light_gpio_status(LED_WHITE_GPIO_NUM);
	led_Flash.led_switch = led_data.Switch;


	 nl6621_SaveSmartLedData(&led_Flash);	

//	log_info("After data: action:0x%02x; attr_flags:0x%02x; WHITE:%d, RED:%d, GREEN:%d, BLUE:%d.\n", 
//			action, attr_flags,
//			led_data.led_data.led_table[3].led_pwm, 
//			led_data.led_data.led_table[0].led_pwm, 
//			led_data.led_data.led_table[1].led_pwm,	
//			led_data.led_data.led_table[2].led_pwm 
//			);

    return 0;
}		/* -----  end of function light_data_parse  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  light_start
 *  Description:  Start light PWM control. 
 *         Note:
 * =====================================================================================
 */
void light_start(unsigned char action, 
		unsigned char attr_flags, 
        unsigned char *ctrl_data)
{
    unsigned char i = 0;

    if (led_data.status_flag == 1) {
        log_info("Light PWM output is running.\n");	 
    }

	led_data.status_flag = 0;
    light_data_parse(action, attr_flags, ctrl_data);
    led_data.status_flag = 1;    


	if(led_data.Switch == 1) {  
		NVIC_EnableIRQ(TMR1_IRQn);
  		BSP_Timer1Init(4);
	} else {
	   	NSTusecDelay(20);
		for(i=0;i<4;i++)
	  	  BSP_GPIOSetValue(led_data.led_data.led_table[i].led_gpio, 0);	
	}


}		/* -----  end of function light_start  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  light_stop
 *  Description:  Stop light PWM output 
 *         Note:
 * =====================================================================================
 */
void light_stop(void)
{
	int i;
    for (i = 0; i < LED_MAX_NUM; i++) {
        BSP_GPIOSetValue(led_data.led_data.led_table[i].led_gpio, 0);
    }
    led_data.status_flag = 1;
}		/* -----  end of function light_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  light_init
 *  Description:  Init smart light 
 *         Note:
 * =====================================================================================
 */
void light_init(void)
{
    memset(&led_data, 0, sizeof(led_data));

	led_data.led_data.led_table[0].led_gpio = LED_RED_GPIO_NUM;
	led_data.led_data.led_table[1].led_gpio = LED_GREEN_GPIO_NUM;
	led_data.led_data.led_table[2].led_gpio = LED_BLUE_GPIO_NUM;
	led_data.led_data.led_table[3].led_gpio = LED_WHITE_GPIO_NUM;

	/* Initialize the Light gpio with reg/green/blue/while */
	BSP_GPIOPinMux(SOCKET_GPIO_NUM);		    /* Smart Socket */
	BSP_GPIOSetDir(SOCKET_GPIO_NUM, 1);			/* output */
	BSP_GPIOSetValue(SOCKET_GPIO_NUM, 0);	    /* low level */

	BSP_GPIOPinMux(LED_RED_GPIO_NUM);		    /* RED */
	BSP_GPIOSetDir(LED_RED_GPIO_NUM, 1);		/* output */
	BSP_GPIOSetValue(LED_RED_GPIO_NUM, 0);	    /* low level */

	BSP_GPIOPinMux(LED_GREEN_GPIO_NUM);			/* GREEN */
	BSP_GPIOSetDir(LED_GREEN_GPIO_NUM, 1);		/* output */
	BSP_GPIOSetValue(LED_GREEN_GPIO_NUM, 0);	/* low level */

	BSP_GPIOPinMux(LED_BLUE_GPIO_NUM);			/* BLUE */
	BSP_GPIOSetDir(LED_BLUE_GPIO_NUM, 1);		/* output */
	BSP_GPIOSetValue(LED_BLUE_GPIO_NUM, 0);	    /* low level */

	BSP_GPIOPinMux(LED_WHITE_GPIO_NUM);			/* WHITE */
	BSP_GPIOSetDir(LED_WHITE_GPIO_NUM, 1);		/* output */
	BSP_GPIOSetValue(LED_WHITE_GPIO_NUM, 0);	/* low level */

}		/* -----  end of function light_init  ----- */


void print_data(char *buf , UINT8 length)
{
//    UINT8 i = 0;
//	for(i=0; i<length; i++)
//	{
//	   BSP_UartPutcPolled(buf[i]);
//	}
}


