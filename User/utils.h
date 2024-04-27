#ifndef _UTILS_H_
#define _UTILS_H_
#include "gd32f10x.h"
#include "AppConfig.h"
#include "Usart.h"
#include <stdlib.h>

uint8_t calc_code_sum(uint8_t *dat,uint16_t len);
uint8_t code_check(uint8_t *dat,uint16_t len);
void u_printf(char *format,...);
char isNumber(char *buff,unsigned short len);

char findCmdIndex(char **cmds,unsigned char cmds_len,char *cmd);

void * pvTaskMem_Malloc(size_t xWantedSize );

void vTaskMem_Free( void * pv );

void * pvTaskMem_MallocFromISR( size_t xWantedSize );

#endif
