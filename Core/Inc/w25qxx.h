#ifndef __W25QXX_H
#define __W25QXX_H
#include "main.h"
#include <stdint.h>  // 添加標準整數類型的頭文件
//////////////////////////////////////////////////////////////////////////////////
// 本程式僅供學習使用，如需商業用途，請聯繫作者取得授權。
// ALIENTEK STM32F429開發板 W25QXX 驅動程式
// 技術支持：ALIENTEK
// 技術論壇：www.openedv.com
// 創建日期：2016/1/16
// 版本：V1.0
// 版權所有，侵權必究。
// Copyright(C) 廣州星翼電子科技有限公司 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

// W25QXX 系列芯片 ID 定義
// W25Q80  ID  0XEF13
// W25Q16  ID  0XEF14
// W25Q32  ID  0XEF15
// W25Q64  ID  0XEF16
// W25Q128 ID  0XEF17
// W25Q256 ID  0XEF18
#define W25Q80  0XEF13
#define W25Q16  0XEF14
#define W25Q32  0XEF15
#define W25Q64  0XEF16
#define W25Q128 0XEF17
#define W25Q256 0XEF18

extern uint16_t W25QXX_TYPE;  // 定義 W25QXX 型號

// W25QXX 芯片選擇控制
#define W25QXX_CS(n)  (n ? HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_RESET))

//////////////////////////////////////////////////////////////////////////////////
// 指令定義
#define W25X_WriteEnable        0x06  // 寫使能
#define W25X_WriteDisable       0x04  // 寫禁止
#define W25X_ReadStatusReg1     0x05  // 讀狀態寄存器1
#define W25X_ReadStatusReg2     0x35  // 讀狀態寄存器2
#define W25X_ReadStatusReg3     0x15  // 讀狀態寄存器3
#define W25X_WriteStatusReg1    0x01  // 寫狀態寄存器1
#define W25X_WriteStatusReg2    0x31  // 寫狀態寄存器2
#define W25X_WriteStatusReg3    0x11  // 寫狀態寄存器3
#define W25X_ReadData           0x03  // 讀數據
#define W25X_FastReadData       0x0B  // 快速讀數據
#define W25X_FastReadDual       0x3B  // 快速雙線讀數據
#define W25X_PageProgram        0x02  // 頁編程
#define W25X_BlockErase         0xD8  // 塊擦除
#define W25X_SectorErase        0x20  // 扇區擦除
#define W25X_ChipErase          0xC7  // 芯片擦除
#define W25X_PowerDown          0xB9  // 進入掉電模式
#define W25X_ReleasePowerDown   0xAB  // 解除掉電模式
#define W25X_DeviceID           0xAB  // 設備ID
#define W25X_ManufactDeviceID   0x90  // 製造商ID
#define W25X_JedecDeviceID      0x9F  // JEDEC ID
#define W25X_Enable4ByteAddr    0xB7  // 啟用4字節地址
#define W25X_Exit4ByteAddr      0xE9  // 退出4字節地址

void W25QXX_Init(void);  // 初始化 W25QXX
uint16_t W25QXX_ReadID(void);  // 讀取 FLASH ID
uint8_t W25QXX_ReadSR(uint8_t regno);  // 讀取狀態寄存器
void W25QXX_4ByteAddr_Enable(void);  // 啟用4字節地址模式
void W25QXX_Write_SR(uint8_t regno, uint8_t sr);  // 寫狀態寄存器
void W25QXX_Write_Enable(void);  // 寫使能
void W25QXX_Write_Disable(void);  // 寫禁止
void W25QXX_Write_NoCheck(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);  // 寫入數據（不檢查）
void W25QXX_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);  // 讀取數據
void W25QXX_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);  // 寫入數據
void W25QXX_Erase_Chip(void);  // 擦除整個芯片
void W25QXX_Erase_Sector(uint32_t Dst_Addr);  // 擦除扇區
void W25QXX_Wait_Busy(void);  // 等待忙碌狀態解除
void W25QXX_PowerDown(void);  // 進入掉電模式
void W25QXX_WAKEUP(void);  // 喚醒

#endif
