#include "APP.h"
#define BAUDRATE 115200
#define DEVICE_ADDR 0x01
unsigned short holdReg[HOLD_REG_SIZE]		= {0};
unsigned short inputReg[INPUT_REG_SIZE]		= {0};
unsigned short coilReg[COIL_REG_SIZE]		= {0};
unsigned short coilInReg[COIL_IN_REG_SIZE]	= {0};

/**
串口数据回调调用modebus数据解析
**/
void rs232_callback(char *data,int len)
{
	logd("rs232_callback:%d\r\n",len);
	#ifdef MODEBUS
		struct Port port;
		port.fd = uart3;
		port.fd_id = 3;
		modbus_s_parser(&port,(unsigned char*)data,(unsigned int)len);
	#endif
}

/**
串口数据回调调用modebus数据解析
**/
void rs485_callback(char *data,int len)
{
	logd("rs485_callback:%d\r\n",len);
	#ifdef MODEBUS
		struct Port port;
		port.fd = usart0;
		port.fd_id = 0;
		modbus_s_parser(&port,(unsigned char*)data,(unsigned int)len);
	#endif
}

/**
串口数据回调调用modebus数据解析
**/
void mod4G_callback(char *data,int len)
{
	logd("mod4G_callback:%d %s\r\n",len,data);	
	#ifdef MODEBUS
		struct Port port;
		port.fd = usart1;
		port.fd_id = 1;
		modbus_s_parser(&port,(unsigned char*)data,(unsigned int)len);
	#endif
}

/**
485芯片读写方向回调
fd 串口描述符
val 1控制IO口输出1 0控制IO输出0
**/
void modbus_send_rtio(unsigned char fd,unsigned char val)
{
    logd("modbus_send_rtio:%d\r\n",val);
}

/**
写回调
fd 串口描述符
func 功能码
addr 寄存器地址
**/
void modbus_write_cb(unsigned char fd,unsigned char func,unsigned short addr)
{
    logd("modbus_write_cb:%X\r\n",func);
}

/**
读回调
fd 串口描述符
func 功能码
addr 寄存器地址
**/
void modbus_read_cb(unsigned char fd,unsigned char func,unsigned short addr)
{
    logd("modbus_read_cb:%X\r\n",func);

}

/**
fd 串口描述符
addr 寄存器地址
返回1可写 0不可写
**/
unsigned char isRegRegCanWrite(unsigned char fd,unsigned short addr)
{
	if(fd == usart1)
	{
		if(addr == 1 || addr == 7)
			return 0;
	}
	
	if(fd == usart0)
		return 1;
	
	return 0;
}



void tel_config(void)
{
	
	#ifdef MODEBUS
		struct modbus_s_reg msr = {0};
	
		modbus_s_init(); 
		msr.coilsReg = NULL;
		msr.coilsLen = 0;

		msr.coilsInReg = NULL;
		msr.coilsInLen = 0;

		msr.holdReg = holdReg;
		msr.holdLen = HOLD_REG_SIZE;

		msr.inputReg = inputReg;
		msr.inputLen = INPUT_REG_SIZE;
		modbus_s_init_reg(&msr);//初始化寄存器
	
		struct Port_info pi;
		struct Port port;
		
		#ifdef RS485_MODEBUS
		
			port.fd=usart0;
			port.fd_id=1;

			pi.port 		= port;
			pi.port_type 	= _RS485;
			pi.isIORT 		= 1;//是否IO控制收发
			pi.devAddr 		= DEVICE_ADDR;
			pi.sw_RT_io 	= modbus_send_rtio;
			pi.isRegCanWrite= isRegRegCanWrite;
			pi.write_cb 	= modbus_write_cb;
			pi.read_cb		= modbus_read_cb;
			pi.baudrate		= BAUDRATE;
			modubs_s_config_port(&pi);//初始化通信口
			
			
			struct Usart_Config uc_0;
			uc_0.baudrate	= BAUDRATE;
			uc_0.bit_stop 	= BIT_STOP_1;
			uc_0.parity 	= PARITY_NONE;
			uc_0.word_len 	= WORD_LEN_8;
			uc_0.cb 		= rs232_callback;
			usart_init(usart0,&uc_0);//485
		#endif

		
		#ifdef RS232_MODEBUS
			port.fd=uart3;
			port.fd_id=0;

			pi.port 		= port;
			pi.port_type 	= _RS232;
			pi.isIORT 		= 0;//是否IO控制收发
			pi.devAddr 		= DEVICE_ADDR;
			pi.sw_RT_io 	= NULL;
			pi.isRegCanWrite= isRegRegCanWrite;
			pi.write_cb 	= modbus_write_cb;
			pi.read_cb		= modbus_read_cb;
			pi.baudrate		= BAUDRATE;
			modubs_s_config_port(&pi);//初始化通信口
			
			
			struct Usart_Config uc_3;
			uc_3.baudrate	= BAUDRATE;
			uc_3.bit_stop 	= BIT_STOP_1;
			uc_3.parity 	= PARITY_NONE;
			uc_3.word_len 	= WORD_LEN_8;
			uc_3.cb 		= NULL;
			usart_init(uart3,&uc_3);//232
		#endif

	#endif


    struct Usart_Config uc_1;
    uc_1.baudrate	=115200;
    uc_1.bit_stop 	= BIT_STOP_1;
    uc_1.parity 	= PARITY_NONE;
    uc_1.word_len 	= WORD_LEN_8;
    uc_1.cb 		= mod4G_callback;
    usart_init(usart1,&uc_1);//4g
    

}


void comm_sysinit(void)
{
    
	memset(holdReg,0,HOLD_REG_SIZE);
	memset(inputReg,0,INPUT_REG_SIZE);
	memset(coilReg,0,COIL_REG_SIZE);
	memset(coilInReg,0,COIL_IN_REG_SIZE);
	
	systick_config();
	utimer_init(24);

	#ifdef LOGD_OUT
		log_init(uart4);
	#endif
	
}

void led_Task(void *params)
{   

    while(1)
    {
       // led_heart();
        vTaskDelay(1000);
		//battery_get_vol();
        //logd("led_Task\r\n");
		#ifdef DEBUG
			logd("xPortGetFreeHeapSize:%d\r\n",xPortGetFreeHeapSize());
			logd("led_Task uxTaskGetStackHighWaterMark:%d\r\n",uxTaskGetStackHighWaterMark(NULL));
		#endif				
    }
    
}


int main(void)
{

   
	comm_sysinit();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	
    logd("modbus server boot ....\r\n");

    xTaskCreate(led_Task,"led_Task",256,NULL,FC_TASK_PRIORITY_NORMAL,NULL);
	
    vTaskStartScheduler();
    
	while(1)
	{

       
	}
}



