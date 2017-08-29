#include "wavplay.h"
#include "ff.h"
#include "stdio.h"
#include "wm8731.h"
#include "usart.h"
#include "key.h"

#include "delay.h"
#include <stdio.h>
#include <string.h>
#define I2S_ENABLE_MASK      0x0400

volatile u8 gHalf_dma_iflag = 0;
volatile u8 gComp_dma_iflag = 0;
u32 wr_pt = 0;

#define AUDIO_BUFF_SIZE (1024*8)  // 512*
u8 gAudio_buff[AUDIO_BUFF_SIZE];
DMA_InitTypeDef DMA_InitStructure;      
FIL gFile;
__IO u32 gReadLen=0;
#define ROOT_DIR "0:"


WAVE_TypeDef WAVE_Format;

 char*  scan_files (char* path,char *filter,int open)
{
    FRESULT res;
    FILINFO fno = {0};
    DIR dir;
    int i;
    char *fn;
    unsigned int file_num = 1;
#if _USE_LFN
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif

	if(!open)
    		printf("%s/ \r\n", path);
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fname[0] == '.') continue;
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {
                //sprintf(&path[i], "/%s", fn);
               //res = scan_files(path);
                //if (res != FR_OK) break;
               // path[i] = 0;
            } else {
            	if((!memcmp(filter,( fn+strlen(fn) - strlen(filter)),strlen(filter)))){
			if(open == 0)
				printf("    %02d  %s\r\n",file_num, fn);
			else
				if(file_num == open)
					return fn;
				
			file_num++;
		}
            }
        }
    }
 
    return NULL;
}


int get_stop(void)
{
	int ret = 0;
    u8 val = 0;
	if(uart_data_valid()){
	        val = uart_read_char();
			if(val == 0x03) // ctr + c
				ret = 1;

	        if(val == 'u'){ // up key
	            wm_8731_vol_up();
	        }
	        else if(val == 'd'){// down key
	            wm_8731_vol_down();
	        }
	}

	if(KEY_Scan() == 1)
		ret = 1;
	
	return ret ;
}

void AUDIO_Init(u32 audio_sample,u32 frame_bit)
{
	I2S_user_Init(audio_sample,frame_bit);
	Audio_DMA_Init(frame_bit);	

    	wm_8731_init(audio_sample,frame_bit);
}





void I2S_DMAConfig_Rx(u32 buff_size,u32 buff_addr)
{
	//NVIC_InitTypeDef  NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	DMA_Cmd(DMA1_Stream3, DISABLE);
	while(DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);

	DMA_DeInit(DMA1_Stream3);

	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buff_addr; //接收数据的内存的地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_BufferSize = buff_size/2; //单位为上面的DataSize
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	/* Initiate DMA */
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);
	//SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

}


extern void I2S_GPIO_Init(void);

void I2S_RXConfig(uint32_t Freq)
{
	I2S_InitTypeDef I2S_InitStructure;

	I2S_GPIO_Init();
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/* Deinitialize SPI2 peripheral */
	SPI_I2S_DeInit(SPI2);

	/* I2S2 peripheral configuration */
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;
	I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_AudioFreq = Freq;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_Init(SPI2, &I2S_InitStructure);


}
#define RX_BUFF_SIZE (80*1024)
u8 rec_buff[RX_BUFF_SIZE];

void record_init()
{

	wm_8731_record_init(48000,16);
	I2S_RXConfig(48000);
	I2S_DMAConfig_Rx(RX_BUFF_SIZE,(u32)rec_buff);

}
 void record_Start(void)
 {
	 DMA_Cmd(DMA1_Stream3, ENABLE);
 
	 SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
	 I2S_Cmd(SPI2, ENABLE);
 
 }

 void record_Stop(void)
 {
	 DMA_Cmd(DMA1_Stream3, DISABLE);
 
	 SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, DISABLE);
	 I2S_Cmd(SPI2, DISABLE);
 }

 void AUDIO_TransferComplete(void )
{

    
	DMA_Cmd(DMA1_Stream4,DISABLE);
    I2S_Cmd(SPI2,DISABLE);
    wm_8731_reset();
    
	gHalf_dma_iflag = 0;
	gComp_dma_iflag = 0;
	
	gReadLen = 0;

	f_close(&gFile);
	printf("wav play complete\r\n");
}





int get_user_choice()
{
	int input = 0;
	scan_files(ROOT_DIR,".wav",0);
	printf("\r\n\r\nplease choice the auido file num\r\n");
	scanf("%d",&input);

	printf("chosing %d\r\n",input);
	return input;
}

