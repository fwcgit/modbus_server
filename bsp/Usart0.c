#include "APP.h"
uint16_t rx_index_0 = 0;
uint8_t rece_buff_0[USART0_RX_SIZE];
uint8_t sendBuff_0[USART0_RX_SIZE];

static struct Usart_Config u_config={0};

void DMA0_Channel3_IRQHandler(void)
{

	if(dma_interrupt_flag_get(DMA0, DMA_CH3, DMA_INT_FLAG_FTF))
    {     
        dma_interrupt_flag_clear(DMA0, DMA_CH3, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA0, DMA_CH3, DMA_INT_FLAG_FTF);
		dma_channel_disable(DMA0, DMA_CH3);  

    }

}

void USART0_IRQHandler(void)
{

	uint8_t dat;
	 if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE))
	 {
		 dat = usart_data_receive(USART0);
		 if(rx_index_0 >= USART0_RX_SIZE){
			rx_index_0 = 0;
		 }
		 rece_buff_0[rx_index_0++] = dat;
		usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RBNE);
	 }else if(RESET != usart_interrupt_flag_get(USART0,USART_INT_FLAG_IDLE))
	 {
		//Usart0_Receive();
		dma_channel_disable(DMA0, DMA_CH4);
        usart_data_receive(USART0);     // �Ƕ����ɣ������岻��IDLE�ж�
		rx_index_0 = USART0_RX_SIZE - dma_transfer_number_get(DMA0, DMA_CH4);
		memset(rece_buff_0+rx_index_0,0,USART0_RX_SIZE - rx_index_0);
        dma_transfer_number_config(DMA0, DMA_CH4, USART0_RX_SIZE);
        dma_channel_enable(DMA0, DMA_CH4);
		usart_interrupt_flag_clear(USART0,USART_INT_FLAG_IDLE);
		if(NULL != u_config.cb)
            u_config.cb((char *)rece_buff_0,rx_index_0);
	 }

}

void Usart0_GPIO_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_9);
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,GPIO_PIN_10);
	
}

void Usart0_DMA_Init(void)
{
	
	dma_parameter_struct dma_init_struct;
    
    rcu_periph_clock_enable(RCU_DMA0);
    
    /* deinitialize DMA channel4 (USART0 rx) */
    dma_deinit(DMA0, DMA_CH3);
    dma_init_struct.direction 		= DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr 	= (uint32_t)sendBuff_0;
    dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number 			= 0;
    dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART0);
    dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH3, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH3);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH3);

	dma_interrupt_enable(DMA0,DMA_CH3,DMA_INTF_FTFIF);
	nvic_irq_enable(DMA0_Channel3_IRQn,0,0);
	
}

void Usart0_Init(void)
{
	
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, u_config.baudrate);
    usart_word_length_set(USART0, u_config.word_len);
    usart_stop_bit_set(USART0, u_config.bit_stop);
    usart_parity_config(USART0, u_config.parity);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
	
	usart_dma_transmit_config(USART0,USART_DENT_ENABLE);//ʹ��DMA����
    usart_dma_receive_config(USART0, USART_DENR_ENABLE);
	
	usart_interrupt_enable(USART0,USART_INT_FLAG_IDLE);
	//usart_interrupt_enable(USART0,USART_INT_FLAG_RBNE);//ʹ�ܽ����ж�
	nvic_irq_enable(USART0_IRQn,7,4);
}

void Usart0_Receive_DMA(void)
{
	dma_parameter_struct dma_init_struct;
	/* deinitialize DMA channel4 (USART0 rx) */
	dma_deinit(DMA0, DMA_CH4);
	dma_init_struct.direction 		= DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_addr 	= (uint32_t)rece_buff_0;
	dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT; 
	dma_init_struct.number 			= USART0_RX_SIZE;
	dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART0);
	dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
	dma_init(DMA0, DMA_CH4, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA0, DMA_CH4);
	/* enable DMA channel4 */
	dma_channel_enable(DMA0, DMA_CH4);

}

void Usart0_Config(struct Usart_Config *uc)
{
    memcpy(&u_config,uc,sizeof(struct Usart_Config));
    
	Usart0_GPIO_Config();
	Usart0_Init();
	Usart0_DMA_Init();
	Usart0_Receive_DMA();
}

void Usart0_sendBytes(char *dat,uint32_t len,char isblock)
{
    if(isblock){
    	for(int i = 0;i < len;i++){
            usart_data_transmit(USART0, *(dat+i));
            while (RESET == usart_flag_get(USART0, USART_FLAG_TBE));
        }
    }else{
    	if(dma_transfer_number_get(DMA0,DMA_CH3) != 0) return;
        memset(sendBuff_0,0,sizeof(sendBuff_0));
        memcpy(sendBuff_0,dat,len);
        dma_channel_disable(DMA0,DMA_CH3);
        dma_transfer_number_config(DMA0,DMA_CH3,len);
        dma_channel_enable(DMA0,DMA_CH3);
    }

}
