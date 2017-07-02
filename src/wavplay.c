#include "wavplay.h"
#include "ff.h"
#include "stdio.h"

#define I2S_ENABLE_MASK      0x0400

volatile u8 gHalf_dma_iflag = 0;
volatile u8 gComp_dma_iflag = 0;

u8 gAudio_buff[2048]={0};
DMA_InitTypeDef DMA_InitStructure;      
FIL gFile;
__IO u32 gReadLen=0;



WAVE_TypeDef WAVE_Format;


void AUDIO_Init(u32 audio_sample)
{
	I2S_user_Init(audio_sample);
	Audio_DMA_Init();	
}

 void AUDIO_TransferComplete(void )
{
	
	DMA_Cmd(DMA1_Stream4,DISABLE);
	gHalf_dma_iflag = 0;
	gComp_dma_iflag = 0;
	printf("wav play complete\r\n");
	gReadLen = 0;
}
void wav_play(void)
{
	FATFS fatfs;            
	u32 data_len = 0;
	UINT BytesRead = 0;	
	FRESULT rt = FR_EXIST ;


  	f_mount(0, &fatfs);

	
	rt = f_open(&gFile, "0:/valder.wav" , FA_READ);
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
		printf("wav parsing ok,sample rate %d\r\n data sieze%x\r\n ByteRate %d\r\n  NumChannels %d\r\n  BitsPerSample %d\r\n",\
		    WAVE_Format.SampleRate,WAVE_Format.DataSize,WAVE_Format.ByteRate,WAVE_Format.NumChannels,WAVE_Format.BitsPerSample);

  	AUDIO_Init(WAVE_Format.SampleRate);
	
  	f_lseek(&gFile, sizeof(WAVE_Format));								//ignore head file

	if(WAVE_Format.DataSize < 1024*2)
		data_len = WAVE_Format.DataSize;
	else 
		data_len = 1024*2;

	gReadLen = 0;
	gReadLen += data_len;
  	f_read(&gFile, gAudio_buff, data_len, &BytesRead); 
  	Audio_MAL_Play((u32)gAudio_buff, data_len);

	while(1){
		if(gHalf_dma_iflag){
			gHalf_dma_iflag = 0;
			if(gReadLen < WAVE_Format.DataSize){
				if(WAVE_Format.DataSize - gReadLen < 1024){	
					f_read(&gFile, gAudio_buff, WAVE_Format.DataSize - gReadLen, &BytesRead);
					gReadLen = WAVE_Format.DataSize;
				}
				else {
					rt = f_read(&gFile, gAudio_buff, 1024, &BytesRead);
					gReadLen += 1024;
				}
			}
			else
				AUDIO_TransferComplete(); 			
		}

		else if(gComp_dma_iflag){
			gComp_dma_iflag = 0;
			if (gReadLen < WAVE_Format.DataSize){											//check out for file end
				//while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE);//wait for DMA disabled 	
					  
				if(WAVE_Format.DataSize - gReadLen < 1024){ 
					rt = f_read(&gFile, gAudio_buff+1024, WAVE_Format.DataSize - gReadLen, &BytesRead);
					gReadLen = WAVE_Format.DataSize;
				}
				else{
					rt = f_read(&gFile, gAudio_buff+1024, 1024, &BytesRead);
					gReadLen += 1024;
				}
			}
			else{
				AUDIO_TransferComplete();		
			}

		}
	}


    err:
        return ;
}	
//DMA传送配置
void Audio_DMA_Init(void)  
{ 
  	NVIC_InitTypeDef NVIC_InitStructure;
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
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
  	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  	DMA_Init(DMA1_Stream4, &DMA_InitStructure);  
	
  	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC | DMA_IT_HT, ENABLE);

  	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);   
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
  	DMA_InitStructure.DMA_BufferSize=(uint32_t)Size/2;
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

