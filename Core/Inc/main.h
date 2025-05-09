/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define StateLED_Pin GPIO_PIN_13
#define StateLED_GPIO_Port GPIOC
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

/* USER CODE BEGIN Private defines */
#define GetNetTimeTask  (0x0001)
#define SendDataTask    (0x0002)
#define SendLogTask     (0x0003)
#define Waiting         (0x00FF)
#define Manual					(0x0004)
#define Remote					(0x0005)

#define W_EEPROM        (0x0010)
#define R_EEPROM        (0x0011)
#define OPEN_ERROR1     (0x0000)
#define OPEN_ERROR2     (0x0001)
#define OPEN_ERROR3     (0x0002)
#define OPEN_ERROR4     (0x0003)
#define OPEN_ERROR5     (0x0004)
#define OPEN_ERROR6     (0x0005)
#define OPEN_ERROR7     (0x0006)
#define OPEN_ERROR8     (0x0007)

#define W25QXX_CS_Pin GPIO_PIN_12
#define W25QXX_CS_GPIO_Port GPIOB
extern volatile uint8_t SetTime;


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
