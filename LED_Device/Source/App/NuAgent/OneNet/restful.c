#include "common.h"
#include "restful.h"
#include "onenet_comm.h"
#include "cJSON.h"



unsigned char new_device = 0;
char DeviceID[20] = {0};




/*
以下只提供设备添加，查询，删除功能。
*/

#ifdef 	 Devid_RestFul

	//新增设备
	static int HttpPOST_NewDevice(void)
	{
	    int ret=0;
	    char *postBuf=NULL;
	    char *Content = "{\"title\":\"6211662F4E2D56FD4EBA\"}\r\n";
	    int ContentLen=0;
	
	    postBuf = (char*)OSMMalloc(400);
	    if (postBuf==NULL) return 1;
	
	    ContentLen = strlen(Content);  

	    sprintf(postBuf,"POST /devices HTTP/1.1\r\napi-key:%s\r\nHost:%s\r\nContent-Length:%d\r\n\r\n%s",
		        HTTP_API_KEY, RESTFUL_SERVER_DOMAIN,ContentLen,Content);
	
	    log_info("%s", postBuf);   
		 
        ret = Socket_TCPClientSendData(postBuf, strlen(postBuf));
		
	    if (ret <= 0) {
	        log_info("HTTP POST transmit data failed(ret:%d)\n", ret);
	    } else {
	        log_notice("HTTP POST transmit data success(ret:%d)\n",ret);
	    }

	    OSMFree(postBuf);    
	
	    return 0;         
	}
	
	//查询设备
	static int HttpGET_Device(char* deviceid)
	{
	    int ret=0;
	    char *postBuf=NULL;
	
	    postBuf = (char*)OSMMalloc(400);
	    if (postBuf==NULL) return 1;
	
	
	    sprintf(postBuf,"GET /devices/%s HTTP/1.1\r\napi-key:%s\r\nHost:%s\r\n\r\n",
		        deviceid,HTTP_API_KEY, RESTFUL_SERVER_DOMAIN);
	
	    log_info("%s", postBuf);   
		 
        ret = Socket_TCPClientSendData(postBuf, strlen( postBuf ));
		
	    if (ret <= 0) {
	        log_info("HTTP GET transmit data failed(ret:%d).\n", ret);
	    } else {
	        log_notice("HTTP GET transmit data success.\n");
	    }
		  
	    OSMFree( postBuf );    
	
	    return 0;         
	
	}
	
	//删除设备
	static int HttpDELETE_Device(char* deviceid)
	{
	    int ret=0;
	    char *postBuf=NULL;
	
	    postBuf = (char*)OSMMalloc(400);
	    if (postBuf==NULL) return 1;
	
	
	    sprintf(postBuf,"DELETE /devices/%s HTTP/1.1\r\napi-key:%s\r\nHost:%s\r\n\r\n",
		        deviceid,HTTP_API_KEY, RESTFUL_SERVER_DOMAIN);
	
	    log_info("%s", postBuf);   

        ret = Socket_TCPClientSendData(postBuf, strlen( postBuf ));
		
	    if (ret <= 0) {
	        log_info("HTTP Delete transmit data failed(ret:%d).\n", ret);
	    } else {
	        log_notice("HTTP Delete transmit data success.\n");
	    }			 
	  
	    OSMFree( postBuf );    
	
	    return 0;         
	
	}

	//循环测试
	int RestFul_Run(void)
	{
	    HttpPOST_NewDevice();//新建设备，
        OSTimeDly(500);

        if(new_device == 1)
        {
            new_device = 0;
            HttpGET_Device(DeviceID);//查询设备  
            OSTimeDly(500);

            HttpDELETE_Device(DeviceID);//删除设备
            OSTimeDly(500);             
        }
	    return 0;
	}

	//json处理数据
    int RestFul_RecvProcess(char *data, unsigned int length)
	{
	    cJSON *json , *json_value , *json_string,*json_Object;  
	    int presult;
	
	    //判断是否收到HTTP OK
    	presult = Search_str(data , "200 OK\r\n");
        if( presult != NULL ){
		 	presult = Search_str(data , "{\"errno\"");
	
			if(presult != NULL ){
			    json = cJSON_Parse(&data[presult-9]);  
			    if (!json){  
			        log_info("Error before: [%s]\n",cJSON_GetErrorPtr());  
			    } else { 
                    // 解析时间戳  
                    json_value = cJSON_GetObjectItem( json , "errno");  
                    if( json_value->type == cJSON_Number )  { 

					   if(json_value->valueint == 0){//解析是数值

						  json_Object = cJSON_GetObjectItem( json , "data"); 

						  if(json_Object->type == cJSON_Object){   //解析是对象

						  	 json_string = 	cJSON_GetObjectItem( json_Object , "device_id"); 

							 if(json_string->type == cJSON_String)
							 {
							     log_info("device_id: %s\r\n",json_string->valuestring);
								 memcpy((unsigned char*)DeviceID,(unsigned char*)json_string->valuestring,strlen(json_string->valuestring));//拷贝设备ID号
								 new_device = 1;
							 }
						  }     					   
					   } 
                    }  
                    // 释放内存空间  
                    cJSON_Delete(json);     		
				}
			}
	    } else {
	        log_info("Http Response Error\r\n");
	        log_info("%s",data);
			return 1;
	    }
		return 0;	
	}

#endif















