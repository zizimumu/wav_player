#include <stm32f4xx.h>
#include "tft.h"

#define LCD_Rs_Hight()  CNTRPORT->BSRRL = (1<<RsPinNum)
#define LCD_Rs_Low()  CNTRPORT->BSRRH = (1<<RsPinNum)

#define LCD_WR_Hight()  CNTRPORT->BSRRL = (1<<WRPinNum)
#define LCD_WR_Low()  CNTRPORT->BSRRH = (1<<WRPinNum)

#define LCD_Rd_Hight()  CNTRPORT->BSRRL = (1<<RdPinNum)
#define LCD_Rd_Low()  CNTRPORT->BSRRH = (1<<RdPinNum)

#define LCD_Cs_Hight()  CNTRPORT->BSRRL = (1<<CsPinNum)
#define LCD_Cs_Low()  CNTRPORT->BSRRH = (1<<CsPinNum)

#define LCD_Ret_Hight()  CNTRPORT->BSRRL = (1<<RetPinNum)
#define LCD_Ret_Low()  CNTRPORT->BSRRH = (1<<RetPinNum)

#define DELAY() 	{__nop();}


//#define Bus_16

#ifdef  Bus_16    //条件编译-16位数据模式   
void LCD_Writ_Bus(unsigned char VH,unsigned char VL)   //并行数据写入函数
{
	DATAPORT ->BSRRH = 0xffff;
	DATAPORT->ODR |= ( ( (uint16_t)VH )<<8 | VL);

	LCD_WR_Low();
	DELAY();
	LCD_WR_Hight();

}
u16 LCD_Read_Bus(void )  
{
    LCD_Rd_Low();
    DELAY();
    LCD_Rd_Hight();
	return (DATAPORT->IDR) ;
}

#else			//条件编译-8位数据模式 
void LCD_Writ_Bus(unsigned char VH,unsigned char VL)   //并行数据写入函数
{	
    DATAPORT ->BSRRH =0xffff;
	DATAPORT->ODR |= ( ((uint32_t)VH)  );

	LCD_WR_Low();
	DELAY();
	LCD_WR_Hight();

	DATAPORT ->BSRRH =0xffff;
	DATAPORT->ODR |= ( ((uint32_t)VL)  );

	LCD_WR_Low();
	DELAY();
	LCD_WR_Hight();

}
u16 LCD_Read_Bus(void )   //并行数据写入函数
{	
	u16 tmp=0;

	LCD_Rs_Hight();

	DATAPORT->MODER = 0x00000000;					//in put moder
	DATAPORT->PUPDR = 0x00005555;

	LCD_Rd_Low();
	DELAY();
	LCD_Rd_Hight();

	DELAY();

 	LCD_Rd_Low();
	DELAY();
	LCD_Rd_Hight();

	DELAY();

 	LCD_Rd_Low();
	DELAY();
	LCD_Rd_Hight();

//	delay_us(1000);
	tmp = (DATAPORT->IDR & 0x000000ff) ;
	tmp = (tmp<<8);

	LCD_Rd_Low();
	DELAY();
	LCD_Rd_Hight();

//	delay_us(1000);
	tmp = (tmp | (DATAPORT->IDR & 0x000000ff) ) ;

    DATAPORT->MODER = 0x55555555;					//out put moder

	return tmp;
}
#endif
void LCD_WR_DATA8(unsigned char VH,unsigned char VL) //发送数据-8位参数
{
    LCD_Rs_Hight();
	LCD_Writ_Bus(VH,VL);
}  
 void LCD_WR_DATA(uint16_t da)
{
	LCD_Rs_Hight();
	LCD_Writ_Bus(da>>8,da);
}	  
void LCD_WR_REG(uint16_t da)	 
{	
	LCD_Rs_Low();
	LCD_Writ_Bus(da>>8,da);
}
 void LCD_WR_REG_DATA(uint16_t reg,uint16_t da)
{
    LCD_WR_REG(reg);
	LCD_WR_DATA(da);
}

