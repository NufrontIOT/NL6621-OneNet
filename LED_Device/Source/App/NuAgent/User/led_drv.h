#ifndef __LED_DRV_H__
#define __LED_DRV_H__



#define LED_MAX_NUM  				(4)
#define LED_MIN_BRIGHTNESS			(0)
#define LED_MAX_BRIGHTNESS			(255)

#define SOCKET_GPIO_NUM				(3)

#define LED_RED_GPIO_NUM			(11)
#define LED_GREEN_GPIO_NUM			(30)
#define LED_BLUE_GPIO_NUM			(12)
#define LED_WHITE_GPIO_NUM			(9)

#pragma pack(1)

typedef struct {
	unsigned char led_gpio;                  /* the pwm output gpio */ 
	unsigned char led_pwm;                  /* limit: 0 to 255 percent */ 
} led_status_t;

struct led_status {
	led_status_t led_table[LED_MAX_NUM]; 
};

typedef struct{
	unsigned char	led_w;
    unsigned char	led_r;
    unsigned char	led_g;
    unsigned char	led_b;
	unsigned char	led_switch; 
}__attribute__((packed))led_status_flash;

struct led_pwm_data {
	unsigned char Switch;
    struct led_status led_data;
	unsigned char status_flag;		        /* 0:stop; 1:start */
    int timer_cnt;     			            /* store the count of current timer */
};

#pragma pack()


extern struct led_pwm_data led_data;

void light_init(void);
void light_stop(void);
void light_start(unsigned char action, 
		unsigned char attr_flags, 
        unsigned char *ctrl_data);
int light_data_parse(
		unsigned char action, 
		unsigned char attr_flags, 
        unsigned char *ctrl_data);
unsigned char get_light_gpio_status(unsigned char gpio);
void light_timer_irq_handle(void);



void print_data(char *buf , UINT8 length);

#endif


