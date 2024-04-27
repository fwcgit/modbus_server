#include "modbus_s.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "crc.h"
#include "string.h"
#include "stdlib.h"
#include "AppConfig.h"
#include "Log.h"
#include "systick.h"
#include "utils.h"

static struct modbus_s_reg *regs = NULL;
static QueueHandle_t xQueueSend     = NULL;
static QueueHandle_t xQueueRecv     = NULL;
static unsigned char queue_send_Size = 5;
static unsigned char queue_recv_Size = 5;

static struct Port_info spis[T4G+1]={0};

static unsigned char isVerify(unsigned char *data,unsigned int len)
{
    if(len < 2) return 0;
    unsigned short oldCrc = *(data+(len-2)) & 0x00ff;
    oldCrc <<= 8 ;
    oldCrc |= (*(data+(len-1))&0x00ff);

    unsigned short crc = CRC16(data,len-2);
    
    return (unsigned char)(oldCrc == crc);
}

static void rsp_old_data(struct modbus_rt_data *mrt)
{
        if(uxQueueMessagesWaiting(xQueueSend) >= queue_send_Size)return;
        struct modbus_rt_data mrt_;
        mrt_.port = mrt->port;
        mrt_.len = mrt->len;
        mrt_.data = (unsigned char *)pvTaskMem_Malloc(sizeof(char)*mrt->len);
        memcpy(mrt_.data,mrt->data,mrt->len);

        xQueueSendToBack(xQueueSend,&mrt_,0);
}

static void rsp_write_comm(struct Port port,unsigned char fun,unsigned short startAddr,unsigned short val)
{
        if(uxQueueMessagesWaiting(xQueueSend) >= queue_send_Size)return;
        struct modbus_rt_data mrt;
        unsigned short rspLen = 8;
        mrt.port = port;
        mrt.len = rspLen;
        mrt.data = (unsigned char *)pvTaskMem_Malloc(sizeof(char)*rspLen);

        mrt.data[0] = spis[port.fd_id].devAddr;
        mrt.data[1] = fun;
        mrt.data[2] = (unsigned char)((startAddr >> 8) & 0x00ff);
        mrt.data[3] = (unsigned char)((startAddr >> 0) & 0x00ff);
        mrt.data[4] = (unsigned char)((val >> 8) & 0x00ff);
        mrt.data[5] = (unsigned char)((val >> 0) & 0x00ff);
        unsigned short crc = CRC16(mrt.data,rspLen-2);
        mrt.data[rspLen-2] = (unsigned char)(crc >> 8&0x00ff);
        mrt.data[rspLen-1] = (unsigned char)(crc&0x00ff);

        xQueueSendToBack(xQueueSend,&mrt,0);
}

static void rsp_reg(struct Port port,unsigned char func,unsigned short *reg,unsigned short startAddr,unsigned short len)
{
        if(uxQueueMessagesWaiting(xQueueSend) >= queue_send_Size)return;
        struct modbus_rt_data mrt;
        unsigned short rspLen = len*2+5;
        mrt.port = port;
        mrt.len = rspLen;
        mrt.data = (unsigned char *)pvTaskMem_Malloc(sizeof(char)*rspLen);

        mrt.data[0] = spis[port.fd_id].devAddr;
        mrt.data[1] = func;
        mrt.data[2] = (unsigned char)(len *2);
       
        for(unsigned short i =startAddr,j = 0;i < (startAddr+len);i++,j++)
        {
            mrt.data[j*2+3] = (unsigned char)((reg[i] >> 8) & 0x00ff);
            mrt.data[j*2+4] = (unsigned char)(reg[i] & 0x00ff);
        }

        unsigned short crc = CRC16(mrt.data,rspLen-2);
        mrt.data[rspLen-2] = (unsigned char)(crc >> 8&0x00ff);
        mrt.data[rspLen-1] = (unsigned char)(crc&0x00ff);

        xQueueSendToBack(xQueueSend,&mrt,0);
}