void Lcd_Init(void)
{

	//GPIO_InitTypeDef  GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);


	DATAPORT->MODER = 0x55555555;					//out put moder
	DATAPORT->OTYPER &= 0xffff0000;					//Output push-pull
	DATAPORT->OSPEEDR = 0xaaaaaaaa;					//speed :50M 


	CNTRPORT->MODER &= ~( (uint32_t)0x11<<(RsPinNum*2) |
					  (uint32_t)0x11<<(WRPinNum*2) |
					  (uint32_t)0x11<<(RdPinNum*2) |
					  (uint32_t)0x11<<(CsPinNum*2) |
					  (uint32_t)0x11<<(RetPinNum*2) );
	CNTRPORT->MODER |= ( (uint32_t)GPIO_Mode_OUT<<(RsPinNum*2) |
					  (uint32_t)GPIO_Mode_OUT<<(WRPinNum*2) |
					  (uint32_t)GPIO_Mode_OUT<<(RdPinNum*2) |
					  (uint32_t)GPIO_Mode_OUT<<(CsPinNum*2) |
					  (uint32_t)GPIO_Mode_OUT<<(RetPinNum*2) );
	CNTRPORT->OTYPER &=( 0xffff0000 | (~( (uint32_t)1<<RsPinNum |
						(uint32_t)1<<WRPinNum |
						(uint32_t)1<<RdPinNum |
						(uint32_t)1<<CsPinNum |
						(uint32_t)1<<RetPinNum )) );
	CNTRPORT->OSPEEDR &= ~( (uint32_t)0x11<<(RsPinNum*2) |
					  (uint32_t)0x11<<(WRPinNum*2) |
					  (uint32_t)0x11<<(RdPinNum*2) |
					  (uint32_t)0x11<<(CsPinNum*2) |
					  (uint32_t)0x11<<(RetPinNum*2) );
	CNTRPORT->OSPEEDR |= ( (uint32_t)GPIO_Speed_50MHz<<(RsPinNum*2) |
					  (uint32_t)GPIO_Speed_50MHz<<(WRPinNum*2) |
					  (uint32_t)GPIO_Speed_50MHz<<(RdPinNum*2) |
					  (uint32_t)GPIO_Speed_50MHz<<(CsPinNum*2) |
					  (uint32_t)GPIO_Speed_50MHz<<(RetPinNum*2) );
									 
	LCD_Cs_Hight();
	LCD_Ret_Hight();
	delay_ms(5);
	LCD_Ret_Low();
	delay_ms(5);
	LCD_Ret_Hight();
	LCD_Cs_Hight();
	LCD_Rd_Hight();
	LCD_WR_Hight();
	delay_ms(5);
	//LCD_CS =0;  //打开片选使能
	LCD_Cs_Low();

	LCD_WR_REG_DATA(0x11,0x2004);	//Power control 2	
	LCD_WR_REG_DATA(0x13,0xCC00);	//Power control 4	
	LCD_WR_REG_DATA(0x15,0x2600);	//Power control 6
	LCD_WR_REG_DATA(0x14,0x252A);	//Power control 5
	//	LCD_WR_REG_DATA(0x14,0x002A);		
	LCD_WR_REG_DATA(0x12,0x0033);	//Power control 3	
	LCD_WR_REG_DATA(0x13,0xCC04);	//Power control 4	
	delay_ms(1);
	LCD_WR_REG_DATA(0x13,0xCC06);		
	delay_ms(1); 
	LCD_WR_REG_DATA(0x13,0xCC4F);		
	delay_ms(1); 
	LCD_WR_REG_DATA(0x13,0x674F);
	LCD_WR_REG_DATA(0x11,0x2003);
	delay_ms(1); 	
	LCD_WR_REG_DATA(0x30,0x2609);	//Adjust Gamma voltage	
	LCD_WR_REG_DATA(0x31,0x242C);	//Adjust Gamma voltage	
	LCD_WR_REG_DATA(0x32,0x1F23);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x33,0x2425);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x34,0x2226);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x35,0x2523);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x36,0x1C1A);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x37,0x131D);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x38,0x0B11);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x39,0x1210);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x3A,0x1315);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x3B,0x3619);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x3C,0x0D00);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x3D,0x000D);	//Adjust Gamma voltage		
	LCD_WR_REG_DATA(0x16,0x0007);	//Power Control 7	
	LCD_WR_REG_DATA(0x02,0x0013);	//LCD Driving Waveform Control	
	LCD_WR_REG_DATA(0x03,0x0003);	
	//Entry Mode,8-bit (80-system), DFM = 0:  262k-color mode (3 times of 6-bit data transfer to GRAM) 
	//the address counter (AC) is automatically increased by 1 after the data is written to the GRAM
	//右下角为0点

	LCD_WR_REG_DATA(0x01,0x0127);	//Driver Output Control,  size : 240RGB X 320,	LCD Raster Rows : 320,Gate- Lines Used :G1 to G320
	delay_ms(1); 
	LCD_WR_REG_DATA(0x08,0x0303);	//Display Control 2 		
	LCD_WR_REG_DATA(0x0A,0x000B);	//Frame Cycle Control 1	
	LCD_WR_REG_DATA(0x0B,0x0003);   //Frame Cycle Control 2
	LCD_WR_REG_DATA(0x0C,0x0000);   //External Interface Control,Interface for RAM Access:System interface / VSYNC interface
	LCD_WR_REG_DATA(0x41,0x0000);    //Vertical scroll control
	LCD_WR_REG_DATA(0x50,0x0000);   //MDDI Wake Up Control 
	LCD_WR_REG_DATA(0x60,0x0005);   //MTP Control 
	LCD_WR_REG_DATA(0x70,0x000B);   //GOE Start / End Timing Control 
	LCD_WR_REG_DATA(0x71,0x0000);   //GSP Clock Delay Control 
	LCD_WR_REG_DATA(0x78,0x0000);    //Vcom Output Control
	LCD_WR_REG_DATA(0x7A,0x0000);   //Panel Signal Control 2
	LCD_WR_REG_DATA(0x79,0x0007);	//Panel Signal Control 1	
	LCD_WR_REG_DATA(0x07,0x0051);   //Display Control 1,Fixed display,Number of display colors :262,144 colors
	delay_ms(1); 	
	LCD_WR_REG_DATA(0x07,0x0053);		
	LCD_WR_REG_DATA(0x79,0x0000);

	LCD_WR_REG(0x0022);

}

