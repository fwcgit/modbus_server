#include "APP.h"

static char isSend = 0;
static enum Serial uart;
static int baudrate;


static char crc_check(char* data,int len)
{
    unsigned short crc = 0;
    unsigned short oldCrc = 0;
    if(len > 2)
    {
        crc = CRC16((unsigned char *)data,len-2);
        oldCrc = data[len-2]&0x00ff;
        oldCrc <<=8;
        oldCrc|=data[len-1];
        //logD("%x %x",crc,oldCrc);
        return crc == oldCrc;
    }
    return 0;
}

static char check_recv(char* o,short olen,char* d,short dlen)
{
    if(olen != dlen)return 1;
    for(int i = 0;i < olen;i++)
    {
        if(*(o+i) != *(d+i))return 2;
    }
    return 0;
}

static void printOldDataTfg(char *data,int len)
{
#if 0
    int i = 0;
    char *str = NULL;
    if(NULL != data && len > 0){
        str = (char*)pvPortMalloc(sizeof(char)*(len*2)+1);
        memset(str,0,len*2+1);
        for(i = 0; i < len; i++)
        {
            sprintf(str+i*2,"%02X ",*(data+i));
        }
        logd("printOldDatatfg %s", str);
    }
    if(NULL != str)
    {
        vPortFree(str);
        str = NULL;
    }

#endif
}


static char parser_data(unsigned short *reg,char *data,int len)
{
    unsigned short tmp = 0;
    if(NULL == reg || NULL == data) return 1;
    if(len % 2 != 0) return 1;
    for(int i = 0,j = 0;i < len;i+=2,j++)
    {
        tmp = data[i];
        tmp <<= 8;
        tmp|=data[i+1];
        reg[j] = tmp;
        //logD("%d",reg[j]);
    }
    return 0;
}

static int send_cmd(unsigned char *data,short len,char rw)
{
    int ret = 0;
    int t_len = len+2;
    char *cmd = (char *)pvTaskMem_Malloc(sizeof(char)*t_len);
    memcpy(cmd,data,len);
    unsigned short crc = CRC16((unsigned char*)cmd,len);
    cmd[t_len-2] = (char)(crc>>8);
    cmd[t_len-1] = (char)crc;
    usart_write(uart,cmd,t_len);
    printOldDataTfg(cmd,t_len);


    if(rw == 2)
    {
        vTaskDelay(50);
        char *buff = (char *)pvTaskMem_Malloc(sizeof(char)*t_len);;
        int rlen = usart_read(uart, buff, t_len);
       // logD("read");
        printOldDataTfg(buff,rlen);
        ret = check_recv(cmd,t_len,buff,rlen);
        if(buff)
            vTaskMem_Free(buff);
    }

    if(cmd)
        vTaskMem_Free(cmd);

    return ret;
}

int tel_modbus_write_hold(unsigned char dev_addr,unsigned short addr,unsigned short val)
{
    unsigned char data[6];
    data[0] = dev_addr;
    data[1] = 0x06;
    data[2] = (unsigned char)((addr >> 8) & 0x00ff);
    data[3] = (unsigned char)(addr & 0x00ff);
    data[4] = (unsigned char)((val >> 8) & 0x00ff);
    data[5] = (unsigned char)(val & 0x00ff);
    return send_cmd(data,6,2);
}

int tel_modbus_read_hold(unsigned char dev_addr,unsigned short addr,unsigned short len,unsigned short *reg)
{
    int t = len * 2 * 15 *(000.0f / (float)baudrate);

    unsigned char *data = (unsigned char *)pvPortMalloc(sizeof(char)*6);
    data[0] = dev_addr;
    data[1] = 0x03;
    data[2] = (char)(addr>>8);
    data[3] = (char)addr;
    data[4] = (char)(len>>8);
    data[5] = (char)len;
    send_cmd(data,6,1);
    if(data)
        vTaskMem_Free(data);

    vTaskDelay((t+50));
    char *buff = (char *) pvTaskMem_Malloc((len *2+20)*sizeof(char));
    int r_len = usart_read(uart,buff,len*2+20);
    //(buff,r_len);
    if(crc_check(buff,r_len)){
        parser_data(reg,buff+3,buff[2]);
        if(buff)
            vTaskMem_Free(buff);
        return 0;
    }else{
        if(buff)
            vTaskMem_Free(buff);
    }
    return 1;
}