static void rsp_coil(struct Port port,unsigned char func,unsigned short *reg,unsigned short startAddr,unsigned short coilCount)
{
        if(uxQueueMessagesWaiting(xQueueSend) >= queue_send_Size)return;
        struct modbus_rt_data mrt;
        unsigned char byteCount = coilCount / 8 + (coilCount % 8 > 0 ? 1:0);
        unsigned short rspLen = byteCount+5;
        unsigned char startIndex = startAddr / 16;
        unsigned short *reg_p = (unsigned short*)reg;
        reg_p += startIndex;
	    startAddr = startAddr - startIndex * 16;

        unsigned short chindex = 0;
        unsigned char ch = 0;
        unsigned short outSize = 0;
        unsigned short tmp = 0;

        mrt.port = port;
        mrt.len = rspLen;
        mrt.data = (unsigned char *)pvTaskMem_Malloc(sizeof(char)*rspLen);

        mrt.data[0] = spis[port.fd_id].devAddr;
        mrt.data[1] = func;
        mrt.data[2] = byteCount;

        for (int i = 0; i < startAddr + coilCount; i++) {
            if (i % 16 == 0) {
                tmp = *(reg_p++);
            }
            if (i >= startAddr) {
                ch >>= 1;
                if ((tmp & 0x8000) == 0x8000) {
                    ch |= 0x80;
                }
                else {
                    ch &= ~0x80;
                }
                if (chindex > 0 & chindex % 7 == 0) {
                    mrt.data[3+outSize++] = ch;
                    ch = 0;
                    chindex = 0;
                }
                else {
                    chindex++;
                }
            }
            tmp <<= 1;
        }

        for (char i = coilCount; i < byteCount*8; i++) {
            ch >>= 1;
            ch &= ~0x80;
        }
        mrt.data[3+outSize++] = ch;

        unsigned short crc = CRC16(mrt.data,rspLen-2);
        mrt.data[rspLen-2] = (unsigned char)(crc >> 8&0x00ff);
        mrt.data[rspLen-1] = (unsigned char)(crc >> 0&0x00ff);

        xQueueSendToBack(xQueueSend,&mrt,0);
}

static void read_coil(unsigned short *reg,unsigned short coilsLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;

    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short coilCount = *(mrt->data+4) & 0x00ff;
    coilCount <<= 8;
    coilCount |= (*(mrt->data+5) & 0x00ff);
    
    if(startAddr+coilCount > coilsLen){
        
    }else{ 
        rsp_coil(mrt->port,*(mrt->data+1),reg,startAddr,coilCount);
    }

}

static void read_reg(unsigned short *reg,unsigned short regLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;

    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short regCount = *(mrt->data+4) & 0x00ff;
    regCount <<= 8;
    regCount |= (*(mrt->data+5) & 0x00ff);

    if(startAddr+regCount > regLen){
        
    }else{ 
        rsp_reg(mrt->port,*(mrt->data+1),reg,startAddr,regCount);
    }
}

static unsigned short getRegAddr(unsigned short offset,int count)
{
	unsigned short i = count / 16 ;
	return i+offset;
}

static unsigned short getRegVal(unsigned short offset,unsigned short *reg,int count) 
{
	int i = count / 16;
	return reg[i+offset];
}

static void write_muli_coil(unsigned short *reg,unsigned short coilsLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;
 
    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short regCount = *(mrt->data+4) & 0x00ff;
    regCount <<= 8;
    regCount |= (*(mrt->data+5) & 0x00ff);

    if(startAddr+regCount > coilsLen){
        return;
    }
    unsigned char byteCount = *(mrt->data+6);
    unsigned char *byteData = mrt->data+7;
    unsigned char byte = 0;
    unsigned short regVal = 0;
    unsigned short regOffset = 0;
    unsigned short cut = startAddr % 16;
    unsigned short regIndex = startAddr / 16;    
	unsigned short regCnt = 0;
    unsigned short offset = cut;

	for (int i = 0; i < byteCount; i++) {
		byte = byteData[i];
		
		if (i % 2 == 0) {
			regOffset = 0;
		}
		
		for (int j = 0; j < 8; j++) {
			regVal = getRegVal(regIndex,reg,regCnt + cut);
			if ((byte & 0x01) == 0x01) {
				regVal |= (1 << (15 - regOffset++ - offset));
			}
			else {
				regVal &= ~(1 << (15 - regOffset++ - offset));
			}
			byte >>= 1;
			reg[getRegAddr(regIndex,regCnt + cut)] = regVal;
			regCnt++;
			if (((regCnt+cut) % 16) == 0) {
				regOffset = 0;
				offset = 0;
			}
		}		
	}

    rsp_write_comm(mrt->port,0x0F,startAddr,regCount);
    if(NULL != spis[mrt->port.fd_id].write_cb){
            spis[mrt->port.fd_id].write_cb(mrt->port.fd,*(mrt->data+1),startAddr);
    }
}

