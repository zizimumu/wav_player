#ifndef TFT_H
#define TFT_H

#define DATAPORT GPIOD
#define CNTRPORT GPIOB

// PB3,PB4 is JTAG
#define	RsPinNum   0
#define	WRPinNum   1
#define	RdPinNum   2
#define	CsPinNum   5
#define	RetPinNum  6

 //����LCD�ĳߴ�	
#define LCD_W 240
#define LCD_H 320



void Lcd_Init(void); 
void LCD_Clear(u16 Color);
void Address_set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void LCD_WR_DATA8(unsigned char VH,unsigned char VL); //��������-8λ����
void LCD_WR_DATA(uint16_t da);
void LCD_WR_REG(uint16_t da);

void LCD_DrawPoint(u16 x,u16 y,u16 color);

void LCD_DrawPoint_big(u16 x,u16 y,u16 color);

u16  LCD_ReadPoint(u16 x,u16 y); //����
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);

void LCD_DrawLine(u16 x1S, u16 y1S, u16 x2E, u16 y2E,u16 color);

void LCD_DrawRectangle(u16 xStart, u16 yStart, u16 wide, u16 Hight,u16 color);   
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);

void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode);//��ʾһ���ַ�
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len);//��ʾ����
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len);//��ʾ2������
void LCD_ShowString(u16 x,u16 y,const u8 *p);		 //��ʾһ���ַ���,16����
 
void showhanzi(unsigned int x,unsigned int y,unsigned char index);

u16 LCD_Read_Bus(void );



//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //��ɫ
#define BRRED 			 0XFC07 //�غ�ɫ
#define GRAY  			 0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
#define LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)



#endif
