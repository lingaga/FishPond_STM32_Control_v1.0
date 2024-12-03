#ifndef _Device_H_
#define	_Device_H_

/* USER CODE BEGIN Includes */
#include "stdint.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Define */
#define Device1_ON_Pin GPIO_PIN_1
#define Device1_ON_GPIO_Port GPIOA
#define Device1_OFF_Pin GPIO_PIN_5
#define Device1_OFF_GPIO_Port GPIOA

#define Device2_ON_Pin GPIO_PIN_1
#define Device2_ON_GPIO_Port GPIOB
#define Device2_OFF_Pin GPIO_PIN_10
#define Device2_OFF_GPIO_Port GPIOB

#define Device3_ON_Pin GPIO_PIN_14
#define Device3_ON_GPIO_Port GPIOB
#define Device3_OFF_Pin GPIO_PIN_8
#define Device3_OFF_GPIO_Port GPIOC

#define Device4_ON_Pin GPIO_PIN_6
#define Device4_ON_GPIO_Port GPIOA
#define Device4_OFF_Pin GPIO_PIN_4
#define Device4_OFF_GPIO_Port GPIOC

#define Device5_ON_Pin GPIO_PIN_4
#define Device5_ON_GPIO_Port GPIOB
#define Device5_OFF_Pin GPIO_PIN_2
#define Device5_OFF_GPIO_Port GPIOD

#define Device6_ON_Pin GPIO_PIN_15
#define Device6_ON_GPIO_Port GPIOA
#define Device6_OFF_Pin GPIO_PIN_11
#define Device6_OFF_GPIO_Port GPIOA

#define Device7_ON_Pin GPIO_PIN_5
#define Device7_ON_GPIO_Port GPIOB
#define Device7_OFF_Pin GPIO_PIN_3
#define Device7_OFF_GPIO_Port GPIOB

#define Device8_ON_Pin GPIO_PIN_12
#define Device8_ON_GPIO_Port GPIOA
#define Device8_OFF_Pin GPIO_PIN_8
#define Device8_OFF_GPIO_Port GPIOA
/* USER CODE  END  Define */

/* USER CODE BEGIN FunctionPrototypes */
void Device_Open(uint8_t DeviceID);
void Device_Close(uint8_t DeviceID);
/* USER CODE  END  FunctionPrototypes */

#endif
