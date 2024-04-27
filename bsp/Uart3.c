#include "APP.h"

uint8_t rece_buff_3[USART3_RX_SIZE];
uint8_t sendBuff_3[USART_TX_SIZE];
uint16_t rx_index_3 = 0;

static struct Usart_Config u_config={0};

void DMA1_Channel3_4_IRQHandler(void)
{
	if(dma_interrupt_flag_get(DMA1, DMA_CH4, DMA_INT_FLAG_FTF))
    {     
        dma_interrupt_flag_clear(DMA1, DMA_CH4, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA1, DMA_CH4, DMA_INT_FLAG_FTF);
        /* enable DMA channel3 */
		dma_channel_disable(DMA1, DMA_CH4);  

    }
}

void UART3_IRQHandler(void)                   
{
    if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE))   
    {
		usart_interrupt_flag_clear(UART3,USART_INT_FLAG_RBNE);
    }else if(RESET != usart_interrupt_flag_get(UART3,USART_INT_FLAG_IDLE))
	 {
		dma_channel_disable(DMA1, DMA_CH2);
        usart_data_receive(UART3);     // 非读不可，不读清不掉IDLE中断
		rx_index_3 = USART3_RX_SIZE - dma_transfer_number_get(DMA1, DMA_CH2);
		memset(rece_buff_3+rx_index_3,0,USART3_RX_SIZE - rx_index_3);
        dma_transfer_number_config(DMA1, DMA_CH2, USART3_RX_SIZE);
        dma_channel_enable(DMA1, DMA_CH2);
		usart_interrupt_flag_clear(UART3,USART_INT_FLAG_IDLE);
        if(NULL != u_config.cb)
            u_config.cb((char *)rece_buff_3,rx_index_3);
	 }

}

void Uart3_GPIO_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC,GPIO_MODE_AF_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_10);
	gpio_init(GPIOC,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,GPIO_PIN_11);
	
}

void Uart3_DMA_Init(void)
{
	
	dma_parameter_struct dma_init_struct;
    
    rcu_periph_clock_enable(RCU_DMA1);
    
    dma_deinit(DMA1, DMA_CH4);
    dma_init_struct.direction 		= DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr 	= (uint32_t)sendBuff_3;
    dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number 			= 0;
    dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(UART3);
    dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH4);
    /* enable DMA channel4 */
    dma_channel_enable(DMA1, DMA_CH4);
	
	dma_interrupt_enable(DMA1,DMA_CH4,DMA_INTF_FTFIF);
	nvic_irq_enable(DMA1_Channel3_Channel4_IRQn,0,7);
	
}

void Uart3_Init(void)
{
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_UART3);

    /* USART configure */
    usart_deinit(UART3);
    usart_baudrate_set(UART3, u_config.baudrate);
    usart_word_length_set(UART3, u_config.word_len);
    usart_stop_bit_set(UART3, u_config.bit_stop);
    usart_parity_config(UART3, u_config.parity);
    usart_hardware_flow_rts_config(UART3, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART3, USART_CTS_DISABLE);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_enable(UART3);
	
	usart_dma_transmit_config(UART3,USART_DENT_ENABLE);//使能DMA发送
	usart_dma_receive_config(UART3, USART_DENR_ENABLE);
	
	usart_interrupt_enable(UART3,USART_INT_FLAG_IDLE);
	//usart_interrupt_enable(UART3,USART_INT_FLAG_RBNE);//使能接收中断
	nvic_irq_enable(UART3_IRQn,7,0);
}

void Uart3_Receive_DMA(void)
{
	dma_parameter_struct dma_init_struct;
	/* deinitialize DMA channel4 (USART0 rx) */
	dma_deinit(DMA1, DMA_CH2);
	dma_init_struct.direction 		= DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_addr 	= (uint32_t)rece_buff_3;
	dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
	dma_init_struct.number 			= USART3_RX_SIZE;
	dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(UART3);
	dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
	dma_init(DMA1, DMA_CH2, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA1, DMA_CH2);
	/* enable DMA channel4 */
	dma_channel_enable(DMA1, DMA_CH2);
}

void Uart3_Config(struct Usart_Config *uc)
{
    memcpy(&u_config,uc,sizeof(struct Usart_Config));
    
	Uart3_GPIO_Config();
	Uart3_Init();
	Uart3_DMA_Init();
	Uart3_Receive_DMA();
}

void send_data_3(char *data,uint32_t len)
{
	if(dma_transfer_number_get(DMA1,DMA_CH4) != 0) return;
	memset(sendBuff_3,0,sizeof(sendBuff_3));
	memcpy(sendBuff_3,data,len);
	dma_channel_disable(DMA1,DMA_CH4);
	dma_transfer_number_config(DMA1,DMA_CH4,len);
	dma_channel_enable(DMA1,DMA_CH4);
}

void Uart3_sendBytes(char *dat,uint32_t len,char isblock)
{
    if(isblock){
        for(int i = 0;i < len;i++){
            usart_data_transmit(UART3, *(dat+i));
            while (RESET == usart_flag_get(UART3, USART_FLAG_TBE));
        }
    }else{
        send_data_3(dat,len);
    }
}




