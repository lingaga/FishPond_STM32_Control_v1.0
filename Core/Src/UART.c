/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "UART.h"
#include "string.h"
#include "cJSON.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
extern osThreadId Task03Handle;
volatile uint8_t  CurrentValue[16];
uint8_t 	RX1Buffer[55];
uint8_t 	RX2Data;
char      RX2Buffer[256];
char    	CopyRX2Buffer[256];
uint8_t 	RX2Index=0;

/* USER CODE END Variables */

/* USER CODE BEGIN Function */
void UART_SEND1(uint8_t *data)
{
	int i;
	for(i=0;i<8;i++)
	{
		HAL_UART_Transmit_IT(&huart1, &data[i],1);
		while(huart1.gState!=HAL_UART_STATE_READY);
	}
}
void UART_SEND2(char *str)
{
	HAL_UART_Transmit_IT(&huart2, (uint8_t*)str,strlen(str));
	while(huart2.gState!=HAL_UART_STATE_READY);	
}
void UART_SEND3(char *str)
{
	HAL_UART_Transmit_IT(&huart3, (uint8_t*)str,strlen(str));
	while(huart3.gState!=HAL_UART_STATE_READY);	
}
void UART_SEND4(char *str)
{
	HAL_UART_Transmit_IT(&huart6, (uint8_t*)str,strlen(str));
	while(huart6.gState!=HAL_UART_STATE_READY);	
}

/**
 * @brief UART接收完成中斷回調函式
 *
 * @param huart UART句柄
 *
 * 此函式是UART接收完成中斷的回調函式，用於處理接收到的數據。
 * 根據不同的UART實例執行不同的操作。
 *
 * 對於USART1:
 * 1. 從RX1Buffer中提取8個2位元組的數值，並存入CurrentValue陣列中。
 * 2. 重新啟動USART1的中斷接收，等待下一次接收。
 *
 * 對於USART2:
 * 1. 將接收到的單個位元組存入RX2Buffer中。
 * 2. 檢查RX2Buffer中是否包含完整的JSON數據(以'{'開始，以'}'結束)。
 *    - 如果是完整的JSON數據，則複製RX2Buffer的內容到CopyRX2Buffer中，
 *      並從中斷程序中恢復Task03任務。
 *    - 如果不是完整的JSON數據，則清空RX2Buffer。
 * 3. 重新啟動USART2的中斷接收，等待下一個位元組的接收。
 *
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UNUSED(huart);
	if(huart->Instance==USART1)
	{
		
		// 處理從USART1接收到的數據
		for(uint8_t index=0,i=0;index<8;index++)
		{
			CurrentValue[i++]=RX1Buffer[6*(index+1)+1];
			CurrentValue[i++]=RX1Buffer[6*(index+1)+2];
		}
		
		// 重新啟動USART1的中斷接收
		HAL_UART_Receive_IT(&huart1, RX1Buffer, 53);
	}
	if(huart->Instance==USART2)
	{
		
		// 處理從USART2接收到的數據
		RX2Buffer[RX2Index]=RX2Data;
		RX2Index++;
		
		// 檢查RX2Buffer中是否包含完整的JSON數據
		if(RX2Buffer[0]=='{'&&RX2Buffer[RX2Index-1]=='}')
		{
			// 複製完整的JSON數據到CopyRX2Buffer中
			memcpy(CopyRX2Buffer,RX2Buffer,RX2Index);
			// 從中斷程序中恢復Task03任務
			xTaskResumeFromISR(Task03Handle);
			RX2Index=0;
		}	
		else if(RX2Buffer[0]!='{'||RX2Buffer[RX2Index-1]=='}')
		{	
			RX2Index=0;
		}
		// 重新啟動USART2的中斷接收
		HAL_UART_Receive_IT(&huart2, &RX2Data, 1);			
	}
}
/* USER CODE  END  Function */
