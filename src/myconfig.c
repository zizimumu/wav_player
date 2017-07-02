#include <stm32f4xx.h>
#include "stm32f4xx_conf.h"

/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
                                   This value must be a multiple of 0x200. */

/* xu add begin */
void NVIC_Configuration(void )
{
	/* Configure the NVIC Preemption Priority Bits */  
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif

}
void PeriphConfig(void )
{

//  GPIO_InitTypeDef  GPIO_InitStructure;
//  
//  /* Enable the GPIO_LED Clock */
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
//
//  /* Configure the GPIO_LED pin */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOD, &GPIO_InitStructure);
//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
//GPIOD->MODER = 0x55555555;					//out put moder
//GPIOD->OTYPER &= 0xffff0000; 				//Output push-pull
//GPIOD->OSPEEDR = 0xaaaaaaaa; 				//speed :50M 
//GPIOD->ODR = 0x55555555;

//	SPI1_user_Init();
}
/* xu add end */
