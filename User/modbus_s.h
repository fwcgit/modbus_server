#ifndef MODBUS_S_H
#define MODBUS_S_H
#include "Usart.h"

struct Port
{
    unsigned int fd;
    unsigned int fd_id;
};

enum Port_type
{
    _RS232 = 0,_RS485,RS422,BT,ZIGBEE,WIFI,LAN,T4G
};

struct modbus_rt_data
{
    struct Port port;
    unsigned char *data;
    unsigned int len;
};

struct modbus_s_reg
{
    unsigned short *coilsInReg;
    unsigned short coilsInLen;
    unsigned short *coilsReg;
    unsigned short coilsLen;

    unsigned short *holdReg;
    unsigned short holdLen;
    unsigned short *inputReg;
    unsigned short inputLen;
};

struct Port_info
{
    struct Port port;
    enum Port_type port_type;
    unsigned int baudrate;
    unsigned char isIORT;
    unsigned char devAddr;
	unsigned char (*isRegCanWrite)(unsigned char fd,unsigned short addr);
    void (*sw_RT_io)(unsigned char fd,unsigned char val);
    void (*write_cb)(unsigned char fd,unsigned char func,unsigned short addr);
	void (*read_cb)(unsigned char fd,unsigned char func,unsigned short addr);
};

void modbus_s_init(void);
void modubs_s_config_port(struct Port_info *spi);
void modbus_s_init_reg(struct modbus_s_reg *reg);
void modbus_s_parser(struct Port *port,unsigned char *data,unsigned int len);
void send_version(struct Port *port);
#endif
