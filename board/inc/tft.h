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

 //定义LCD的尺寸	
#define LCD_W 240
#define LCD_H 320



void Lcd_Init(void); 
void LCD_Clear(u16 Color);
void Address_set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void LCD_WR_DATA8(unsigned char VH,unsigned char VL); //发送数据-8位参数
void LCD_WR_DATA(uint16_t da);
void LCD_WR_REG(uint16_t da);

void LCD_DrawPoint(u16 x,u16 y,u16 color);

void LCD_DrawPoint_big(u16 x,u16 y,u16 color);

u16  LCD_ReadPoint(u16 x,u16 y); //读点
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);

void LCD_DrawLine(u16 x1S, u16 y1S, u16 x2E, u16 y2E,u16 color);

void LCD_DrawRectangle(u16 xStart, u16 yStart, u16 wide, u16 Hight,u16 color);   
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);

void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode);//显示一个字符
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len);//显示数字
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len);//显示2个数字
void LCD_ShowString(u16 x,u16 y,const u8 *p);		 //显示一个字符串,16字体
 
void showhanzi(unsigned int x,unsigned int y,unsigned char index);

u16 LCD_Read_Bus(void );



//画笔颜色
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
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)



#endif
