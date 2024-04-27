#include "APP.h"

uint8_t rece_buff_4[UART4_RX_SIZE];
uint16_t rx_index_4 = 0;

static struct Usart_Config u_config={0};

void UART4_IRQHandler(void)                   
{
    if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE))   
    {
        uint8_t dat = usart_data_receive(UART4);
        if(rx_index_4 >= UART4_RX_SIZE){
            rx_index_4 = 0;
        }
        rece_buff_4[rx_index_4++] = dat;
		usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
    }

}

void Uart4_GPIO_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
	gpio_init(GPIOC,GPIO_MODE_AF_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_12);
	gpio_init(GPIOD,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_10MHZ,GPIO_PIN_2);
	
}

void UART4_Init(void)
{
	/* enable USART clock */
    rcu_periph_clock_enable(RCU_UART4);

    /* USART configure */
    usart_deinit(UART4);
    usart_baudrate_set(UART4, u_config.baudrate);
    usart_word_length_set(UART4, u_config.word_len);
    usart_stop_bit_set(UART4, u_config.bit_stop);
    usart_parity_config(UART4, u_config.parity);
    usart_hardware_flow_rts_config(UART4, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(UART4, USART_CTS_DISABLE);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
    usart_enable(UART4);
	
	//usart_dma_transmit_config(UART4,USART_DENT_ENABLE);//使能DMA发送
	//usart_dma_receive_config(UART4, USART_DENR_ENABLE);
	
	//usart_interrupt_enable(UART4,USART_INT_FLAG_IDLE);
	usart_interrupt_enable(UART4,USART_INT_FLAG_RBNE);//使能接收中断
	nvic_irq_enable(UART4_IRQn,7,0);
}



void Uart4_Config(struct Usart_Config *uc)
{
    memcpy(&u_config,uc,sizeof(struct Usart_Config));
    Uart4_GPIO_Config();
	UART4_Init();
}

void Uart4_sendBytes(char *dat,uint32_t len,char isblock)
{
	for(int i = 0;i < len;i++){
        usart_data_transmit(UART4, *(dat+i));
        while (RESET == usart_flag_get(UART4, USART_FLAG_TBE));
    }
}