/**
 * 读输入寄存器
 * @param devAddr 设备地址
 * @param regAddr 寄存器地址
 * @param dataLen 数据长度
 */
int tel_modbus_read_input(char devAddr,short startRegAddr,short dataLen,unsigned short *reg)
{
    int t = dataLen * 2 * 15 * (1000.0 / (float)baudrate);
    unsigned char *data = (unsigned char *)pvPortMalloc(sizeof(char)*6);
    data[0] = devAddr;
    data[1] = 0x04;
    data[2] = (char)(startRegAddr>>8);
    data[3] = (char)startRegAddr;
    data[4] = (char)(dataLen>>8);
    data[5] = (char)dataLen;
    send_cmd(data,6,1);
    if(data)
        vTaskMem_Free(data);

    vTaskDelay((t+50));
    char *buff = (char *) pvTaskMem_Malloc((dataLen *2+20)*sizeof(char));
    int r_len = usart_read(uart,buff,dataLen+10);
    if(crc_check(buff,r_len)){
        parser_data(reg,buff+3,buff[2]);
        if(buff)
            vTaskMem_Free(buff);
        return 0;
    }else{
        if(buff)
            vTaskMem_Free(buff);
    }
    return 1;
}

/**
 * 写保持寄存器
 * @param devAddr 设备地址
 * @param regAddr 寄存器地址
 * @param dataLen 数据长度
 * @param data 值
 */
int tel_modbus_write_mult_hold(unsigned char devAddr,unsigned short regAddr,unsigned short dataLen,unsigned short *_data)
{

    short t_len = 7+(dataLen * 2);
    unsigned char *data = (unsigned char *)pvPortMalloc(sizeof(char)*t_len);
    data[0] = devAddr;
    data[1] = 0x10;
    data[2] = (unsigned char)(regAddr>>8);
    data[3] = (unsigned char)regAddr;
    data[4] = (unsigned char)(dataLen>>8);
    data[5] = (unsigned char)dataLen;
    data[6] = (unsigned char)(dataLen*2);
    for(int i = 0;i < dataLen;i++){
        data[7+i*2] = (unsigned char)(*(_data+i) >> 8);
        data[7+(i*2+1)] = (unsigned char)*(_data+i);
    }
    int ret=send_cmd(data,t_len,2);
    if(data)
        vTaskMem_Free(data);

    return ret;
}

int modbus_write(struct modbus_port *mp,unsigned char deviceAddr,unsigned char func,unsigned short startAddr,unsigned short len,unsigned short *data)
{
    if(NULL == data)
        return 1;

    uart = mp->serial;
    baudrate = mp->baudrate;
    
    if(func == 0x06)
        return tel_modbus_write_hold(deviceAddr,startAddr,data[0]);
    else if(func == 0x10)
        return tel_modbus_write_mult_hold(deviceAddr,startAddr,len,data);

    return 0;
}

int modbus_read(struct modbus_port *mp,unsigned char deviceAddr,unsigned char func,unsigned short startAddr,unsigned short len,unsigned short *data)
{
    if(NULL == data)
        return 1; 

    uart = mp->serial;
    baudrate = mp->baudrate;
    
    if(func == 0x03)
        return tel_modbus_read_hold(deviceAddr,startAddr,len,data);
    else if(func == 0x04)
        return tel_modbus_read_input(deviceAddr,startAddr,len,data);

    return 0;
}
