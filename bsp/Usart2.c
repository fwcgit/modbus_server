#include "APP.h"

uint8_t rece_buff_2[USART2_RX_SIZE];
uint8_t sendBuff_2[USART_TX_SIZE];
uint16_t rx_index_2 = 0;
uint16_t data_len = 0;

static struct Usart_Config u_config={0};

void DMA0_Channel1_IRQHandler(void)
{

	if(dma_interrupt_flag_get(DMA0, DMA_CH1, DMA_INT_FLAG_FTF))
    {     
        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
        /* enable DMA channel3 */
		dma_channel_disable(DMA0, DMA_CH1);  

    }
	
}

void USART2_IRQHandler(void)                   
{
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE))   
    {
		usart_data_receive(USART2);
		usart_interrupt_flag_clear(USART2,USART_INT_FLAG_RBNE);
    }else if(RESET != usart_interrupt_flag_get(USART2,USART_INT_FLAG_IDLE))
	 {
		dma_channel_disable(DMA0, DMA_CH2);
        usart_data_receive(USART2);     // 非读不可，不读清不掉IDLE中断
		rx_index_2 = USART2_RX_SIZE - dma_transfer_number_get(DMA0, DMA_CH2);
		memset(rece_buff_2+rx_index_2,0,USART2_RX_SIZE - rx_index_2);
        dma_transfer_number_config(DMA0, DMA_CH2, USART2_RX_SIZE);
        dma_channel_enable(DMA0, DMA_CH2);
		usart_interrupt_flag_clear(USART2,USART_INT_FLAG_IDLE);
        if(NULL != u_config.cb)
           u_config.cb((char *)rece_buff_2,rx_index_2);
	 }

}

void Usart2_GPIO_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_10);
	gpio_init(GPIOB,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,GPIO_PIN_11);
	
}

void Usart2_DMA_Init(void)
{
	
	dma_parameter_struct dma_init_struct;
    
    rcu_periph_clock_enable(RCU_DMA0);
    
    /* deinitialize DMA channel4 (USART2 rx) */
    dma_deinit(DMA0, DMA_CH1);
    dma_init_struct.direction 		= DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr 	= (uint32_t)sendBuff_2;
    dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number 			= 0;
    dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART2);
    dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH1, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH1);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH1);
	
	dma_interrupt_enable(DMA0,DMA_CH1,DMA_INTF_FTFIF);
	nvic_irq_enable(DMA0_Channel1_IRQn,7,0);
	
}

void Usart2_Init(void)
{
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, u_config.baudrate);
    usart_word_length_set(USART2, u_config.word_len);
    usart_stop_bit_set(USART2, u_config.bit_stop);
    usart_parity_config(USART2, u_config.parity);
    usart_hardware_flow_rts_config(USART2, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART2, USART_CTS_DISABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
	
	usart_dma_transmit_config(USART2,USART_DENT_ENABLE);//使能DMA发送
	usart_dma_receive_config(USART2, USART_DENR_ENABLE);
	
	usart_interrupt_enable(USART2,USART_INT_FLAG_IDLE);
	//usart_interrupt_enable(USART2,USART_INT_FLAG_RBNE);//使能接收中断
	nvic_irq_enable(USART2_IRQn,0,0);
}

void Usart2_Receive_DMA(void)
{
	dma_parameter_struct dma_init_struct;
	/* deinitialize DMA channel4 (USART0 rx) */
	dma_deinit(DMA0, DMA_CH2);
	dma_init_struct.direction 		= DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_addr 	= (uint32_t)rece_buff_2;
	dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
	dma_init_struct.number 			= USART2_RX_SIZE;
	dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART2);
	dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
	dma_init(DMA0, DMA_CH2, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA0, DMA_CH2);
	/* enable DMA channel4 */
	dma_channel_enable(DMA0, DMA_CH2);
}

void Usart2_Config(struct Usart_Config *uc)
{
    memcpy(&u_config,uc,sizeof(struct Usart_Config));
    
	Usart2_GPIO_Config();
	Usart2_Init();
	Usart2_DMA_Init();
	Usart2_Receive_DMA();
}

void send_data(char *data,uint32_t len)
{
	if(dma_transfer_number_get(DMA0,DMA_CH1) != 0) return;
	memset(sendBuff_2,0,len);
	memcpy(sendBuff_2,data,len);
	dma_channel_disable(DMA0,DMA_CH1);
	dma_transfer_number_config(DMA0,DMA_CH1,len);
	dma_channel_enable(DMA0,DMA_CH1);
}

void Usart2_sendBytes(char *dat,uint32_t len,char isblock)
{
    if(isblock){
        for(int i = 0;i < len;i++){
            usart_data_transmit(USART2, *(dat+i));
            while (RESET == usart_flag_get(USART2, USART_FLAG_TBE));
        }
    }else{
        send_data(dat,len);
    }
}