void Address_set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{ 
	LCD_WR_REG(0x0046);		//Horizontal Window Address Position of a window for access in memory
	LCD_WR_DATA8(x2,x1);	  
    LCD_WR_REG(0x0047);		//vertical end positions of a window for access in memory
	LCD_WR_DATA(y2);  
    LCD_WR_REG(0x0048);		//vertical start positionsof a window for access in memory
	LCD_WR_DATA(y1);
  	LCD_WR_REG(0x0020);		//RAM Address Set,ado-ad7
	LCD_WR_DATA(x1);	  
    LCD_WR_REG(0x0021);		////RAM Address Set,ad8-ad16
	LCD_WR_DATA(y1); 
    LCD_WR_REG(0x0022);		//Read / Write Data to GRAM		 						 
}

void LCD_Clear(u16 Color)
{
	u8 VH,VL;
	u32 j=0;
	VH=Color>>8;
	VL=Color;	
	Address_set(0,0,LCD_W-1,LCD_H-1);
	for (j=0;j<LCD_W*LCD_H;j++)
	{
       LCD_WR_DATA8(VH,VL);
	}
}

void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	Address_set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color); 	    
} 	 

void LCD_DrawPoint_big(u16 x,u16 y,u16 color)
{
	LCD_Fill(x-1,y-1,x+1,y+1,color);
} 

void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j; 
	Address_set(xsta,ysta,xend,yend);      //设置光标位置 
	for(i=ysta;i<=yend;i++)
	{													   	 	
		for(j=xsta;j<=xend;j++)LCD_WR_DATA(color);//设置光标位置 	    
	} 					  	    
}  

