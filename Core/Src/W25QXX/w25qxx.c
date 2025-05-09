#include "w25qxx.h"
#include "spi.h"
#include "usart.h"
#include "main.h"
#include <stdint.h>  // 添加標準整數類型的頭文件
//////////////////////////////////////////////////////////////////////////////////	 
// 本程式僅供學習使用，如需商業用途，請聯繫作者取得授權。
// ALIENTEK STM32F429開發板 W25QXX驅動程式	   
// 技術支持：ALIENTEK
// 技術論壇：www.openedv.com
// 創建日期：2016/1/16
// 版本：V1.0
// 版權所有，侵權必究。
// Copyright(C) 廣州星翼電子科技有限公司 2014-2024
// All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

uint16_t W25QXX_TYPE=W25Q64;	// 默認型號為W25Q64

// 4Kbytes為一個Sector
// 16個Sector為1個Block
// W25Q80
// 總容量為1M字節，每片芯片有16個Block，256個Sector 

// 初始化SPI FLASH的IO口
void W25QXX_Init(void)
{ 
    uint8_t temp;

    __HAL_SPI_ENABLE(&hspi2);                 // 啟用SPI2
    SPI2_ReadWriteByte(0Xff);                 // 啟動傳輸
	
    W25QXX_CS(1);			                // SPI FLASH不選擇
    W25QXX_TYPE=W25QXX_ReadID();	        // 讀取FLASH ID

    if(W25QXX_TYPE==W25Q256)                
    {
        temp=W25QXX_ReadSR(3);              // 讀取狀態寄存器3，確認是否為4字節地址模式
        if((temp&0X01)==0)			        // 若不是4字節地址模式，則進入4字節地址模式
        {
            W25QXX_CS(0); 			        // 選擇芯片
            SPI2_ReadWriteByte(W25X_Enable4ByteAddr);// 發送進入4字節地址模式指令
            W25QXX_CS(1);       		  	// 取消芯片選擇   
        }
    }
}  

// 讀取W25QXX的狀態寄存器
uint8_t W25QXX_ReadSR(uint8_t regno)   
{  
    uint8_t byte=0,command=0; 
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    // 讀取狀態寄存器1指令
            break;
        case 2:
            command=W25X_ReadStatusReg2;    // 讀取狀態寄存器2指令
            break;
        case 3:
            command=W25X_ReadStatusReg3;    // 讀取狀態寄存器3指令
            break;
        default:
            command=W25X_ReadStatusReg1;    
            break;
    }    
    W25QXX_CS(0);                       	// 啟用芯片   
    SPI2_ReadWriteByte(command);            // 發送讀取狀態寄存器指令  
    byte=SPI2_ReadWriteByte(0Xff);          // 讀取一個字節 
    W25QXX_CS(1);                         	// 取消芯片選擇     
    return byte;   
} 

// 寫W25QXX狀態寄存器
void W25QXX_Write_SR(uint8_t regno,uint8_t sr)   
{   
    uint8_t command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    // 寫狀態寄存器1指令
            break;
        case 2:
            command=W25X_WriteStatusReg2;    // 寫狀態寄存器2指令
            break;
        case 3:
            command=W25X_WriteStatusReg3;    // 寫狀態寄存器3指令
            break;
        default:
            command=W25X_WriteStatusReg1;    
            break;
    }   
    W25QXX_CS(0);                            // 啟用芯片   
    SPI2_ReadWriteByte(command);            // 發送寫狀態寄存器指令  
    SPI2_ReadWriteByte(sr);                 // 寫入一個字節 
    W25QXX_CS(1);                            // 取消芯片選擇     	      
}   

// W25QXX寫使能	
void W25QXX_Write_Enable(void)   
{
    W25QXX_CS(0);                            // 啟用芯片   
    SPI2_ReadWriteByte(W25X_WriteEnable);   // 發送寫使能指令  
    W25QXX_CS(1);                            // 取消芯片選擇     	      
} 

// W25QXX寫禁能	
void W25QXX_Write_Disable(void)   
{  
    W25QXX_CS(0);                            // 啟用芯片   
    SPI2_ReadWriteByte(W25X_WriteDisable);  // 發送寫禁能指令  
    W25QXX_CS(1);                            // 取消芯片選擇     	      
} 

// 讀取芯片ID
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;	  
    W25QXX_CS(0);				    
    SPI2_ReadWriteByte(0x90);// 發送讀取ID指令   
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 	 			   
    Temp|=SPI2_ReadWriteByte(0xFF)<<8;  
    Temp|=SPI2_ReadWriteByte(0xFF);	 
    W25QXX_CS(1);				    
    return Temp;
}   		    

// 讀取SPI FLASH
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
    uint16_t i;   										    
    W25QXX_CS(0);                            // 啟用芯片   
    SPI2_ReadWriteByte(W25X_ReadData);      // 發送讀取指令
    if(W25QXX_TYPE==W25Q256)                // 如果是W25Q256，地址為4字節，需發送最高位
    {
        SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>24));    
    }
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>16));   // 發送24bit地址    
    SPI2_ReadWriteByte((uint8_t)((ReadAddr)>>8));   
    SPI2_ReadWriteByte((uint8_t)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
    { 
        pBuffer[i]=SPI2_ReadWriteByte(0XFF);    // 循環讀取數據  
    }
    W25QXX_CS(1);  				    	      
}  

