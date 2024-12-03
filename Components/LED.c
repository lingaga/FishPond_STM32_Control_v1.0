/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "LED.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Function */
void RGB(char color)
{
	//1:Red 2:Green 3:Blue 4:Purple
	switch(color)
	{
		case 'R':
			LED_R(0);
			LED_G(1);
			LED_B(1);
			break;
		case 'G':
			LED_R(1);
			LED_G(0);
			LED_B(1);
			break;		
		case 'B':
			LED_R(1);
			LED_G(1);
			LED_B(0);
			break;		
		case 'P':
			LED_R(0);
			LED_G(1);
			LED_B(0);
			break;	
		default:
			break;		
	}
}
void LED_ON(uint8_t select)
{
	switch(select)
	{
		case 1:
			HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);   //LED1 ON
			break;
		case 2:
			HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);   //LED2 ON
			break;	
		case 3:
			HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_SET);   //LED3 ON
			break;	
		case 4:
			HAL_GPIO_WritePin(LED4_GPIO_Port,LED4_Pin,GPIO_PIN_SET);   //LED4 ON
			break;	
		case 5:
			HAL_GPIO_WritePin(LED5_GPIO_Port,LED5_Pin,GPIO_PIN_SET);   //LED5 ON
			break;	
		case 6:
			HAL_GPIO_WritePin(LED6_GPIO_Port,LED6_Pin,GPIO_PIN_SET);   //LED6 ON
			break;
		case 7:
			HAL_GPIO_WritePin(LED7_GPIO_Port,LED7_Pin,GPIO_PIN_SET);   //LED7 ON
			break;	
		case 8:
			HAL_GPIO_WritePin(LED8_GPIO_Port,LED8_Pin,GPIO_PIN_SET);   //LED8 ON
			break;	
		default:
			break;		
	}		
}
void LED_OFF(uint8_t select)
{
	switch(select)
	{
		case 1:
			HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);   //LED1 OFF
			break;
		case 2:
			HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);   //LED2 OFF
			break;	
		case 3:
			HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_RESET);   //LED3 OFF
			break;	
		case 4:
			HAL_GPIO_WritePin(LED4_GPIO_Port,LED4_Pin,GPIO_PIN_RESET);   //LED4 OFF
			break;	
		case 5:
			HAL_GPIO_WritePin(LED5_GPIO_Port,LED5_Pin,GPIO_PIN_RESET);   //LED5 OFF
			break;	
		case 6:
			HAL_GPIO_WritePin(LED6_GPIO_Port,LED6_Pin,GPIO_PIN_RESET);   //LED6 OFF
			break;
		case 7:
			HAL_GPIO_WritePin(LED7_GPIO_Port,LED7_Pin,GPIO_PIN_RESET);   //LED7 OFF
			break;	
		case 8:
			HAL_GPIO_WritePin(LED8_GPIO_Port,LED8_Pin,GPIO_PIN_RESET);   //LED8 OFF
			break;	
		default:
			break;		
	}		
}

void LED_SHINE()
{
	HAL_GPIO_WritePin(StateLED_GPIO_Port,StateLED_Pin,GPIO_PIN_SET);   //D2 ON
	HAL_Delay(1000);
	HAL_GPIO_WritePin(StateLED_GPIO_Port,StateLED_Pin,GPIO_PIN_RESET);   //D2 OFF
	HAL_Delay(1000);
}
/* USER CODE END Function */
