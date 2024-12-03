#ifndef _UART_H_
#define	_UART_H_

/* USER CODE BEGIN Includes */
#include "usart.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
extern uint8_t  RX1Buffer[55];
extern uint8_t  RX1Index;
extern volatile uint8_t  CurrentValue[16];
extern uint8_t  RX2Data;
extern char     RX2Buffer[256];
extern char    	CopyRX2Buffer[256];
extern uint8_t  RX2Index;
/* USER CODE END Variables */

/* USER CODE BEGIN FunctionPrototypes */
void UART_SEND1(uint8_t *data);
void UART_SEND2(char *str);
void UART_SEND3(char *str);
void UART_SEND4(char *str);
/* USER CODE END FunctionPrototypes */

#endif