#define DAM_MIN_GEP 3
void wait_dma_done(void)
{
	u32 dma,remaind_sz;
	//u8 *r_packet;
	int pre,next;
	
	remaind_sz = (AUDIO_BUFF_SIZE  - wr_pt)/(WAVE_Format.BitsPerSample/8);

	pre = remaind_sz - DAM_MIN_GEP;
	next = remaind_sz + DAM_MIN_GEP;

	if(pre < 0)
		pre = 0;
	if(next > AUDIO_BUFF_SIZE/(WAVE_Format.BitsPerSample/8) )
		next = AUDIO_BUFF_SIZE/(WAVE_Format.BitsPerSample/8);

	while(1){
		dma = DMA_GetCurrDataCounter(DMA1_Stream4) ;
		if(dma >= pre && dma <= next)
			break;
	}

    memset(wr_pt+gAudio_buff,0,AUDIO_BUFF_SIZE-wr_pt);
    memset(gAudio_buff,0,wr_pt);
    delay_ms(500);

    
    
}


void wav_play(void)
{
	FATFS fatfs;            
	u32 data_len = 0,input;
	UINT BytesRead = 0;	
	FRESULT rt = FR_EXIST ;
	char path[100];
	char *audio_fs;
	int audio_num = 1;
    u32 valid;


  	f_mount(0, &fatfs);

	scan_files(ROOT_DIR,".wav",0);
play :	
	memset(path,0,sizeof(path));
    memset(gAudio_buff,0,sizeof(gAudio_buff));
	audio_fs = scan_files(ROOT_DIR,".wav",audio_num);

	if(audio_fs == NULL){
		printf("no wav file find !!!\r\n");
		while(1);
	}
	sprintf(path,"%s/%s",ROOT_DIR,audio_fs);
	printf("%s openning\r\n",path);
	
	rt = f_open(&gFile, path, FA_READ);
	if(rt != FR_OK){
		printf("open file error %d \r\n",rt) ;
        	goto err;
    	}
	

	rt = f_read(&gFile, gAudio_buff, 1024, &BytesRead);
	if(rt != FR_OK){
		printf("read file error %d \r\n",rt) ;
       		 goto err;
    	}
	
	if(WaveParsing(gAudio_buff) )
		printf("wav parsing error\r\n");
	else
		printf("wav parsing ok\r\nsample rate %d\r\n data sieze %x\r\n ByteRate %d\r\n  NumChannels %d\r\n  BitsPerSample %d\r\n",\
		    WAVE_Format.SampleRate,WAVE_Format.DataSize,WAVE_Format.ByteRate,WAVE_Format.NumChannels,WAVE_Format.BitsPerSample);



  	AUDIO_Init(WAVE_Format.SampleRate,WAVE_Format.BitsPerSample);
	
  	f_lseek(&gFile, sizeof(WAVE_Format));								//ignore head file

	if(WAVE_Format.DataSize < AUDIO_BUFF_SIZE)
		data_len = WAVE_Format.DataSize;
	else 
		data_len = AUDIO_BUFF_SIZE;

	
	//wr_pt += data_len;
  	f_read(&gFile, gAudio_buff, data_len, &BytesRead); 
  	Audio_MAL_Play((u32)gAudio_buff, data_len);
   // delay_ms(500);

    //

    wr_pt = data_len;
    if(wr_pt >= AUDIO_BUFF_SIZE)
        wr_pt = 0;

	while(1){
		if(get_stop()){
			printf("ctrl + c to stop \r\n");
            wait_dma_done();
			AUDIO_TransferComplete();
			audio_num = get_user_choice();
			goto play;
		}

        valid = AUDIO_BUFF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream4)*(WAVE_Format.BitsPerSample/8);
        if(valid >= wr_pt)
            valid = valid - wr_pt;
        else
            valid = AUDIO_BUFF_SIZE - (wr_pt - valid);

        if(valid >= 1024){

            rt = f_read(&gFile, gAudio_buff + wr_pt,512, &BytesRead);
            wr_pt += BytesRead;
            if(wr_pt >= AUDIO_BUFF_SIZE)
                wr_pt = 0;

            if(BytesRead < 512){
                wait_dma_done();
				AUDIO_TransferComplete(); 
				audio_num++;
				goto play;
            }
        }

      }
    err:
        return ;
}	



