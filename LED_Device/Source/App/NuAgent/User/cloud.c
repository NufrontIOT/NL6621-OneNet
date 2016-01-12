/*
 * =====================================================================================
 *
 *       Filename:  cloud.c
 *
 *    Description:  cloud network 
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
#include "common.h"
#include "cloud.h"
#include "EdpKit.h"
#include "restful.h"
#include "onenet_comm.h"
#include "led_drv.h"


char * recv_buf = NULL;
char * send_buf = NULL;
CLOUD_CONN_VAL_G cloud_conn_status;



extern unsigned char TCPClient_reset_flag;
extern struct sys_status_t sys_status;
extern Onenet_config  OneNetdata;
extern int SensorData_Upload(char *dataPoint, void* value, unsigned char datatype);

unsigned char Attr_flags = 0;

#ifndef  Devid_RestFul

void recv_data_process(char * buffer, int len)
{
    int rtn;
    uint8 mtype;
    EdpPacket* pkg;

    //LOUIS add
	uint8 jsonorbin;
    char* src_devid;
    cJSON* save_json;
    char* save_json_str;

    cJSON* desc_json;
    char* desc_json_str;
    char* save_bin; 
    uint32 save_binlen;
    char* json_ack;


    char* push_data;
    uint32 push_datalen;

    char* cmdid;
    uint16 cmdid_len;
    char*  cmd_req;
    uint32 cmd_req_len;
    EdpPacket* send_pkg;
//    char* ds_id;
//    double dValue = 0;

    char* simple_str = NULL;
    char cmd_resp[] = "ok";
    unsigned cmd_resp_len = 0;

    RecvBuffer recvbuf;

	//EDP CMD
	char strtoint[30] = {0};
	unsigned char ctrl_data[5] = {0};


    recvbuf._data = (uint8*)buffer;
    recvbuf._write_pos = len;
    recvbuf._read_pos = 0;
    recvbuf._capacity = BUFFER_SIZE;

	log_info("Recv data(len:%d).\n", len);

    while (1)
    {
        /* ��ȡһ����ɵ�EDP�� */
        if ((pkg = GetEdpPacket(&recvbuf)) == 0)
        {
            log_info("need more bytes...\n");
            break;
        }

        /* ��ȡ���EDP������Ϣ���ͣ��������EDP������Ϣ����, �ֱ���EDP������ */
        mtype = EdpPacketType(pkg);
        switch(mtype)
        {
            case CONNRESP:
                /* ����EDP�� - ������Ӧ */
                rtn = UnpackConnectResp(pkg);
                log_info("Receive connect response, rtn: %d\n", rtn);
			
				/* ����Sensor Task���� */
			    sys_status.status = SYS_STATUS_WIFI_STA_LOGIN;
				cloud_conn_status.conn_status = CLOUD_CONN_DONE;
				
                break;

            case PINGRESP:
                /* ����EDP�� - ������Ӧ */
                UnpackPingResp(pkg); 
                log_info("recv ping resp\n");
                break;

            case PUSHDATA:
                /* ����EDP�� - ����ת�� */
                UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                log_info("recv push data, src_devid: %s, push_data: %s, len: %d\n", 
                        src_devid, push_data, push_datalen);
                free(src_devid);
                free(push_data);
                break;

            case SAVEDATA:
                /* ����EDP�� - ���ݴ洢 */
                 if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
                 {
                      if (jsonorbin == kTypeFullJson 
						    || jsonorbin == kTypeSimpleJsonWithoutTime
						    || jsonorbin == kTypeSimpleJsonWithTime) 
						    {
							     printf("json type is %d\n", jsonorbin);
					             /* ����EDP�� - json���ݴ洢 */ 
	                             UnpackSavedataJson(pkg, &save_json); 
	                             save_json_str = cJSON_Print(save_json);
	                             log_info("recv save data json, src_devid: %s, json: %s\n", 
	                                 src_devid, save_json_str); 
	                             free(save_json_str); 
	                             cJSON_Delete(save_json); 

								/* UnpackSavedataInt(jsonorbin, pkg, &ds_id, &iValue); */
								/* log_info("ds_id = %s\nvalue= %d\n", ds_id, iValue); */
				
								//UnpackSavedataDouble((SaveDataType)jsonorbin, pkg, &ds_id, &dValue);
								//log_info("ds_id = %s\nvalue = %f\n", ds_id, dValue);
				
								/* UnpackSavedataString(jsonorbin, pkg, &ds_id, &cValue); */
								/* log_info("ds_id = %s\nvalue = %s\n", ds_id, cValue); */
								/* free(cValue); */

								//free(ds_id);
                        }
		                else if (jsonorbin == kTypeBin)
		                {/* ����EDP�� - bin���ݴ洢 */
		                    UnpackSavedataBin(pkg, &desc_json, (uint8**)&save_bin, &save_binlen);
		                    desc_json_str=cJSON_Print(desc_json);
		                    log_info("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d\n", 
		                            src_devid, desc_json_str, save_bin, save_binlen);
		                    free(desc_json_str);
		                    cJSON_Delete(desc_json);
		                    free(save_bin);
		                }
		             	else if (jsonorbin == kTypeString){
						    UnpackSavedataSimpleString(pkg, &simple_str);
						    
						    log_info("%s\n", simple_str);
						    free(simple_str);
						}
		                free(src_devid);
	            	}else{
						printf("error\n");
					}

                    break;

            case SAVEACK:
				json_ack = NULL;
				UnpackSavedataAck(pkg, &json_ack);
				log_info("save json ack = %s\n", json_ack);
				free(json_ack);
				break;

            case CMDREQ: //�����͵ƿ���
			    log_info("recevice server cmd:\r\n");
				//dump_hex(pkg->_data, pkg->_write_pos);
			 	if (UnpackCmdReq(pkg, &cmdid, &cmdid_len, 	
							 &cmd_req, &cmd_req_len) == 0){

//						 log_info("cmdid:");
//						 print_data(cmdid,cmdid_len);
//						 log_info("\r\ncmreq:");
//                         print_data(cmd_req,cmd_req_len);
//						 log_info("\r\n");	

						 Attr_flags = 0;	 
						 memcpy((unsigned char *)strtoint,(unsigned char *)cmd_req,cmd_req_len);	


						 if(strtoint[0] == 'o') {  //����	  onoff
						 	 Attr_flags |= 0x01;
							 ctrl_data[0] = strtoint[5] - 0x30;
						 } else if(strtoint[0] == 'f'){  //��������	 factory
						 	 Attr_flags |= 0x02;
						 } else if(strtoint[0] == 'w'){ 	//��   white
						  	 Attr_flags |= 0x10;
							 ctrl_data[1] = my_atoi(&strtoint[5]);
						 } else if(cmd_req[0] == 'r'){  //��   red
						 	 Attr_flags |= 0x20;
							 ctrl_data[2] = my_atoi(&strtoint[3]);
						 } else if(cmd_req[0] == 'g'){  //��   green
                             Attr_flags |= 0x40;
							 ctrl_data[3] = my_atoi(&strtoint[5]);
						 } else if(cmd_req[0] == 'b'){  //��   blue
                             Attr_flags |= 0x80;
							 ctrl_data[4] = my_atoi(&strtoint[4]);						 
						 } 

						 if(Attr_flags)
						    light_start(0,Attr_flags,ctrl_data);
					    /* 
					     * �û������Լ������������أ���Ӧ��Ϣ�����Ϊ�գ��˴����践��2���ַ�"ok"��
					     * ���������Ҫ�ͷ�
					     */
					    cmd_resp_len = strlen(cmd_resp);
					    send_pkg = PacketCmdResp(cmdid, cmdid_len,
								     cmd_resp, cmd_resp_len);
			#ifdef _ENCRYPT
					    if (g_is_encrypt){
						SymmEncrypt(send_pkg);
					    }
			#endif
					    Socket_TCPClientSendData((char*)send_pkg->_data, send_pkg->_write_pos);
					    DeleteBuffer(&send_pkg);
					    OSTimeDly(5);


					    free(cmdid);
					    free(cmd_req);
					}
					break;

            default:
                /* δ֪��Ϣ���� */
                log_info("recv failed...\n");
                break;
        }
        DeleteBuffer(&pkg);
    }
}

