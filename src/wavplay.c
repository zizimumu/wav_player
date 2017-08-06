#include "wavplay.h"
#include "ff.h"
#include "stdio.h"
#include "wm8731.h"
#include "usart.h"
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
	return ret ;
}

void AUDIO_Init(u32 audio_sample,u32 frame_bit)
{
	I2S_user_Init(audio_sample,frame_bit);
	Audio_DMA_Init(frame_bit);	

    wm_8731_init(audio_sample,frame_bit);
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