#define MIN_BLOCK 512
void wav_record(void)
{
	FATFS fatfs;            
	u32 data_len = 0,input;
	UINT BytesRead = 0;	
	FRESULT rt = FR_EXIST ;
	char path[100];
	char *audio_fs;
	int audio_num = 1;
    	u32 rd,wr,i,free,sz;
	u32 f_sz = 0,block;




  	f_mount(0, &fatfs);
	
	rt = f_open(&gFile, "0:/test.bin", FA_CREATE_ALWAYS | FA_WRITE|FA_READ);
	if(rt != FR_OK){
		printf("open file error %d \r\n",rt) ;
		return ;
    	}


#if 0
	sz = 20*1024*1024;
	data_len = 40960;

	for(i=0;i<data_len;i++)
		rec_buff[i]=(u8)i;

	while(1){
		rt = f_write(&gFile,rec_buff,data_len,&BytesRead);

		if(BytesRead != data_len)
			printf("write data err %d,%d\r\n",BytesRead,rt);
		
		f_sz += data_len;

		if(f_sz >= sz){
			//f_close(&gFile);
			printf("write done\r\n");
			break ;

		}


	}
	printf("read ckeck ...\r\n");
	f_sz = 0;
	f_close(&gFile);
	rt = f_open(&gFile, "0:/test.bin", FA_READ);
	if(rt != FR_OK){
		printf("open file error %d \r\n",rt) ;
		return ;
    	}	
	while(1){
		memset(rec_buff,0,data_len);
		rt = f_read(&gFile,rec_buff,data_len,&BytesRead);

		if(BytesRead != data_len)
			printf("read data err %d,%d\r\n",BytesRead,rt);

		for(i=0;i<data_len;i++){
			if(rec_buff[i] != (u8)i){
				printf("ckeck faild,%d,rec_buff[i]=%d\r\n",i,rec_buff[i]);
			}
		}
		
		f_sz += data_len;

		if(f_sz >= sz){
			f_close(&gFile);
			printf("ckeck done\r\n");
			return ;

		}


	}

	
#endif	


	record_init();
	record_Start();

	rd = 0;
	block = 0;
	while(1){
		if(get_stop()){
			printf("stopped ,file size %d\r\n",f_sz);
			record_Stop();
			f_close(&gFile);

			return ;
		}	

#if 0
		wr = RX_BUFF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream3)*2;
		if(wr >= rd)
			free = wr - rd;
		else
			free = RX_BUFF_SIZE - rd + wr;

		if(free >= MIN_BLOCK){

			if(RX_BUFF_SIZE - rd >= MIN_BLOCK){
				rt = f_write(&gFile,rec_buff + rd,MIN_BLOCK,&BytesRead);
				if(BytesRead !=MIN_BLOCK )
					printf("write data err %d,%d\r\n",BytesRead,rt);
				
				rd += MIN_BLOCK;
				if(rd >= RX_BUFF_SIZE)
					rd = 0;

			}
			else{
				sz = MIN_BLOCK - (RX_BUFF_SIZE - rd);
				rt = f_write(&gFile,rec_buff + rd,RX_BUFF_SIZE - rd,&BytesRead);
				if(BytesRead !=RX_BUFF_SIZE - rd )
					printf("write data err a %d,%d\r\n",RX_BUFF_SIZE - rd,rt);
								
				rt = f_write(&gFile,rec_buff,sz,&BytesRead);
				if(BytesRead !=sz )
					printf("write data err a %d,%d\r\n",sz,rt);

				rd = sz;
			}
			f_sz += MIN_BLOCK;
				
		}
#else
		
		wr = RX_BUFF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream3)*2;
		if(wr >= RX_BUFF_SIZE/2 && block == 0){
			block = 1;
			rd = 0;
			//for(i=0;i<RX_BUFF_SIZE/2/512;i++){
				rt = f_write(&gFile,rec_buff ,RX_BUFF_SIZE/2,&BytesRead);
				if(BytesRead !=RX_BUFF_SIZE/2 )
					printf("write data err %d,%d\r\n",BytesRead,rt);
				rd  += RX_BUFF_SIZE/2;
			//}
				
		}

		else if(wr < RX_BUFF_SIZE/2 && block == 1){
			block = 0;
			rd = 0;
			//for(i=0;i<RX_BUFF_SIZE/2/512;i++){
				rt = f_write(&gFile,rec_buff +RX_BUFF_SIZE/2,RX_BUFF_SIZE/2,&BytesRead);
				if(BytesRead !=RX_BUFF_SIZE/2 )
					printf("write data err %d,%d\r\n",BytesRead,rt);
				rd  += RX_BUFF_SIZE/2;
			//}
		}
#endif
	}


}	

