#include "Usart.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

uint8_t *recv[]		= {rece_buff_0,rece_buff_1,rece_buff_2,rece_buff_3,rece_buff_4};
uint16_t *recv_len[]= {&rx_index_0,&rx_index_1,&rx_index_2,&rx_index_3,&rx_index_4};

void (*writes[])(char *dat,uint32_t len,char isblock)	={Usart0_sendBytes,Usart1_sendBytes,Usart2_sendBytes,Uart3_sendBytes,Uart4_sendBytes};
void (*u_configs[])(struct Usart_Config *uc)			={Usart0_Config,Usart1_Config,Usart2_Config,Uart3_Config,Uart4_Config};

static unsigned int USART_P[]={USART0,USART1,USART2,UART3,UART4};

int usart_read(enum Serial serial,char *buff,int len)
{
    int r_len = 0;
    if(NULL == buff)
        return 0;
    
	if(serial < 0 || serial >= 5)
	   return 0;
    
    if(len <= 0)
        return 0;
    
    if(*recv_len[serial] <= 0)
        return 0;
   
    if(len >= *recv_len[serial])
    {
        memcpy(buff,recv[serial],*recv_len[serial]);
        r_len = *recv_len[serial];
    }
    else    
    {
        memcpy(buff,recv[serial],len);
        r_len = len;
    }

    memset(recv[serial],0,*recv_len[serial]);
    *recv_len[serial] = 0;
    return r_len;
}


int usart_write_block(enum Serial serial,char *buff,int len)
{
	if(len > USART_TX_SIZE)
	   return 0;
	if(NULL == buff)
	   return 0;
	if(serial < 0 || serial >= 5)
	   return 0;
   
	writes[serial](buff,len,1);
	
	return len;
}

int usart_write(enum Serial serial,char *buff,int len)
{
	if(len > USART_TX_SIZE)
	   return 0;
	if(NULL == buff)
	   return 0;

	if(serial < 0 || serial >= 5)
	   return 0;

	writes[serial](buff,len,0);
	return len;
}


int usart_init(enum Serial serial,struct Usart_Config *uc)
{
	if(serial < 0 || serial >= 5)
	   return 1;  
	
	u_configs[serial](uc);
	return 0;
}
void usart_clear_buff(enum Serial serial,int len)
{
    if(serial > 3)
        return;
    
    memset(recv[serial],0,len);
    *recv_len[serial] = 0;
}

/***
SET 1�������
**/
unsigned char isSendStatus(enum Serial serial)
{
    return usart_flag_get(USART_P[serial], USART_FLAG_TC);
}
