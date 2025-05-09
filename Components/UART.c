/* USER CODE BEGIN Includes */
//#include "FreeRTOS.h"
//#include "task.h"
#include "main.h"
//#include "cmsis_os.h"
#include "UART.h"
#include "string.h"
#include "cJSON.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
//extern osThreadId Task03Handle;
//extern volatile uint8_t  CurrentValue[16];
uint8_t 	RX1Buffer[55];
uint8_t 	RX1Data;
uint8_t 	RX1Index=0;
uint8_t 	RX6Data;
char      RX6Buffer[512];
char    	CopyRX6Buffer[512];
char  		strBuffer[106];
uint8_t 	RX6Index=0;

uint16_t calculated_crc;
uint16_t received_crc;
/* USER CODE END Variables */

/* USER CODE BEGIN Function */

uint16_t crc16(uint8_t *buffer, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= buffer[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}
void UART_SEND1(uint8_t *data, uint8_t Address_485)
{
  int i;
  for (i = 0; i < 8; i++)
  {
    HAL_UART_Transmit_IT(&huart1, &data[i], 1);
    while (huart1.gState != HAL_UART_STATE_READY);
  }
  osDelay(100);
}
//void UART_SEND2(char *str)
//{
//	HAL_UART_Transmit_IT(&huart2, (uint8_t*)str,strlen(str));
//	while(huart2.gState!=HAL_UART_STATE_READY);	
//}
void UART_SEND6(char *str)
{
	HAL_UART_Transmit_IT(&huart6, (uint8_t*)str,strlen(str));
	while(huart6.gState!=HAL_UART_STATE_READY);	
}
//void UART_SEND4(char *str)
//{
//	HAL_UART_Transmit_IT(&huart6, (uint8_t*)str,strlen(str));
//	while(huart6.gState!=HAL_UART_STATE_READY);	
//}

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
	if (huart->Instance == USART1)
  {
    RX1Buffer[RX1Index] = RX1Data;
		RX1Index++;
    if (RX1Buffer[0] == 0x02 && RX1Index >=8 )
    {
      memset(RX1Buffer, 0, sizeof(RX1Buffer));
      RX1Index = 0;
    }
		else if (RX1Buffer[0] == 0x01 && RX1Index >= 53)
		{
				// 计算CRC16
				uint16_t calculated_crc = crc16(RX1Buffer, 51);
				uint16_t received_crc = (RX1Buffer[52] << 8) | RX1Buffer[51];

				// 验证CRC
				if (calculated_crc == received_crc)
				{
						for (uint8_t index = 0, i = 0; index < 8; index++)
						{
								CurrentValue[i++] = RX1Buffer[6 * (index + 1) + 1];
								CurrentValue[i++] = RX1Buffer[6 * (index + 1) + 2];
						}
				}
				
				memset(RX1Buffer, 0, sizeof(RX1Buffer));
				RX1Index = 0;
		}        
		else if (RX1Index >= 7 && (RX1Buffer[0] != 0x01 && RX1Buffer[0] != 0x02))
		{
				memset(RX1Buffer, 0, sizeof(RX1Buffer));
				RX1Index = 0;
		}
		
    HAL_UART_Receive_IT(&huart1, &RX1Data, 1);
  }
	if(huart->Instance==USART6)
	{
		// 處理從USART6接收到的數據
		RX6Buffer[RX6Index]=RX6Data;
		RX6Index++;
		
		// 檢查RX6Buffer中是否包含完整的JSON數據
		if(RX6Buffer[0]=='{'&&RX6Buffer[RX6Index-1]=='}')
		{
			// 複製完整的JSON數據到CopyRX6Buffer中
			memcpy(CopyRX6Buffer,RX6Buffer,RX6Index);
			// 從中斷程序中恢復Task03任務
			//xTaskResumeFromISR(Task03Handle);
			RX6Index=0;
		}	
		else if(RX6Buffer[0]!='{'||RX6Buffer[RX6Index-1]=='}')
		{	
			RX6Index=0;
		}
		// 重新啟動USART6的中斷接收
		HAL_UART_Receive_IT(&huart6, &RX6Data, 1);			
	}
}
/* USER CODE  END  Function */
