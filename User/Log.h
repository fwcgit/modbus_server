#ifndef LOG_H_
#define LOG_H_
#include "Usart.h"
void log_init(enum Serial serial);
void logd(char *format,...);
#endif
