#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include "AppConfig.h"
static unsigned int USART_P[]	={USART0,USART1,USART2,UART3,UART4};
static enum Serial uart 	= uart4;;
void log_init(enum Serial serial)
{
    uart = serial;
    struct Usart_Config uc;
    uc.baudrate = 115200;
    uc.bit_stop = BIT_STOP_1;
    uc.parity 	= PARITY_NONE;
    uc.word_len = WORD_LEN_8;
    uc.cb 		= NULL;
    usart_init(serial,&uc);

}

void logd(char *format,...)
{
#ifdef LOGD_OUT
   va_list args;
   
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
#endif
}

int fputc(int ch,FILE *f)
{  
    #ifdef LOGD_OUT
    usart_data_transmit(USART_P[uart], (uint8_t)ch);
    while (RESET == usart_flag_get(USART_P[uart], USART_FLAG_TBE));
    #endif
    return ch;  
}