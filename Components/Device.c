/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "Device.h"
#include "UART.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Function */
uint8_t Relay1ON[]={0X02,0X05,0X00,0X00,0XFF,0X00,0X8C,0X09};
uint8_t Relay1OFF[]={0X02,0X05,0X00,0X00,0X00,0X00,0XCD,0XF9};
uint8_t Relay2ON[]={0X02,0X05,0X00,0X01,0XFF,0X00,0XDD,0XC9};
uint8_t Relay2OFF[]={0X02,0X05,0X00,0X01,0X00,0X00,0X9C,0X39};
uint8_t Relay3ON[]={0X02,0X05,0X00,0X02,0XFF,0X00,0X2D,0XC9};
uint8_t Relay3OFF[]={0X02,0X05,0X00,0X02,0X00,0X00,0X6C,0X39};
uint8_t Relay4ON[]={0X02,0X05,0X00,0X03,0XFF,0X00,0X7C,0X09};
uint8_t Relay4OFF[]={0X02,0X05,0X00,0X03,0X00,0X00,0X3D,0XF9};
uint8_t Relay5ON[]={0X02,0X05,0X00,0X04,0XFF,0X00,0XCD,0XC8};
uint8_t Relay5OFF[]={0X02,0X05,0X00,0X04,0X00,0X00,0X8C,0X38};
uint8_t Relay6ON[]={0X02,0X05,0X00,0X05,0XFF,0X00,0X9C,0X08};
uint8_t Relay6OFF[]={0X02,0X05,0X00,0X05,0X00,0X00,0XDD,0XF8};
uint8_t Relay7ON[]={0X02,0X05,0X00,0X06,0XFF,0X00,0X6C,0X08};
uint8_t Relay7OFF[]={0X02,0X05,0X00,0X06,0X00,0X00,0X2D,0XF8};
uint8_t Relay8ON[]={0X02,0X05,0X00,0X07,0XFF,0X00,0X3D,0XC8};
uint8_t Relay8OFF[]={0X02,0X05,0X00,0X07,0X00,0X00,0X7C,0X38};
uint8_t Relay9ON[]={0X02,0X05,0X00,0X08,0XFF,0X00,0X0D,0XCB};
uint8_t Relay9OFF[]={0X02,0X05,0X00,0X08,0X00,0X00,0X4C,0X3B};
uint8_t Relay10ON[]={0X02,0X05,0X00,0X09,0XFF,0X00,0X5C,0X0B};
uint8_t Relay10OFF[]={0X02,0X05,0X00,0X09,0X00,0X00,0X1D,0XFB};
uint8_t Relay11ON[]={0X02,0X05,0X00,0X0A,0XFF,0X00,0XAC,0X0B};
uint8_t Relay11OFF[]={0X02,0X05,0X00,0X0A,0X00,0X00,0XED,0XFB};
uint8_t Relay12ON[]={0X02,0X05,0X00,0X0B,0XFF,0X00,0XFD,0XCB};
uint8_t Relay12OFF[]={0X02,0X05,0X00,0X0B,0X00,0X00,0XBC,0X3B};
uint8_t Relay13ON[]={0X02,0X05,0X00,0X0C,0XFF,0X00,0X4C,0X0A};
uint8_t Relay13OFF[]={0X02,0X05,0X00,0X0C,0X00,0X00,0X0D,0XFA};
uint8_t Relay14ON[]={0X02,0X05,0X00,0X0D,0XFF,0X00,0X1D,0XCA};
uint8_t Relay14OFF[]={0X02,0X05,0X00,0X0D,0X00,0X00,0X5C,0X3A};
uint8_t Relay15ON[]={0X02,0X05,0X00,0X0E,0XFF,0X00,0XED,0XCA};
uint8_t Relay15OFF[]={0X02,0X05,0X00,0X0E,0X00,0X00,0XAC,0X3A};
uint8_t Relay16ON[]={0X02,0X05,0X00,0X0F,0XFF,0X00,0XBC,0X0A};
uint8_t Relay16OFF[]={0X02,0X05,0X00,0X0F,0X00,0X00,0XFD,0XFA};
volatile uint8_t controllerCommunicationInProgress;
osMutexId uart1Mutex;

