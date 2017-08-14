#include "main.h"
#include "tft.h"

#define WAV_DATA 1

#include "i2s_sampledata.c"
u16 tmp=0;
extern void main_loop(void );


int main(void)
{

	SystemInit();
	NVIC_Configuration();
	PeriphConfig();

	COM1Init(921600);
	//COM1_DMA_Init();
	//TIM3_Init(8400,10000);

	//Key_Init();
	//LED_Init();
	//COM1Init(115200);
	//delay_ms(1000);

	//STM32_Print("timer begin\r\n");

/*
	Lcd_Init();   //tft初始化
	LCD_Clear(RED);

	Address_set(3,3,3,3);//设置光标位置 
	tmp = LCD_Read_Bus();

	LCD_Clear(DARKBLUE);
*/
	//LCD_DrawRectangle(10,10,50,100,BLUE);

	s_putstring("starting the test................\r\n");
//	main_loop();
//	wm_8731_init();



	s_putstring("inited the wm8731\r\n");
	wav_record();
	while(1){
	}

}