static void write_coil(unsigned short *reg,unsigned short coilsLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;

    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short val = *(mrt->data+4) & 0x00ff;
    val <<= 8;
    val |= (*(mrt->data+5) & 0x00ff);
    
    if(startAddr >= coilsLen){
        
    }else{ 
        unsigned char startIndex = startAddr / 16;
        unsigned short s = reg[startIndex];
        unsigned char offset = startAddr - startIndex * 16;
        if(val == 0xff00)
            s |= 1 << (15-offset);
        else
            s &= ~(1<<(15-offset));

        reg[startIndex] = s;
        rsp_write_comm(mrt->port,0x05,startAddr,val);
        if(NULL != spis[mrt->port.fd_id].write_cb){
            spis[mrt->port.fd_id].write_cb(mrt->port.fd,*(mrt->data+1),startAddr);
        }
    }
}

static void write_hold(unsigned short *reg,unsigned short regLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;

    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short val = *(mrt->data+4) & 0x00ff;
    val <<= 8;
    val |= (*(mrt->data+5) & 0x00ff);

    if(startAddr >= regLen){
        
    }else{

		if(NULL != spis[mrt->port.fd_id].isRegCanWrite){
			if(spis[mrt->port.fd_id].isRegCanWrite(mrt->port.fd,startAddr))
			{
				reg[startAddr] = val;
			}
        }
		else
			reg[startAddr] = val;
		
		
        rsp_write_comm(mrt->port,*(mrt->data+1),startAddr,val);
        if(NULL != spis[mrt->port.fd_id].write_cb){
            spis[mrt->port.fd_id].write_cb(mrt->port.fd,*(mrt->data+1),startAddr);
        }
    }
}

static void write_muli_hold(unsigned short *reg,unsigned short regLen,struct modbus_rt_data *mrt)
{
    if(NULL == reg) return;

    unsigned short startAddr = *(mrt->data+2) & 0x00ff;
    startAddr <<= 8;
    startAddr |=(*(mrt->data+3)&0x00ff);

    unsigned short len = *(mrt->data+4) & 0x00ff;
    len <<= 8;
    len |= (*(mrt->data+5) & 0x00ff);

    unsigned char byteLen = *(mrt->data+6);

    if((startAddr+len) >= regLen){
        
    }else{

        unsigned short tmp = 0;
        for(int i = 0,j = 0;i < byteLen;i+=2,j++)
        {
            tmp = *(mrt->data+(i+7)) & 0x00ff;
            tmp<<=8;
            tmp|=*(mrt->data+(i+1+7));
            
			if(NULL != spis[mrt->port.fd_id].isRegCanWrite){
				if(spis[mrt->port.fd_id].isRegCanWrite(mrt->port.fd,startAddr))
				{
					reg[startAddr+j] = tmp;
				}
			}
			else
				reg[startAddr+j] = tmp;
        }
        rsp_write_comm(mrt->port,*(mrt->data+1),startAddr,len);
        if(NULL != spis[mrt->port.fd_id].write_cb){
            spis[mrt->port.fd_id].write_cb(mrt->port.fd,*(mrt->data+1),startAddr);
        }
    }
}

static void parser_data_cmd(struct modbus_rt_data *mrt)
{
    
    unsigned char fun  = mrt->data[1];
    //logd("parser_data_cmd:%d %d\r\n",mrt->len,fun);
	
	if(fun == 0x01 || fun == 0x02 || fun == 0x03 || fun == 0x04)
	{
		if(NULL != spis[mrt->port.fd_id].read_cb){
            spis[mrt->port.fd_id].read_cb(mrt->port.fd,*(mrt->data+1),mrt->data[2]);
        }
	}
	
    if(fun == 0x01){//读线圈
        read_coil(regs->coilsReg,regs->coilsLen,mrt);
    }else if(fun == 0x02){//读取输入状态
        read_coil(regs->coilsInReg,regs->coilsInLen,mrt);
    }else if(fun == 0x03){//读保持寄存器
        read_reg(regs->holdReg,regs->holdLen,mrt);
    }else if(fun == 0x04){//读输入寄存器
        read_reg(regs->inputReg,regs->inputLen,mrt);
    }else if(fun == 0x05){//写单线圈
        write_coil(regs->coilsReg,regs->coilsLen,mrt);
    }else if(fun == 0x06){//写单保持寄器
        write_hold(regs->holdReg,regs->holdLen,mrt);
    }else if(fun == 0x0F){//写多个线圈
        write_muli_coil(regs->coilsReg,regs->coilsLen,mrt);
    }else if(fun == 0x10){//写多个保持寄存器
        write_muli_hold(regs->holdReg,regs->holdLen,mrt);
    }
}

