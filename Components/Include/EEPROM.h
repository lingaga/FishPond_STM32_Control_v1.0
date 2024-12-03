#ifndef _EEPROM_H_
#define	_EEPROM_H_

/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "TimeSwitch.h"
/* USER CODE END Includes */ 

/* USER CODE BEGIN struct */

/* USER CODE   END struct */ 

/* USER CODE BEGIN Define */
#define EEPROM_ADDR 0xA0

#define Device_Status_ADDR 0x80
#define Write_EEPROM( MemAddress, pData, Size) HAL_I2C_Mem_Write_IT(&hi2c1, EEPROM_ADDR, (MemAddress), I2C_MEMADD_SIZE_16BIT, (pData), (Size))
#define Read_EEPROM( MemAddress, pData, Size)  HAL_I2C_Mem_Read_IT(&hi2c1, EEPROM_ADDR, (MemAddress), I2C_MEMADD_SIZE_16BIT, (pData), (Size))
/* USER CODE  END  Define */

/* USER CODE BEGIN Variables */
extern uint8_t DeviceStatus[8];
extern uint16_t TimeSwitch_ADD[8];
extern uint8_t TimeSwitchData[8];
/* USER CODE END Variables */

/* USER CODE BEGIN FunctionPrototypes */
HAL_StatusTypeDef SaveTimerSwitchToEEPROM(TimerSwitch* ,uint8_t );
HAL_StatusTypeDef ReadTimerSwitchFromEEPROM(TimerSwitch*);
/* USER CODE END FunctionPrototypes */
#endif
