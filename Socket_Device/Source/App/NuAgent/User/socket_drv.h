#ifndef __SOCKET_DRV_H__
#define __SOCKET_DRV_H__


extern int  socket_state;




//ÖÇÄÜ²å×ù
void strip_init(void);
void strip_stop(void);
void check_socket(void);
void Set_Strip(unsigned char state);
void print_data(char *buf , UINT8 length);



#endif