#endif

int Agent_cloud_process(void)
{
	EdpPacket* connect_pkg = NULL;
	int ret;
	unsigned char failedcnt = 0;

	cloud_conn_status.conn_status = SOCK_DONE;

AGAIN:  

	/* ��Onenet����������EDP���ӱ��� */
	connect_pkg = PacketConnect1((char*)OneNetdata.src_devid, (char*)OneNetdata.api_key);

	/* dump package data */
	log_info("Packet connect data(len:%d):\n", connect_pkg->_write_pos);
	dump_hex(connect_pkg->_data, connect_pkg->_write_pos);
	log_info("SRC_DEVID:%s CLOUD_API_KEY:%s\r\n",OneNetdata.src_devid,OneNetdata.api_key);

	ret = Socket_TCPClientSendData((char *)connect_pkg->_data, connect_pkg->_write_pos);
	DeleteBuffer(&connect_pkg);
	if (ret < 0) {
		cloud_conn_status.conn_status = CLOUD_CONN_ERROR;
		log_info("Send connect data failed(ret:%d).\n", ret);
        failedcnt++;
        /* �������Ӵ�������3�����˳����� */
        if (failedcnt > 3) {
            Socket_TCPClientClose();
            return -1;
        }
        OSTimeDly(50);
		goto AGAIN;
	} else {
		log_notice("Send connect data success.\n");
	}



	/* ���������ڽ����ƶ˷����������� */
	while (1)
	{
		ret = Socket_TCPClientRecvData(recv_buf, CLOUD_RECV_BUFFER_SIZE);
		if (ret > 0) {
			recv_buf[ret] = '\0';
            recv_data_process(recv_buf, ret);
		}
		
		if (ret < 0 || TCPClient_reset_flag == 0xa5 ) {  //�ر�TCP����
			log_err("Recv tcp client data error(%d)\n", ret);
			Socket_TCPClientClose();
		   	sys_status.status = SYS_STATUS_WIFI_STA_CONNECTED;
			return -1;
		}
	}
}

void Agent_cloud_init(void)
{
	/* ����������ݺͷ������ݵ��ڴ�� */
	recv_buf = malloc(CLOUD_RECV_BUFFER_SIZE);
	send_buf = malloc(CLOUD_SEND_BUFFER_SIZE);
}


