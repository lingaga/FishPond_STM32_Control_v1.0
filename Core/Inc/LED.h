#ifndef _LED_H_
#define	_LED_H_

/* USER CODE BEGIN Includes */
#include "gpio.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Define */
#define LED1_Pin GPIO_PIN_3
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOA
#define LED4_Pin GPIO_PIN_6
#define LED4_GPIO_Port GPIOA
#define LED5_Pin GPIO_PIN_7
#define LED5_GPIO_Port GPIOA
#define LED6_Pin GPIO_PIN_0
#define LED6_GPIO_Port GPIOB
#define LED7_Pin GPIO_PIN_1
#define LED7_GPIO_Port GPIOB
#define LED8_Pin GPIO_PIN_2
#define LED8_GPIO_Port GPIOB
#define StateLED_Pin GPIO_PIN_13
#define StateLED_GPIO_Port GPIOC


#define RED_Pin GPIO_PIN_0
#define RED_GPIO_Port GPIOA
#define GREEN_Pin GPIO_PIN_1
#define GREEN_GPIO_Port GPIOA
#define BLUE_Pin GPIO_PIN_2
#define BLUE_GPIO_Port GPIOA

#define LED_R(n)			(n?HAL_GPIO_WritePin(RED_GPIO_Port,RED_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(RED_GPIO_Port,RED_Pin,GPIO_PIN_RESET))
#define LED_G(n)			(n?HAL_GPIO_WritePin(GREEN_GPIO_Port,GREEN_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(GREEN_GPIO_Port,GREEN_Pin,GPIO_PIN_RESET))
#define LED_B(n)			(n?HAL_GPIO_WritePin(BLUE_GPIO_Port,BLUE_Pin,GPIO_PIN_SET):HAL_GPIO_WritePin(BLUE_GPIO_Port,BLUE_Pin,GPIO_PIN_RESET))
/* USER CODE END Define */

/* USER CODE BEGIN FunctionPrototypes */
void RGB(char color);
void LED_ON(uint8_t select);
void LED_OFF(uint8_t select);
void LED_SHINE();
/* USER CODE END FunctionPrototypes */
#endif
