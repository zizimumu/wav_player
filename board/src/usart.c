#include "usart.h"
#include "stm32f4xx_dma.h"

#include <stdio.h>
#include <string.h>

#define MAX_SEND_LEN 1024

static unsigned char USART1_DMA_Buf[MAX_SEND_LEN];
static unsigned int g_DMASendLenTotal_Next = 0;
static unsigned int g_DMASendLenTotal = 0;



void COM1Init(u32 BaudRate)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	USART_InitTypeDef USART_InitStructure;
  
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);    
  	USART_InitStructure.USART_BaudRate = BaudRate;
  	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure.USART_Parity = USART_Parity_No;
  	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
  	USART_Init(USART1, &USART_InitStructure);

	

	
  	USART_Cmd(USART1, ENABLE);

	

	
  	USART_ClearFlag(USART1, USART_FLAG_TC);
}
void  COM1_DMA_Init(void )
{
	 

	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                             //��DMAʱ��
    DMA_DeInit(DMA2_Stream7);                                               //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);                //ԴͷBUF ���� (&(USART1->DR)) 
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART1_DMA_Buf;                //Ŀ��BUF ����Ҫд���ĸ�������֮��
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                        //������Դͷ//��������Ϊ���ݴ����Ŀ�ĵػ�����Դ
    DMA_InitStructure.DMA_BufferSize = 0;                                        //DMA����Ĵ�С ��λ���±��趨
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;               //�����ַ�Ĵ���������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                        //�ڴ��ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;        //�����ֽ�Ϊ��λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;            //�ڴ��ֽ�Ϊ��λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //��������������ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                            //4���ȼ�֮һ��(������) VeryHigh/High/Medium/Low

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;		   
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;


	DMA_Init(DMA2_Stream7, &DMA_InitStructure);                                    //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��1�Ĵ���

	USART_Cmd(USART1, DISABLE);
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); 
	USART_Cmd(USART1, ENABLE);
	
}
void STM32_dma_Print(char *pBuf)
{
	unsigned int len = 0;
	if( 0 == pBuf)
		return ;
	len = strlen(pBuf);

	USART_DMACmd(USART1,USART_DMAReq_Tx,DISABLE);
	USART_Cmd(USART1, DISABLE);
	DMA_Cmd(DMA2_Stream7, DISABLE);


	if( 0 == DMA2_Stream7->NDTR )  // DMA complete
	{
		strcpy((char *)USART1_DMA_Buf,(char *)pBuf);
		DMA2_Stream7->NDTR = len;
		g_DMASendLenTotal = len;
		
		DMA2_Stream7->M0AR = (u32)USART1_DMA_Buf;
		//g_DMASendLenTotal_Next= 0;
		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
		USART_Cmd(USART1, ENABLE);

		DMA2->HIFCR = ( (DMA_FLAG_TCIF7|DMA_FLAG_HTIF7 )& 0x0F7D0F7D);//must clear the flag
		DMA_Cmd(DMA2_Stream7, ENABLE);
	}
	else
	{
		
		g_DMASendLenTotal_Next = len+DMA2_Stream7->NDTR;
		if(g_DMASendLenTotal + len>MAX_SEND_LEN)
		{
			USART_SendStr(USART1,"usart send buf over!\r\n");
			return ;
		}
		strcpy((char *)&USART1_DMA_Buf[g_DMASendLenTotal],pBuf);
		g_DMASendLenTotal += len;
		
		DMA2_Stream7->NDTR= g_DMASendLenTotal_Next;
		DMA2_Stream7->M0AR = (u32)USART1_DMA_Buf+g_DMASendLenTotal-g_DMASendLenTotal_Next;

		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
		USART_Cmd(USART1, ENABLE);
		DMA2->HIFCR = ( (DMA_FLAG_TCIF7|DMA_FLAG_HTIF7 )& 0x0F7D0F7D);//must clear the flag
		DMA_Cmd(DMA2_Stream7, ENABLE);

	}
}


char s_getchar(void)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return (char)(USART1->DR &0x00ff) ;
	
}

int uart_data_valid(void)
{
	int ret = ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET ) ;

	return ret;
	
}
u8 uart_read_char(void)
{
	return (char)(USART1->DR &0x00ff) ;
}


void s_putchar(char data)
{
  	USART_SendData(USART1, data);
  	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}

void s_putstring(char * str)
{
	while(*str != 0){
		s_putchar(*str);
		str++;
	}
}




#if 1
#pragma import(__use_no_semihosting) 

struct __FILE 
{ 
	int handle; 
}; 
FILE __stdout;  
FILE __stdin;  

      
_sys_exit(int x) 
{ 
	x = x; 
} 

#endif
int fputc(int ch, FILE *f)
{
  	USART_SendData(USART1, (u8) ch);
  	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
  	return ch;
}

int fgetc(FILE *stream)
{
	u8 ret = 0;
	ret = s_getchar();
	s_putchar(ret);
	return ret;
}


