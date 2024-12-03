/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "EEPROM.h"
#include "TimeSwitch.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "UART.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
uint8_t DeviceStatus[8];
uint16_t TimeSwitch_ADD[8]={0x0000,0x0008,0x0010,0x0018,0x0020,0x0028,0x0030,0x0038}; //8組定時開/關儲存位址
uint8_t TimeSwitchData[8];
/* USER CODE END Variables */

/* USER CODE BEGIN Function */
/**
 * @brief 將指定的定時開關設定寫入EEPROM
 *
 * @param timeSwitch 指向存放定時開關設定的陣列
 * @param DeviceID 要寫入EEPROM的定時開關設定索引
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: 寫入成功
 *         - HAL_ERROR: 寫入失敗
 *
 * 此函式會將timeSwitch陣列中索引為DeviceID的定時開關設定寫入EEPROM。
 * 首先，它會將該定時開關設定複製到臨時緩衝區TimeSwitchData中。
 * 然後，它會呼叫Write_EEPROM函式，將TimeSwitchData的內容寫入對應的EEPROM位址。
 * 如果寫入成功，函式會返回HAL_OK，否則返回HAL_ERROR。
 */
HAL_StatusTypeDef SaveTimerSwitchToEEPROM(TimerSwitch* timeSwitch,uint8_t DeviceID)
{
  memcpy(TimeSwitchData, &timeSwitch[DeviceID], sizeof(TimeSwitchData));
	if(Write_EEPROM(TimeSwitch_ADD[DeviceID],TimeSwitchData,sizeof(TimeSwitchData))==HAL_OK)
		return HAL_OK;
	else
		return HAL_ERROR;
}
/**
* @brief 從EEPROM中讀取8組定時開關資訊
 *
 * @param timeSwitch 指向存放讀取定時開關設定的陣列
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: 讀取成功
 *         - HAL_ERROR: 讀取失敗
 *
 * 此函式會從EEPROM中讀取8組計時器開關設定，並將它們存放到傳入的timeSwitch陣列中。
 * 每次讀取前會先延遲5ms，以確保EEPROM的存取有足夠的時間。
 * 如果在讀取過程中發生任何錯誤，函式會立即返回HAL_ERROR。
 */
HAL_StatusTypeDef ReadTimerSwitchFromEEPROM(TimerSwitch* timeSwitch)
{
	uint8_t Index;
	for(Index=0;Index<8;Index++)
	{
		if (Read_EEPROM(TimeSwitch_ADD[Index], TimeSwitchData, sizeof(TimeSwitchData)) != HAL_OK)
			return HAL_ERROR;
		osDelay(5); 
    memcpy(&timeSwitch[Index], TimeSwitchData, sizeof(TimeSwitchData));
	}
	return HAL_OK;
}

/* USER CODE  END  Function */
