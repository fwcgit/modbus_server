#ifndef MODBUS_H
#define MODBUS_H
#include "Usart.h"
struct modbus_port{
    enum Serial serial;
    int baudrate;
};

struct modbus_cmd{
    unsigned char devAddr;
    unsigned char fun;
    unsigned short startAddr;
    unsigned short len;
    unsigned short *data;
};

int modbus_write(struct modbus_port *mp,unsigned char deviceAddr,unsigned char func,unsigned short startAddr,unsigned short len,unsigned short *data);
int modbus_read(struct modbus_port *mp,unsigned char deviceAddr,unsigned char func,unsigned short startAddr,unsigned short len,unsigned short *data);
#endif