//DMA传送配置
void Audio_DMA_Init(u32 frame_bit)  
{ 
  	NVIC_InitTypeDef NVIC_InitStructure;
	u32 p_size,m_size;

	if(frame_bit == 16){
		m_size = DMA_MemoryDataSize_HalfWord;
        p_size = DMA_PeripheralDataSize_HalfWord;
     }
	else if(frame_bit == 32){
		m_size = DMA_MemoryDataSize_Word;
        p_size = DMA_PeripheralDataSize_Word;

    }
	else{
		printf("dma init err\r\n");
		return ;
	}
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); 
  	DMA_Cmd(DMA1_Stream4, DISABLE);
  	DMA_DeInit(DMA1_Stream4);
  	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x4000380C;
  	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
  	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  	DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  	DMA_InitStructure.DMA_PeripheralDataSize = p_size;
  	DMA_InitStructure.DMA_MemoryDataSize = m_size; 
  	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  	DMA_Init(DMA1_Stream4, &DMA_InitStructure);  

  #if 0  
  	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC | DMA_IT_HT, ENABLE);
  	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);  
#endif
    
  	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
}

/*
uint32_t AUDIO_Play(u16* pBuffer, u32 Size)
{
  	AudioTotalSize=Size; 
  	Audio_MAL_Play((u32)pBuffer,(u32)(DMA_MAX(Size/4)));//更新媒体层数据并播放
  	AudioRemSize=(Size/2)-DMA_MAX(AudioTotalSize);//更新剩余数据量
  	CurrentPos=pBuffer+DMA_MAX(AudioTotalSize);//更新当新数据指针 
  	return 0;
}
*/

void Audio_MAL_Play(u32 Addr, u32 Size)
{         
  	DMA_InitStructure.DMA_Memory0BaseAddr=(uint32_t)Addr;
  	DMA_InitStructure.DMA_BufferSize=(uint32_t)Size/(WAVE_Format.BitsPerSample/8);
  	DMA_Init(DMA1_Stream4,&DMA_InitStructure);
  	DMA_Cmd(DMA1_Stream4,ENABLE); 
  	if ((SPI2->I2SCFGR & I2S_ENABLE_MASK)==0)I2S_Cmd(SPI2,ENABLE);
}


u8 WaveParsing(const u8 * buf)
{
  	u32 temp=0x00;


  	temp=ReadUnit((u8*)buf,0,4,BigEndian);							//read 'RIFF'
  	if(temp!=CHUNK_ID)return 1;
  	WAVE_Format.RIFFchunksize=ReadUnit((u8*)buf,4,4,LittleEndian);	//read file lenght
  	temp=ReadUnit((u8*)buf,8,4,BigEndian);							//read 'WAVE'
  	if(temp!=FILE_FORMAT)return 2;
  	temp=ReadUnit((u8*)buf,12,4,BigEndian);							//read 'fmt '
  	if(temp!=FORMAT_ID)return 3;
  	temp=ReadUnit((u8*)buf,16,4,LittleEndian);						//read lenght of 'fmt' 

  	WAVE_Format.FormatTag=ReadUnit((u8*)buf,20,2,LittleEndian);		//读音频格式
  	if(WAVE_Format.FormatTag!=WAVE_FORMAT_PCM)return 4;  
  	WAVE_Format.NumChannels=ReadUnit((u8*)buf,22,2,LittleEndian);	//read chanel number
	WAVE_Format.SampleRate=ReadUnit((u8*)buf,24,4,LittleEndian);	//read sample rate
	WAVE_Format.ByteRate=ReadUnit((u8*)buf,28,4,LittleEndian);
	WAVE_Format.BlockAlign=ReadUnit((u8*)buf,32,2,LittleEndian);
	WAVE_Format.BitsPerSample=ReadUnit((u8*)buf,34,2,LittleEndian);
	if(WAVE_Format.BitsPerSample!=BITS_PER_SAMPLE_16)return 5;

	temp=ReadUnit((u8*)buf,36,4,BigEndian);//读'data'

	if(temp!=DATA_ID)return 8;
	WAVE_Format.DataSize=ReadUnit((u8*)buf,40,4,LittleEndian);			

	return 0;
}
u32 ReadUnit(u8 *buffer,u8 idx,u8 NbrOfBytes,Endianness BytesFormat)
{
  	u32 index=0;
  	u32 temp=0;
  
  	for(index=0;index<NbrOfBytes;index++)temp|=buffer[idx+index]<<(index*8);
  	if (BytesFormat == BigEndian)temp= __REV(temp);
  	return temp;
}


void audio_irq_handle(void)
{    

  	if (DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4)!=RESET){   
		gComp_dma_iflag = 1;
		DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);  
  	}
	else if(DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_HTIF4)!=RESET){
		DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_HTIF4); 
		gHalf_dma_iflag = 1;
	}
}

