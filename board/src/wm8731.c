#include "wm8731.h"
#include "delay.h"

#define CLK_HIGH()  	GPIO_SetBits(GPIOA,GPIO_Pin_5)
#define CLK_LOW()  		GPIO_ResetBits(GPIOA,GPIO_Pin_5)

#define CSB_HIGH()		GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define CSB_LOW()		GPIO_ResetBits(GPIOA,GPIO_Pin_4)

#define DATA_BIT_HIGH()		GPIO_SetBits(GPIOA,GPIO_Pin_7)
#define DATA_BIT_LOW()		GPIO_ResetBits(GPIOA,GPIO_Pin_7)

void wm8731_gpio_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  /* Configure the GPIO_LED pin */
  //CSB sclk  SDIN 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;// | GPIO_Pin_5  | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void write_register_io(u8 addr,u8 value)
{
	u16 send_data;
	u16 tmp = 1;
	u8 i;

	send_data = ((u16)addr )<< 9 | value ;
	CSB_LOW();
	delay_us(1);

	for(i = 0; i < 16; i++){
		CLK_LOW();
		delay_us(500);

		if(send_data & (tmp<<i))
			DATA_BIT_HIGH();
		else
			DATA_BIT_LOW();

		CLK_HIGH();
		delay_us(500);
	}
	CSB_HIGH();
	delay_us(1);

}

void write_register(u16 addr,u16 value)
{
	CSB_LOW();
	delay_us(1);
	SPI1_RW_16bit( ((addr<< 9) | (value&0x01ff)) );
	delay_us(1);
	CSB_HIGH();
}

void wm_spi_init(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	SPI_InitTypeDef  SPI_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA ,ENABLE);

	// GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);	// for nss
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  	SPI_I2S_DeInit(SPI1);
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//全双工
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;//8位数据模式
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//空闲模式下SCK为1
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//数据采样从第2个时间边沿开始
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//NSS软件管理
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;//波特率
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//大端模式
  	SPI_InitStructure.SPI_CRCPolynomial = 7;//CRC多项式
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;//主机模式

	
  	SPI_Init(SPI1, &SPI_InitStructure);
	//SPI_SSOutputCmd(SPI1,ENABLE);
  	SPI_Cmd(SPI1, ENABLE);
}


#define CODEC_DATA_FORTMAT_DSP 3
#define CODEC_DATA_FORTMAT_I2S 2
#define CODEC_DATA_FORTMAT_MSB_L 1
#define CODEC_DATA_FORTMAT_MSB_R 0

#define CODEC_DATA_LENGTH_32 3
#define CODEC_DATA_LENGTH_24 2
#define CODEC_DATA_LENGTH_20 1
#define CODEC_DATA_LENGTH_16 0

#define CODEC_MODEL_SLAVE  0
#define CODEC_MODEL_MAST 1





void wm_8731_init(void )
{
	wm_spi_init();
	wm8731_gpio_init();
	
	write_register(0x0f,0x00);		//reset all

	delay_ms(1);
	write_register(0x09,0x00);		//inactive
	delay_ms(1);
	
	//write_register(0x00,0x17);		//left line in , vol	
	//write_register(0x01,0x17);		//left line in , vol	
	write_register(0x02,0x6a);		//Left Headphone Out: set left line out volume,the max is 0x7f
	write_register(3, 0x6a);  	// Right Headphone Out: set right line out volume,,the max is 0x7f
	write_register(4, 0x15); 		 // Analogue Audio Path Control: set mic as input and boost it, and enable dac 
	write_register(5, 0x00);  	// ADC ,DAC Digital Audio Path Control: disable soft mute   
	write_register(6, 0);  			// power down control: power on all 
	write_register(7, CODEC_DATA_FORTMAT_MSB_L | (CODEC_DATA_LENGTH_16 <<2) );  	// 0x01:MSB,left,iwl=16-bits, Enable slave Mode;0x09 : MSB,left,24bit
	write_register(8, 0x00);  	// Normal, Base OVer-Sampleing Rate 384 fs (BOSR=1) 
	write_register(9, 0x01);  	// active interface
}
