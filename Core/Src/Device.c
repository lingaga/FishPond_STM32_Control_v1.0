/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "Device.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Function */
void Device_Open(uint8_t DeviceID)
{
	switch(DeviceID)
	{
		case 1:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);    //Device1 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);    //Device2 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET); 
			break;	
		case 3:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);   //Device3 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
			break;		
		case 4:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);    //Device4 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
			break;
		case 5:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);    //Device5 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
			break;
		case 6:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);   //Device6 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
			break;
		case 7:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);    //Device7 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
			break;
		case 8:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);   //Device8 ON
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
			break;
		default:
			break;	
	}
}
void Device_Close(uint8_t DeviceID)
{
	switch(DeviceID)
	{
		case 1:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);    //Device1 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);   //Device2 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
			break;	
		case 3:
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);    //Device3 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
			break;
		case 4:
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);    //Device4 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);
			break;	
		case 5:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);    //Device5 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			break;
		case 6:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);   //Device6 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
			break;
		case 7:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);    //Device7 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
			break;
		case 8:
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);    //Device8 OFF
			osDelay(100);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
			break;		
		default:
			break;	
	}
}


/* USER CODE END Function */
