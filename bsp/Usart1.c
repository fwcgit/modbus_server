#include "APP.h"
uint16_t rx_index_1 = 0;
uint8_t rece_buff_1[USART1_RX_SIZE];
uint8_t sendBuff_1[USART_TX_SIZE];
//static struct Usart_Config u_config={0};
static void (*cb)(char *buff,int len) = NULL;

void DMA0_Channel6_IRQHandler(void)
{

	if(dma_interrupt_flag_get(DMA0, DMA_CH6, DMA_INT_FLAG_FTF))
    {     
        dma_interrupt_flag_clear(DMA0, DMA_CH6, DMA_INT_FLAG_G);
        dma_interrupt_flag_clear(DMA0, DMA_CH6, DMA_INT_FLAG_FTF);
        /* enable DMA channel3 */
		dma_channel_disable(DMA0, DMA_CH6);  

    }

}

void USART1_IRQHandler(void)
{
	uint8_t dat;
	 if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE))
	 {
		dat = usart_data_receive(USART1);
		 if(rx_index_1 >= USART1_RX_SIZE){
			rx_index_1 = 0;
		 }
		 rece_buff_1[rx_index_1++] = dat;
		usart_interrupt_flag_clear(USART1,USART_INT_FLAG_RBNE);
	 }
	 else if(RESET != usart_interrupt_flag_get(USART1,USART_INT_FLAG_IDLE))
	 {
		//Usart1_Receive();

		dma_channel_disable(DMA0, DMA_CH5);
        usart_data_receive(USART1);     // 非读不可，不读清不掉IDLE中断
		rx_index_1 = USART1_RX_SIZE - dma_transfer_number_get(DMA0, DMA_CH5);
		memset(rece_buff_1+rx_index_1,0,USART1_RX_SIZE - rx_index_1);
        dma_transfer_number_config(DMA0, DMA_CH5, USART1_RX_SIZE);
        dma_channel_enable(DMA0, DMA_CH5);
		usart_interrupt_flag_clear(USART1,USART_INT_FLAG_IDLE);
        if(NULL != cb)
            cb((char *)rece_buff_1,rx_index_1);
	 }
}

void Usart1_GPIO_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_2);
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,GPIO_PIN_3);
	
}

void Usart1_DMA_Init(void)
{
	
	dma_parameter_struct dma_init_struct;
    
    rcu_periph_clock_enable(RCU_DMA0);
    
    /* deinitialize DMA channel4 (USART0 rx) */
    dma_deinit(DMA0, DMA_CH6);
    dma_init_struct.direction 		= DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr		= (uint32_t)sendBuff_1;
    dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number 			= 0;
    dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART1);
    dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH6, &dma_init_struct);
	
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH6);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH6);
	
	dma_interrupt_enable(DMA0,DMA_CH6,DMA_INTF_FTFIF);
	nvic_irq_enable(DMA0_Channel6_IRQn,0,3);
	
}

void Usart1_Init(struct Usart_Config *uc)
{
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1,uc->baudrate);
    usart_word_length_set(USART1, uc->word_len);
    usart_stop_bit_set(USART1, uc->bit_stop);
    usart_parity_config(USART1, uc->parity);
    usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);
	
	usart_dma_transmit_config(USART1,USART_DENT_ENABLE);//使能DMA发送
	usart_dma_receive_config(USART1, USART_DENR_ENABLE);
	
	usart_interrupt_enable(USART1,USART_INT_FLAG_IDLE);
	//usart_interrupt_enable(USART1,USART_INT_FLAG_RBNE);//使能接收中断
	nvic_irq_enable(USART1_IRQn,7,0);
}

void Usart1_Receive_DMA(void)
{
	dma_parameter_struct dma_init_struct;
	/* deinitialize DMA channel4 (USART0 rx) */
	dma_deinit(DMA0, DMA_CH5);
	dma_init_struct.direction 		= DMA_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_addr 	= (uint32_t)rece_buff_1;
	dma_init_struct.memory_inc 		= DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width 	= DMA_MEMORY_WIDTH_8BIT;
	dma_init_struct.number 			= USART1_RX_SIZE;
	dma_init_struct.periph_addr 	= (uint32_t)&USART_DATA(USART1);
	dma_init_struct.periph_inc 		= DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width 	= DMA_PERIPHERAL_WIDTH_8BIT;
	dma_init_struct.priority 		= DMA_PRIORITY_ULTRA_HIGH;
	dma_init(DMA0, DMA_CH5, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA0, DMA_CH5);
	/* enable DMA channel4 */
	dma_channel_enable(DMA0, DMA_CH5);
}

void Usart1_Config(struct Usart_Config *uc)
{
    //memcpy(&u_config,uc,sizeof(struct Usart_Config));

	Usart1_GPIO_Config();
    
    if(NULL != uc){
        cb = uc->cb;
        Usart1_Init(uc);
    }
    
	Usart1_DMA_Init();
	Usart1_Receive_DMA();
}

void Usart1_sendBytes(char *dat,uint32_t len,char isblock)
{
    if(isblock){
        for(int i = 0;i < len;i++){
            usart_data_transmit(USART1, *(dat+i));
            while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        }
    }else{
        if(dma_transfer_number_get(DMA0,DMA_CH6) != 0) return;
        memset(sendBuff_1,0,sizeof(sendBuff_1));
        memcpy(sendBuff_1,dat,len);
        dma_channel_disable(DMA0,DMA_CH6);
        dma_transfer_number_config(DMA0,DMA_CH6,len);
        dma_channel_enable(DMA0,DMA_CH6);
    }
}
