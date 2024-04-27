#include "APP.h"
uint8_t calc_code_sum(uint8_t *dat,uint16_t len)
{
	uint16_t i;
	uint32_t sum = 0;
	
	for(i = 0 ; i < len ; i++)
	{
		sum = sum+*(dat+i);
	}
	
	return (uint8_t)sum;
}

uint8_t code_check(uint8_t *dat,uint16_t len)
{
	if(calc_code_sum(dat,len) == *(dat+(len)))
	{
		return 1;
	}
	
	return 0;
}


char isNumber(char *buff,unsigned short len)
{
    for(short i = 0;i < len;i++)
        if(buff[i] < '0' && buff[i] > '9')
            return 0;
        
   return 1;     
}

char findCmdIndex(char **cmds,unsigned char cmds_len,char *cmd)
{
	if(NULL == cmds) return -1;
	
	for(char i = 0;i< cmds_len;i++)
	{
		if(strcmp(*(cmds+i),cmd) == 0)
			return i;
	}
	
	return -1;
}	

void * pvTaskMem_Malloc( size_t xWantedSize )
{
    void * pvReturn = NULL;
    
    portENTER_CRITICAL();
    pvReturn = pvPortMalloc(xWantedSize);
    portEXIT_CRITICAL();
    
    return pvReturn;
}


void vTaskMem_Free( void * pv )
{
    portENTER_CRITICAL();
    vPortFree(pv);
    portEXIT_CRITICAL();
}

void * pvTaskMem_MallocFromISR( size_t xWantedSize )
{
    void * pvReturn = NULL;
    uint32_t uiBasepri;
    
    uiBasepri = portSET_INTERRUPT_MASK_FROM_ISR();
    pvReturn = pvPortMalloc(xWantedSize);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uiBasepri);
    
    return pvReturn;
}


