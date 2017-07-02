//V1.0.0
#include "stm32f4xx_it.h"
#include "main.h"
#include <stdio.h>

uint16_t g_IOValue = GPIO_Pin_12;


//NMI exception handler
void NMI_Handler(void)
{
}

//Hard Fault exception handler
void HardFault_Handler(void)
{
  	while (1)
  	{
  	}
}

//Memory Manage exception handler
void MemManage_Handler(void)
{
  	while (1)
  	{
  	}
}

//Bus Fault exception handler
void BusFault_Handler(void)
{
  	while (1)
  	{
  	}
}

//Usage Fault exception handler
void UsageFault_Handler(void)
{
  	while (1)
  	{
  	}
}

//SVCall exception handler
void SVC_Handler(void)
{
}

//Debug Monitor exception handler
void DebugMon_Handler(void)
{
}

//PendSVC exception handler
void PendSV_Handler(void)
{
}
void TIM3_IRQHandler(void)
{

}
//SysTick handler
extern u32 ntime;
void SysTick_Handler(void)
{
	ntime--;
}
void SDIO_IRQHandler(void)
{
  	SD_ProcessIRQSrc();
}

void SD_SDIO_DMA_IRQHANDLER(void)
{	
	//printf("SD_SDIO_DMA_IRQHANDLER\r\n");
  	SD_ProcessDMAIRQ();
}

void DMA1_Stream4_IRQHandler(void)
{    
   audio_irq_handle();
}


