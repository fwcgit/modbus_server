#ifndef USART_H_
#define USART_H_
#include "gd32f10x.h"
extern uint16_t rx_index_0;
extern uint16_t rx_index_1;
extern uint16_t rx_index_2;
extern uint16_t rx_index_3;
extern uint16_t rx_index_4;

extern uint8_t rece_buff_0[];
extern uint8_t rece_buff_1[];
extern uint8_t rece_buff_2[];
extern uint8_t rece_buff_3[];
extern uint8_t rece_buff_4[];

#define USART0_RX_SIZE 256
#define USART1_RX_SIZE 1024
#define USART2_RX_SIZE 1024
#define USART3_RX_SIZE 1024
#define UART4_RX_SIZE 256
#define USART_TX_SIZE 1024

#define BIT_STOP_1      USART_STB_1BIT
#define BIT_STOP_1_5    USART_STB_1_5BIT
#define BIT_STOP_2      USART_STB_2BIT

#define PARITY_NONE USART_PM_NONE
#define PARITY_EVEN USART_PM_EVEN
#define PARITY_ODD USART_PM_ODD
#define WORD_LEN_8 USART_WL_8BIT
#define WORD_LEN_9 USART_WL_9BIT

enum Serial
{
    usart0 = 0,usart1,usart2,uart3,uart4
};

struct Usart_Config
{
    int baudrate;
    int parity;
    int bit_stop;
    int word_len;
    void (*cb)(char *buff,int len);
};

void Usart0_Config(struct Usart_Config *uc);

void Usart1_Config(struct Usart_Config *uc);

void Usart2_Config(struct Usart_Config *uc);

void Uart3_Config(struct Usart_Config *uc);

void Uart4_Config(struct Usart_Config *uc);


void Usart0_sendBytes(char *dat,uint32_t len,char isblock);

void Usart1_sendBytes(char *dat,uint32_t len,char isblock);

void Usart2_sendBytes(char *dat,uint32_t len,char isblock);

void Uart3_sendBytes(char *dat,uint32_t len,char isblock);

void Uart4_sendBytes(char *dat,uint32_t len,char isblock);

int usart_init(enum Serial serial,struct Usart_Config *uc);

int usart_read(enum Serial serial,char *buff,int len);

int usart_write(enum Serial serial,char *buff,int len);

int usart_write_block(enum Serial serial,char *buff,int len);

void usart_clear_buff(enum Serial serial,int len);

unsigned char isSendStatus(enum Serial serial);

#endif