// 寫入頁
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint16_t i;  
    W25QXX_Write_Enable();                  //SET WEL 
    W25QXX_CS(0);                            // 啟用芯片   
    SPI2_ReadWriteByte(W25X_PageProgram);   // 發送寫頁指令 
    if(W25QXX_TYPE==W25Q256)                // 如果是W25Q256，地址為4字節，需發送最高位
    {
        SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>24)); 
    }
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>16)); // 發送24bit地址    
    SPI2_ReadWriteByte((uint8_t)((WriteAddr)>>8));   
    SPI2_ReadWriteByte((uint8_t)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)
        SPI2_ReadWriteByte(pBuffer[i]);// 循環寫入數據  
    W25QXX_CS(1);                            // 取消芯片選擇 
    W25QXX_Wait_Busy();					   // 等待寫入完成
} 

// 無檢查寫入
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
    uint16_t pageremain;	   
    pageremain=256-WriteAddr%256; // 單頁剩餘字節數		 	    
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;// 不大於256個字節
    while(1)
    {	   
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;// 寫入完成
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;	

            NumByteToWrite-=pageremain;			  // 減去已寫入的字節數
            if(NumByteToWrite>256)pageremain=256; // 一次最多寫入256個字節
            else pageremain=NumByteToWrite; 	  // 剩餘字節數
        }
    };	    
} 

// 寫入SPI FLASH
uint8_t W25QXX_BUFFER[4096];		 
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;	   
    uint16_t i;    
    uint8_t * W25QXX_BUF;	  
    W25QXX_BUF=W25QXX_BUFFER;	     
    secpos=WriteAddr/4096;// 扇區地址  
    secoff=WriteAddr%4096;// 扇區內偏移
    secremain=4096-secoff;// 扇區剩餘空間   
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;// 不大於4096個字節
    while(1) 
    {	
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);// 讀出整個扇區的數據
        for(i=0;i<secremain;i++)// 檢查數據
        {
            if(W25QXX_BUF[secoff+i]!=0XFF)break;// 需要擦除  	  
        }
        if(i<secremain)// 需要擦除
        {
            W25QXX_Erase_Sector(secpos);// 擦除整個扇區
            for(i=0;i<secremain;i++)	   // 複製
            {
                W25QXX_BUF[i+secoff]=pBuffer[i];	  
            }
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);// 寫入整個扇區  
        }
        else 
            W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);// 寫入已經擦除的區域				   
        if(NumByteToWrite==secremain)break;// 寫入完成
        else// 寫入未完成
        {
            secpos++;// 扇區地址加1
            secoff=0;// 偏移位置為0 	 

            pBuffer+=secremain;  // 指針偏移
            WriteAddr+=secremain;// 寫入地址偏移	   
            NumByteToWrite-=secremain;				// 字節數減去已寫入的
            if(NumByteToWrite>4096)secremain=4096;	// 剩餘數據大於一個扇區
            else secremain=NumByteToWrite;			// 剩餘數據小於一個扇區
        }	 
    };	 
}

// 擦除整個芯片
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                  // 設置寫使能
    W25QXX_Wait_Busy();   
    W25QXX_CS(0);                           // 啟用芯片   
    SPI2_ReadWriteByte(W25X_ChipErase);     // 發送片擦除命令  
    W25QXX_CS(1);                           // 取消片選     	      
    W25QXX_Wait_Busy();   				   // 等待芯片擦除結束
}   

// 擦除一個扇區
void W25QXX_Erase_Sector(uint32_t Dst_Addr)   
{  
    Dst_Addr*=4096;
    W25QXX_Write_Enable();                  // 設置寫使能 
    W25QXX_Wait_Busy();   
    W25QXX_CS(0);                           // 使能器件   
    SPI2_ReadWriteByte(W25X_SectorErase);   // 發送扇區擦除指令 
    if(W25QXX_TYPE==W25Q256)                // 如果是W25Q256，需要發送4個字節的地址
    {
        SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>24)); 
    }
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //
    SPI2_ReadWriteByte((uint8_t)((Dst_Addr)>>8));   
    SPI2_ReadWriteByte((uint8_t)Dst_Addr);  
    W25QXX_CS(1);                           // 取消片選     	      
    W25QXX_Wait_Busy();                     // 等待擦除完成
}  

// 等待空閒
void W25QXX_Wait_Busy(void)   
{   
    while((W25QXX_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}  

// 進入掉電模式
void W25QXX_PowerDown(void)   
{ 
    W25QXX_CS(0);                            // 使能器件   
    SPI2_ReadWriteByte(W25X_PowerDown);      // 發送掉電命令  
    W25QXX_CS(1);                            // 取消片選     	      
    HAL_Delay(3);                            // 等待TPD  
}   

// 喚醒
void W25QXX_WAKEUP(void)   
{  
    W25QXX_CS(0);                            // 使能器件   
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);// 發送喚醒命令 
    W25QXX_CS(1);                            // 取消片選     	      
    HAL_Delay(3);                            // 等待TRES1
}
