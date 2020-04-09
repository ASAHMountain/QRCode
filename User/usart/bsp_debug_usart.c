#include "./usart/bsp_debug_usart.h"

unsigned int  uart_data_len = 0;     //���ڴ��������ݳ���
unsigned int  uart_data_index = 0;   //�����ѷ������ݸ���
unsigned char uart_send_state= 0; //����״̬��1��ʾ���ڷ��ͣ�0��ʾ����
unsigned char uart_tx_buf[UART_MAX_BUF_SIZE] = {0};//���ڷ������ݻ�����

UART_HandleTypeDef UartHandle;
//extern uint8_t ucTemp;  
 /**
  * @brief  DEBUG_USART GPIO ����,����ģʽ���á�115200 8-N-1
  * @param  ��
  * @retval ��
  */  
void DEBUG_USART_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
      
  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();
 
  /* ʹ�� UART ʱ�� */
  DEBUG_USART_CLK_ENABLE();

  /**USART1 GPIO Configuration    
  PA9     ------> USART2_TX
  PA10    ------> USART2_RX 
  */
  /* ����Tx����Ϊ���ù���  */
  GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
  HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);
  
  /* ����Rx����Ϊ���ù��� */
  GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
  GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
  HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct); 
  
  UartHandle.Instance          = DEBUG_USART;
  
  UartHandle.Init.BaudRate     = DEBUG_USART_BAUDRATE;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  HAL_UART_Init(&UartHandle);

  /*����1�жϳ�ʼ�� */
  HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 0, 0);
  HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
  /*���ô��ڽ����ж� */
  __HAL_UART_ENABLE_IT(&UartHandle,UART_IT_RXNE);  
}

 /**
  * @brief  ��ȡ���ڷ���״̬
  * @param  ��
  * @retval 1��ʾ���ڷ��ͣ�0��ʾ����
  */
uint8_t get_send_sta()
{
	if(uart_send_state)
		return 1;
	return 0;
}
 /**
  * @brief  ������д��USART1���ͻ�����
  * @param  dat����ָ�룬len���ݳ���
  * @retval 0��ʾд��ɹ���1��ʾд��ʧ��
  */
uint8_t uart_send_buf(unsigned char *dat, unsigned int len)
{
	unsigned char addr = 0;
	
	if(uart_send_state)
		return 1;
	
	uart_data_len = len;
	uart_data_index = 0;
	uart_send_state = 1;
	
	for(; len > 0; len--)
		uart_tx_buf[addr++] = *(dat++);
	
	__HAL_UART_ENABLE_IT(&UartHandle,UART_IT_TXE);
	return 0;
}
 /**
  * @brief  USART1�����ж���Ӧ����
  * @param  
  * @retval 
  */
void USART1_IRQHandler(void)
{
	//�����ж�
    if (__HAL_UART_GET_FLAG(&UartHandle,USART_FLAG_TXE) != RESET)
    {
        if(uart_data_index < uart_data_len)
		{
			HAL_UART_Transmit(&UartHandle, (uint8_t *)&uart_tx_buf[uart_data_index++], 1, 1000);
		}
		else
		{
			uart_send_state = 0;
			__HAL_UART_DISABLE_IT(&UartHandle,UART_IT_TXE);
		}
				
		__HAL_UART_CLEAR_FLAG(&UartHandle, USART_FLAG_TXE);
    }
}
/*****************  �����ַ��� **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, uint8_t *str)
{
	unsigned int k=0;
  do 
  {
      HAL_UART_Transmit( &UartHandle,(uint8_t *)(str + k) ,1,1000);
      k++;
  } while(*(str + k)!='\0');
  
}
///�ض���c�⺯��printf������DEBUG_USART���ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
	/* ����һ���ֽ����ݵ�����DEBUG_USART */
	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	
	return (ch);
}

///�ض���c�⺯��scanf������DEBUG_USART����д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		
	int ch;
	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	return (ch);
}