static unsigned char isUart(enum Port_type port_type)
{
   return (port_type == _RS485 || port_type == RS422 || port_type == _RS232);
}

static void send_cmd(struct modbus_rt_data *mrt)
{
    struct Port port = mrt->port;
    struct Port_info spi = spis[port.fd_id];

    if(spi.isIORT){
        if(NULL != spi.sw_RT_io)
                if(NULL != spi.sw_RT_io){
                    spi.sw_RT_io(mrt->port.fd,1);
                }
                vTaskDelay(pdMS_TO_TICKS(5));
            if(isUart(spi.port_type)){
                usart_write(mrt->port.fd,(char*)mrt->data,mrt->len);
                unsigned int t = mrt->len * 15 * (1000.0 / (float)spi.baudrate);
                vTaskDelay(pdMS_TO_TICKS(t+5));
                if(NULL != spi.sw_RT_io){
                    spi.sw_RT_io(mrt->port.fd,0);
                }
            } 
    }else{
        usart_write(mrt->port.fd,(char*)mrt->data,mrt->len);
    }
}

void modbus_s_recv_Task(void *parame)
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5);
    while(1)
    {     
        struct modbus_rt_data mrt;
        
        if(uxQueueMessagesWaiting(xQueueRecv) != 0){
           
            xStatus = xQueueReceive(xQueueRecv,&mrt,xTicksToWait);
            if(xStatus == pdPASS){
                
                if(NULL != mrt.data){
                    parser_data_cmd(&mrt);
                    vTaskMem_Free(mrt.data);
                    mrt.data = NULL;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }else{
            vTaskDelay(pdMS_TO_TICKS(20));
        }        
    }
}

void modbus_s_send_Task(void *parame)
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5);
    while(1)
    {     
        struct modbus_rt_data mrt;
        if(uxQueueMessagesWaiting(xQueueSend) != 0){
            xStatus = xQueueReceive(xQueueSend,&mrt,xTicksToWait);
            if(xStatus == pdPASS){
                
                if(NULL != mrt.data){
                    send_cmd(&mrt);
                    vTaskMem_Free(mrt.data);
                    mrt.data = NULL;
                } 
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }else{
            vTaskDelay(pdMS_TO_TICKS(20));
        }        
    }
}

void modbus_s_init(void)
{
    xQueueSend = xQueueCreate(queue_send_Size,sizeof(struct modbus_rt_data));
    xQueueRecv = xQueueCreate(queue_recv_Size,sizeof(struct modbus_rt_data));

   xTaskCreate(modbus_s_recv_Task,"modbus_s_recv_Task",256,NULL,FC_TASK_PRIORITY_HIGH1,NULL);
   xTaskCreate(modbus_s_send_Task,"modbus_s_send_Task",256,NULL,FC_TASK_PRIORITY_HIGH,NULL);
}

void modbus_s_init_reg(struct modbus_s_reg *reg)
{
    regs = reg;
}

void modubs_s_config_port(struct Port_info *spi)
{
    if(NULL == spi) return;
    if(spi->port.fd_id > T4G) return;
    memcpy(&spis[spi->port.fd_id],spi,sizeof(struct Port_info));
}

void modbus_s_parser(struct Port *port,unsigned char *data,unsigned int len)
{    
    if(isVerify(data,len)){
        //logd("isVerify:success\r\n");
        if(spis[port->fd_id].devAddr == data[0]){
             //logd("uxQueueMessagesWaiting=%d\r\n",uxQueueMessagesWaiting(xQueueRecv));
            if(uxQueueMessagesWaiting(xQueueRecv) >= queue_recv_Size)return;
            struct modbus_rt_data mrt;
            memcpy(&mrt.port,port,sizeof(struct Port));
            mrt.data = (unsigned char *)pvTaskMem_MallocFromISR(sizeof(char)*len);
            mrt.len = len;
            if(mrt.data != NULL){
                memcpy(mrt.data,data,len);
                //logd("memcpy:success\r\n");
                xQueueSendToBackFromISR(xQueueRecv,&mrt,0);
            }
        }
    }else{
        logd("isVerify:fail\r\n");
    }
}