void Device_Open(uint8_t DeviceID)
{
	// 设置控制器通信标志
  controllerCommunicationInProgress = 1;
  // 获取互斥锁,无限期等待
  //osMutexWait(uart1Mutex, osWaitForever);
	osDelay(100);
	switch(DeviceID)
	{
		case 1:
			UART_SEND1(Relay1ON,1);    //Device1 ON
			osDelay(100);
			UART_SEND1(Relay1OFF,1);
		  //osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 2:
			UART_SEND1(Relay3ON,1);    //Device2 ON
			osDelay(100);
			UART_SEND1(Relay3OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;	
		case 3:
			UART_SEND1(Relay5ON,1);   //Device3 ON
			osDelay(100);
			UART_SEND1(Relay5OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;		
		case 4:
			UART_SEND1(Relay7ON,1);    //Device4 ON
			osDelay(100);
			UART_SEND1(Relay7OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 5:
			UART_SEND1(Relay9ON,1);    //Device5 ON
			osDelay(100);
			UART_SEND1(Relay9OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 6:
			UART_SEND1(Relay11ON,1);   //Device6 ON
			osDelay(100);
			UART_SEND1(Relay11OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 7:
			UART_SEND1(Relay13ON,1);    //Device7 ON
			osDelay(100);
			UART_SEND1(Relay13OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 8:
			UART_SEND1(Relay15ON,1);   //Device8 ON
			osDelay(100);
			UART_SEND1(Relay15OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		default:
			break;	
	}
}
void Device_Close(uint8_t DeviceID)
{
	// 设置控制器通信标志
  controllerCommunicationInProgress = 1;
  // 获取互斥锁,无限期等待
  //osMutexWait(uart1Mutex, osWaitForever);
	osDelay(100);
	switch(DeviceID)
	{
		case 1:
			UART_SEND1(Relay2ON,1);    //Device1 OFF
			osDelay(200);
			UART_SEND1(Relay2OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 2:
			UART_SEND1(Relay4ON,1);    //Device2 OFF
			osDelay(200);
			UART_SEND1(Relay4OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;	
		case 3:
			UART_SEND1(Relay6ON,1);   //Device3 OFF
			osDelay(200);
			UART_SEND1(Relay6OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;		
		case 4:
			UART_SEND1(Relay8ON,1);    //Device4 OFF
			osDelay(200);
			UART_SEND1(Relay8OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 5:
			UART_SEND1(Relay10ON,1);    //Device5 OFF
			osDelay(200);
			UART_SEND1(Relay10OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 6:
			UART_SEND1(Relay12ON,1);   //Device6 OFF
			osDelay(200);
			UART_SEND1(Relay12OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 7:
			UART_SEND1(Relay14ON,1);    //Device7 OFF
			osDelay(200);
			UART_SEND1(Relay14OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		case 8:
			UART_SEND1(Relay16ON,1);   //Device8 OFF
			osDelay(200);
			UART_SEND1(Relay16OFF,1);
			//osMutexRelease(uart1Mutex);
			controllerCommunicationInProgress = 0;
			break;
		default:
			break;	
	}
}
//void Device_Open(uint8_t DeviceID)
//{
//	switch(DeviceID)
//	{
//		case 1:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);    //Device1 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
//			break;
//		case 2:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);    //Device2 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET); 
//			break;	
//		case 3:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);   //Device3 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
//			break;		
//		case 4:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);    //Device4 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
//			break;
//		case 5:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);    //Device5 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
//			break;
//		case 6:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);   //Device6 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
//			break;
//		case 7:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);    //Device7 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
//			break;
//		case 8:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);   //Device8 ON
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
//			break;
//		default:
//			break;	
//	}
//}
//void Device_Close(uint8_t DeviceID)
//{
//	switch(DeviceID)
//	{
//		case 1:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);    //Device1 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
//			break;
//		case 2:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);   //Device2 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
//			break;	
//		case 3:
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);    //Device3 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
//			break;
//		case 4:
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);    //Device4 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);
//			break;	
//		case 5:
//			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);    //Device5 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
//			break;
//		case 6:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);   //Device6 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
//			break;
//		case 7:
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);    //Device7 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
//			break;
//		case 8:
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);    //Device8 OFF
//			osDelay(100);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
//			break;		
//		default:
//			break;	
//	}
//}

/* USER CODE END Function */
