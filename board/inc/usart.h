#ifndef __USART_H
#define __USART_H

#include "stm32f4xx_conf.h"

void COM1Init(u32 BaudRate);
void  COM1_DMA_Init(void );
void STM32_dma_Print(char *pBuf);

void s_putstring(char * str);
char s_getchar(void);
void s_putchar(char data);

#endif