void LCD_DrawLine(u16 x1S, u16 y1S, u16 x2E, u16 y2E,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	if( (x2E > LCD_W-1) || (y2E>LCD_H-1))
		return ;
	delta_x=x2E-x1S; //计算坐标增量 
	delta_y=y2E-y1S; 
	uRow=x1S; 
	uCol=y1S; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    

void LCD_DrawRectangle(u16 xStart, u16 yStart, u16 wide, u16 Hight,u16 color)
{
	if( (xStart + wide -1 > LCD_W) || (yStart + Hight -1> LCD_H) )
		return ;
	LCD_DrawLine(xStart,yStart,xStart+wide-1,yStart,color);
	LCD_DrawLine(xStart,yStart,xStart,yStart+Hight-1,color);
	LCD_DrawLine(xStart,yStart+Hight-1,xStart+wide-1,yStart+Hight-1,color);
	LCD_DrawLine(xStart+wide-1,yStart,xStart+wide-1,yStart+Hight-1,color);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1       
		LCD_DrawPoint(x0-b,y0-a,color);             //7           
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b,color);
	}
} 
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"

//mode:叠加方式(1)还是非叠加方式(0)
//void LCD_ShowChar(u16 x,u16 y,u8 num,u8 mode)
//{
//    u8 temp;
//    u8 pos,t;
//	u16 x0=x;
//	u16 colortemp=POINT_COLOR;      
//    if(x>LCD_W-16||y>LCD_H-16)return;	    
//	//设置窗口		   
//	num=num-' ';//得到偏移后的值
//	Address_set(x,y,x+8-1,y+16-1);      //设置光标位置 
//	if(!mode) //非叠加方式
//	{
//		for(pos=0;pos<16;pos++)
//		{ 
//			//temp=asc2_1608[(u16)num*16+pos];		 //调用1608字体
//			for(t=0;t<8;t++)
//		    {                 
//		        if(temp&0x01)POINT_COLOR=colortemp;
//				else POINT_COLOR=BACK_COLOR;
//				LCD_WR_DATA(POINT_COLOR);	
//				temp>>=1; 
//				x++;
//		    }
//			x=x0;
//			y++;
//		}	
//	}else//叠加方式
//	{
//		for(pos=0;pos<16;pos++)
//		{
//		    //temp=asc2_1608[(u16)num*16+pos];		 //调用1608字体
//			for(t=0;t<8;t++)
//		    {                 
//		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点     
//		        temp>>=1; 
//		    }
//		}
//	}
//	POINT_COLOR=colortemp;	    	   	 	  
//}   
//m^n函数
//u32 mypow(u8 m,u8 n)
//{
//	u32 result=1;	 
//	while(n--)result*=m;    
//	return result;
//}			 
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//color:颜色
//num:数值(0~4294967295);	
//void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len)
//{         	
//	u8 t,temp;
//	u8 enshow=0;
//	num=(u16)num;
//	for(t=0;t<len;t++)
//	{
//		temp=(num/mypow(10,len-t-1))%10;
//		if(enshow==0&&t<(len-1))
//		{
//			if(temp==0)
//			{
//				LCD_ShowChar(x+8*t,y,' ',0);
//				continue;
//			}else enshow=1; 
//		 	 
//		}
//	 	LCD_ShowChar(x+8*t,y,temp+48,0); 
//	}
//} 
//显示2个数字
//x,y:起点坐标
//num:数值(0~99);	 
//void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len)
//{         	
//	u8 t,temp;						   
//	for(t=0;t<len;t++)
//	{
//		temp=(num/mypow(10,len-t-1))%10;
//	 	LCD_ShowChar(x+8*t,y,temp+'0',0); 
//	}
//} 
//显示字符串
//x,y:起点坐标  
//*p:字符串起始地址
//用16字体
//void LCD_ShowString(u16 x,u16 y,const u8 *p)
//{         
//    while(*p!='\0')
//    {       
//        if(x>LCD_W-16){x=0;y+=16;}
//        if(y>LCD_H-16){y=x=0;LCD_Clear(RED);}
//        LCD_ShowChar(x,y,*p,0);
//        x+=8;
//        p++;
//    }  
//}



