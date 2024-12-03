/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UART.h"
#include "iwdg.h"
#include "LED.h"
#include "Device.h"
#include "EEPROM.h"
#include "TimeSwitch.h"
#include "cJSON.h"
#include "xRTC.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern   uint8_t RX1Buffer[55];
extern   uint8_t RX1Data;
extern   uint8_t RX2Data;
volatile uint8_t CurrentValue[16];
volatile uint8_t DeviceOpenFlag[8]={0};
volatile uint8_t StateErrorFlag[9]={0};
volatile uint8_t AppOnFlag[9]={0};
volatile uint8_t Task1Flag=Waiting;
volatile uint8_t SetTime=0;
volatile uint8_t ModeTask=Remote;//Manual手動 、 Remote遠端
uint8_t volt[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x18, 0x45, 0xC0};
int TimeCount=0;
TimerSwitch  xTimeSwitch[8];

// 定義UART互斥
extern osMutexId uart1Mutex;
extern volatile uint8_t controllerCommunicationInProgress;
osMutexDef(uart1Mutex);

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId Task01Handle;
osThreadId Task02Handle;
osThreadId FeedDogTaskHandle;
osThreadId Task03Handle;
osThreadId Task04Handle;
osMessageQId LogQueueHandle;
osTimerId Timer01Handle;
osSemaphoreId sendDataBinarySemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Log(char* message);
void GetNetTime(void);
void UpdateData(void);
void UpdateState(void);
void UpdateDataDebug(void);
void ParseDateTimeString(const char *, TimerSwitch *,uint8_t);
uint8_t TimeCompare(uint8_t ID);
uint32_t timestamp[8] = {0};

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask01(void const * argument);
void StartTask02(void const * argument);
void StartFeedDogTask(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);
void vTimer01(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
	uart1Mutex = osMutexCreate(osMutex(uart1Mutex));
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of sendDataBinarySem */
  osSemaphoreDef(sendDataBinarySem);
  sendDataBinarySemHandle = osSemaphoreCreate(osSemaphore(sendDataBinarySem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of Timer01 */
  osTimerDef(Timer01, vTimer01);
  Timer01Handle = osTimerCreate(osTimer(Timer01), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of LogQueue */
  osMessageQDef(LogQueue, 10, uint16_t);
  LogQueueHandle = osMessageCreate(osMessageQ(LogQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of Task01 */
  osThreadDef(Task01, StartTask01, osPriorityIdle, 0, 256);
  Task01Handle = osThreadCreate(osThread(Task01), NULL);

  /* definition and creation of Task02 */
  osThreadDef(Task02, StartTask02, osPriorityIdle, 0, 256);
  Task02Handle = osThreadCreate(osThread(Task02), NULL);

  /* definition and creation of FeedDogTask */
  osThreadDef(FeedDogTask, StartFeedDogTask, osPriorityIdle, 0, 128);
  FeedDogTaskHandle = osThreadCreate(osThread(FeedDogTask), NULL);

  /* definition and creation of Task03 */
  osThreadDef(Task03, StartTask03, osPriorityIdle, 0, 512);
  Task03Handle = osThreadCreate(osThread(Task03), NULL);

  /* definition and creation of Task04 */
  osThreadDef(Task04, StartTask04, osPriorityIdle, 0, 256);
  Task04Handle = osThreadCreate(osThread(Task04), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
	* 這個任務主要負責在系統啟動時執行以下初始化操作:
	*
	* 1. 開啟UART1和UART2的中斷接收功能
	* 2. 從EEPROM讀取設備狀態、定時器開關設定
	* 3. 啟動定時器Timer01並設置Task1Flag為GetNetTimeTask
	* 4. 刪除當前任務
**/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	uint16_t LogMessage=R_EEPROM;
	uint8_t Index=0;
	
	HAL_UART_Receive_IT(&huart1, &RX1Data, 1);
	HAL_UART_Receive_IT(&huart6, &RX2Data, 1);	
	
	// 從 EEPROM 讀取設備狀態
	if(Read_EEPROM(Device_Status_ADDR, DeviceStatus, sizeof(DeviceStatus)) != HAL_OK)
	{	
		osDelay(50);
		xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
	}
	else
	{
		osDelay(100);
		while(Index<8)
		{
			if(DeviceStatus[Index])
			{		
				AppOnFlag[Index]=1;
				Device_Open(Index+1);
				LED_ON(Index+1);
			}
			Index++;		
		}
	}
	
	//從 EEPROM 讀取定時器開關設定
	if(ReadTimerSwitchFromEEPROM(xTimeSwitch) != HAL_OK)
	{	
		osDelay(50);
		xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
	}
	osTimerStart(Timer01Handle,1000);
	Task1Flag=GetNetTimeTask;

	vTaskDelete(NULL);
  /* Infinite loop */
  for(;;)
  {
		
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask01 */
/**
* @brief Function implementing the Task01 thread.
* @param argument: Not used
* @retval None
* 這個任務有以下主要功能:
*
* 1. 獲取網路時間
* 2. 發送數據
* 3. 處理日誌佇列
*/
/* USER CODE END Header_StartTask01 */
void StartTask01(void const * argument)
{
  /* USER CODE BEGIN StartTask01 */
		uint16_t LogMessage=0;
  /* Infinite loop */
  for(;;) 
	{
		if(Task1Flag==GetNetTimeTask)
		{
			//LED_SHINE();
			SetTime=HAL_ERROR;
			GetNetTime();
			osDelay(1000);
			while(SetTime!=HAL_OK)
			{
				osDelay(10000);
				GetNetTime();	
			}
			vTaskResume(Task04Handle);
			RGB('G');	
			Task1Flag=Waiting;
		}
		else if(Task1Flag==SendDataTask)
		{	
			if(ModeTask==Remote)//遠端
			{
				UpdateState();
			}
			else if(ModeTask==Manual)//手動
			{
				//UpdateDataDebug();
				UpdateData();
			}
			Task1Flag=Waiting;
		}
		while(uxQueueMessagesWaiting(LogQueueHandle) != 0)
		{
			xQueueReceive(LogQueueHandle, &LogMessage, portMAX_DELAY);
			switch(LogMessage)
			{
				case W_EEPROM:
						Log("EEPROM write failure");
						break;
				case R_EEPROM:
						Log("EEPROM read failure");
						break;
				case OPEN_ERROR1:
				case OPEN_ERROR2:
				case OPEN_ERROR3:
				case OPEN_ERROR4:
				case OPEN_ERROR5:
				case OPEN_ERROR6:
				case OPEN_ERROR7:
				case OPEN_ERROR8:
						StateErrorFlag[LogMessage]=1;
						break;
				default:
						break;
			}
		}
    osDelay(100);
  }
  /* USER CODE END StartTask01 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief 數據處理和設備狀態更新任務
*
* @param argument 未使用的參數
*
* 這個任務的主要功能是處理感測器數據並更新相應設備的狀態。它會執行以下操作:
*
* 1. 每兩秒會通過序列埠發送讀取ADC 8通道的要求。
* 2. 遍歷CurrentValue陣列中的 8 個通道數據:
*    - 如果通道數據超過閾值且對應設備當前未開啟,則開啟該設備並更新DeviceStatus陣列和EEPROM中的狀態。
*    - 如果通道數據未超過閾值且對應設備當前已開啟,則關閉該設備並更新DeviceStatus陣列和EEPROM中的狀態。
*    - 如果設備狀態發生變化且當前任務處於等待狀態,則設置標誌以發送數據。
*
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
	uint16_t LogMessage=W_EEPROM;
	uint8_t temp, Index=0;
	
	
  /* Infinite loop */
  for(;;) 
	{
		if(TimeCount&1)
		{
			if(!controllerCommunicationInProgress)
			{	
				UART_SEND1(volt,0);		
			}
		}
		else
		{
			Index=0;
			while(Index<8)
			{
				temp=Index<<1;
				//if((CurrentValue[temp]==0x00 && CurrentValue[temp+1]>0x64) || CurrentValue[temp]>0x00)//Current>=0.1A
				if((CurrentValue[temp]==0x03 && CurrentValue[temp+1]>0xE8) || CurrentValue[temp]>0x03)//Current > 1A
				{
					if(!DeviceStatus[Index])
					{
						LED_ON(Index+1);
						DeviceStatus[Index]=1;
						LogMessage=W_EEPROM;
						if(Write_EEPROM(Device_Status_ADDR, DeviceStatus, sizeof(DeviceStatus)) != HAL_OK)
							xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
						osDelay(100);
						if(Task1Flag==Waiting)
							Task1Flag=SendDataTask;
					}
				}
				else
				{
					if(DeviceStatus[Index]) 
					{
						LED_OFF(Index+1);
						DeviceStatus[Index]=0;
						LogMessage=W_EEPROM;
						if(Write_EEPROM(Device_Status_ADDR, DeviceStatus, sizeof(DeviceStatus)) != HAL_OK)
							xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
						osDelay(100);
						if(Task1Flag==Waiting)
							Task1Flag=SendDataTask;
					}
				}
				if(DeviceOpenFlag[Index]>4)
				{
					LogMessage=Index;
					DeviceOpenFlag[Index]=0;
					Device_Close(Index+1);
					AppOnFlag[Index]=0;
					xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);				
				}
				Index++;			
			}
		}
    osDelay(500);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartFeedDogTask */
/**
* @brief Function implementing the FeedDogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFeedDogTask */
void StartFeedDogTask(void const * argument)
{
  /* USER CODE BEGIN StartFeedDogTask */
  /* Infinite loop */
  for(;;)
  {
		HAL_IWDG_Refresh(&hiwdg);
    osDelay(50);
  }
  /* USER CODE END StartFeedDogTask */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the Task03 thread.
* @param argument: Not used
* @retval None
* 這個任務主要功能是解析從序列埠接收到的JSON格式數據,並根據操作類型執行相應的操作。
* 操作包括獲取網路時間、發送數據、開啟/關閉設備以及設置設備開啟/關閉時間等。
*
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
	uint8_t DeviceID=0;
	uint16_t LogMessage=W_EEPROM;
  /* Infinite loop */
  for(;;)
  {
		cJSON *json,*item;
		json=cJSON_Parse(CopyRX2Buffer);
		if(json!=NULL)
		{
			char Operating[20];
			char Mode[20];
			item=cJSON_GetObjectItem(json,"Operating");
			if(item != NULL)
			{
				memcpy(Operating,item->valuestring,strlen(item->valuestring));
				if(!strcmp(Operating,"ReturnTime"))
				{
					item=cJSON_GetObjectItem(json,"CurrentTime");
					memcpy(NetTime,item->valuestring,strlen(item->valuestring));
					
					if(((NetTime[17]-'0')*10+NetTime[18]-'0')>0)
						RTC_Init();
					else
						Task1Flag=GetNetTimeTask; 
				}
				else if(!strcmp(Operating,"GetData"))
				{			
					if(Task1Flag==Waiting)
						Task1Flag=SendDataTask;
				}
				else if(!strcmp(Operating,"DeviceOpen"))
				{
					item=cJSON_GetObjectItem(json,"DeviceID");
					DeviceID=(*item->valuestring)-'0';
					Device_Open(DeviceID);
					AppOnFlag[DeviceID-1]=1;
					DeviceOpenFlag[DeviceID-1]=1;
					timestamp[DeviceID-1] = HAL_GetTick();  // 抓取當前時間
					DeviceID=0;
				}
				else if(!strcmp(Operating,"DeviceClose"))
				{
					item=cJSON_GetObjectItem(json,"DeviceID");
					DeviceID=(*item->valuestring)-'0';
					Device_Close(DeviceID);
					AppOnFlag[DeviceID-1]=0;
					timestamp[DeviceID-1] = HAL_GetTick();  //抓取當前時間
					DeviceID=0;
				}
				else if(!strcmp(Operating,"SetTurnOnTime"))
				{
					item=cJSON_GetObjectItem(json,"DeviceID");
					DeviceID=(*item->valuestring)-'1';
					item=cJSON_GetObjectItem(json,"DateTime");
					ParseDateTimeString(item->valuestring,&xTimeSwitch[DeviceID],On);
					if(SaveTimerSwitchToEEPROM(xTimeSwitch,DeviceID)!=HAL_OK)
						xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
				}
				else if(!strcmp(Operating,"SetTurnOffTime"))
				{
					item=cJSON_GetObjectItem(json,"DeviceID");
					DeviceID=(*item->valuestring)-'1';
					item=cJSON_GetObjectItem(json,"DateTime");
					ParseDateTimeString(item->valuestring,&xTimeSwitch[DeviceID],Off);
					if(SaveTimerSwitchToEEPROM(xTimeSwitch,DeviceID)!=HAL_OK)
						xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);
				}
				memset(Operating, '\0', sizeof(Operating));
			}
			else 
			{
				item=cJSON_GetObjectItem(json,"Mode");
				if(item != NULL) {
					memcpy(Mode,item->valuestring,strlen(item->valuestring));
					if(!strcmp(item->valuestring,"Manual"))
					{
						ModeTask=Manual;

					}
					else if(!strcmp(item->valuestring,"Remote"))
					{
						ModeTask=Remote;
						for(int i=0;i<8;i++)
						{
							StateErrorFlag[i]=0;
							if(DeviceStatus[i]==1)
							{
								AppOnFlag[i]=1;
							}
							else if(DeviceStatus[i]==0)
							{
								AppOnFlag[i]=0;
							}
						}
					}
				}
			}
			cJSON_Delete(json);
		}	
		vTaskSuspend(Task03Handle);		
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief 定時器開關任務
*
* @param argument 未使用的參數
*
* 這個任務主要功能是根據 xTimeSwitch 陣列中的設定時間來開啟或關閉設備。
* 它會定期獲取當前時間,並比較 xTimeSwitch 中的時間設定與當前時間。
* 如果已到達設定時間,它會根據設定開啟或關閉對應的設備。
*
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
	uint8_t DeviceID=0;
	uint16_t LogMessage=W_EEPROM;
  /* Infinite loop */
  for(;;)
  {
		GET_Time();
		if(Time_Offset())
			Task1Flag=GetNetTimeTask;
		DeviceID=0;
		while(DeviceID<8)
		{
			if(xTimeSwitch[DeviceID].confirm&&TimeCompare(DeviceID))
			{
				if(xTimeSwitch[DeviceID].isOn==1)
				{
					Device_Open(DeviceID+1);	
					AppOnFlag[DeviceID]=1;
					ModeTask=Remote;
				}
				else
				{
					Device_Close(DeviceID+1);				
					AppOnFlag[DeviceID]=0;
					ModeTask=Remote;
				}	
				xTimeSwitch[DeviceID].confirm=0;
				if(SaveTimerSwitchToEEPROM(xTimeSwitch,DeviceID)!=HAL_OK)
					xQueueSendToBack(LogQueueHandle, &LogMessage, portMAX_DELAY);			
			}
			DeviceID++;
		}
    osDelay(100);
  }
  /* USER CODE END StartTask04 */
}

/* vTimer01 function */
void vTimer01(void const * argument)
{
  /* USER CODE BEGIN vTimer01 */
	uint8_t index=0;
	TimeCount++;
	if(TimeCount>59)
	{		
		TimeCount=0;	
	}
	if(TimeCount%5==0)
	{
		Task1Flag=SendDataTask;
	}	
	while(index<8)
	{	
		if(DeviceStatus[index]>0&&DeviceOpenFlag[index]>0) 
		{
			DeviceOpenFlag[index]=0;
		}
		else if(DeviceStatus[index]==0&&DeviceOpenFlag[index]>0)
		{
			DeviceOpenFlag[index]++;
		}
		index++;
	}
  /* USER CODE END vTimer01 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/** 
* @brief 記錄訊息並通過乙太網路模組發送至伺服器
*
* @param  message 訊息字串
*
* 這個函式會先創建一個JSON物件，並將傳入的訊息作為"LogMessage"鍵的值加入到物件中。
* 然後將JSON物件序列化為字串,通過乙太網路模組上傳至伺服器。
*
*/
void Log(char* message)
{
  cJSON * DataObject;
	char *data;
  DataObject=cJSON_CreateObject();   
	cJSON_AddItemToObject(DataObject, "LogMessage", cJSON_CreateString(message));	
	data = cJSON_Print(DataObject); ///將JSON物件序列化為字串
	UART_SEND6(data);
	cJSON_Delete(DataObject);
	free(data);
}

/** 
* @brief 獲取網路時間
*
* @param 無
*
* 這個函式是用來向伺服器請求獲取網路時間。它會創立一個 JSON 物件，
* 並將 "Operating" 鍵加入物件裡，其值為 "GetNetTime"。繼而將 JSON 物件
* 序列化為字串，透過乙太網路模組上傳至伺服器。
*
*/
void GetNetTime(void)
{
  cJSON * DataObject;
	char *data;
  DataObject=cJSON_CreateObject();   
	cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("GetNetTime"));	
	data = cJSON_Print(DataObject);
	UART_SEND6(data);
	cJSON_Delete(DataObject);
	free(data);
}

/**
* @brief 更新數據並透過乙太網路模組上傳至伺服器
*
* @param 無
*
* 這個函式用於將目前的數據值更新並透過乙太網路模組發送至伺服器。它會建立一個 JSON 物件，
* 並將以下資訊加入該物件:
*
* - "Operating": "UpdateData" (操作類型)
* - "Date": 目前的時間字串 (CurrentTime)
* - "CH1"~"CH8": 來自 CurrentValue 陣列的 8 個通道數據
*
* 在發送之前，它會將 CurrentValue 陣列中的 16 位元數據解譯為 8 個整數值，
* 並存放在 UpdataData 陣列中。然後將 JSON 物件序列化為字串，
* 並透過乙太網路模組上傳至伺服器。
*
*/
void UpdateDataDebug(void)
{
  cJSON * DataObject;
	char *data;
	uint8_t Index;
	int UpdataData[8]={0};
	
	//從CurrentValue 陣列中取得出8個通道的數據
	for(int i=0;i<8;i++)
	{	
		Index=i<<1;
		UpdataData[i]=((uint16_t)CurrentValue[Index]<<8)|CurrentValue[Index+1];
	}

  DataObject=cJSON_CreateObject();   
	cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));	
	cJSON_AddItemToObject(DataObject, "Date",cJSON_CreateString(CurrentTime));
	cJSON_AddItemToObject(DataObject, "CH1", cJSON_CreateNumber(UpdataData[0]));
	cJSON_AddItemToObject(DataObject, "CH2", cJSON_CreateNumber(UpdataData[1]));
	cJSON_AddItemToObject(DataObject, "CH3", cJSON_CreateNumber(UpdataData[2]));
	cJSON_AddItemToObject(DataObject, "CH4", cJSON_CreateNumber(UpdataData[3]));
	cJSON_AddItemToObject(DataObject, "CH5", cJSON_CreateNumber(UpdataData[4]));
	cJSON_AddItemToObject(DataObject, "CH6", cJSON_CreateNumber(UpdataData[5]));
	cJSON_AddItemToObject(DataObject, "CH7", cJSON_CreateNumber(UpdataData[6]));
	cJSON_AddItemToObject(DataObject, "CH8", cJSON_CreateNumber(UpdataData[7]));
	data = cJSON_Print(DataObject);
	UART_SEND6(data);
	cJSON_Delete(DataObject);
	free(data);
}

void UpdateData(void)
{
  cJSON * DataObject;
	char *data;
	uint8_t Index;
	int UpdataState[8]={0};
	
  for(int i=0;i<8;i++)
  {	
    UpdataState[i]=(uint16_t)DeviceStatus[i];
  }
  DataObject=cJSON_CreateObject();   
	cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));	
	cJSON_AddItemToObject(DataObject, "Date",cJSON_CreateString(CurrentTime));
	if(ModeTask==Remote)
	{ 
		cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Remote"));	
  }
	else
	{
		cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Manual"));
	}
	cJSON_AddItemToObject(DataObject, "CH1", cJSON_CreateNumber(UpdataState[0]));
	cJSON_AddItemToObject(DataObject, "CH2", cJSON_CreateNumber(UpdataState[1]));
	cJSON_AddItemToObject(DataObject, "CH3", cJSON_CreateNumber(UpdataState[2]));
	cJSON_AddItemToObject(DataObject, "CH4", cJSON_CreateNumber(UpdataState[3]));
	cJSON_AddItemToObject(DataObject, "CH5", cJSON_CreateNumber(UpdataState[4]));
	cJSON_AddItemToObject(DataObject, "CH6", cJSON_CreateNumber(UpdataState[5]));
	cJSON_AddItemToObject(DataObject, "CH7", cJSON_CreateNumber(UpdataState[6]));
	cJSON_AddItemToObject(DataObject, "CH8", cJSON_CreateNumber(UpdataState[7]));
	data = cJSON_Print(DataObject);
	UART_SEND6(data);
	cJSON_Delete(DataObject);
	free(data);
}

//錯誤偵測                                                           
void UpdateState()
{
  cJSON * DataObject;
  char *data;
  int UpdataState[8]={0};
	uint32_t now = HAL_GetTick();
	
  
  for(int i=0;i<8;i++)
  {	
    UpdataState[i]=(uint16_t)DeviceStatus[i];
  }
  
  for(int i=0;i<8;i++)
  {
		if(StateErrorFlag[i]) {
      UpdataState[i] = 2;
			StateErrorFlag[i]=0;
			timestamp[i]=0;
    }
    if(AppOnFlag[i]==1&&(uint16_t)DeviceStatus[i]==0 && (now - timestamp[i]) > 15000) {
      UpdataState[i] = 3;
			AppOnFlag[i]=0;
			timestamp[i]=0;
    }
    else if(AppOnFlag[i]==0 && (uint16_t)DeviceStatus[i]==1 && (now - timestamp[i]) > 15000) {
      UpdataState[i] = 4;
			timestamp[i]=0;
    }
  }
	
  for(int i=0;i<8;i++)//狀態異常自動關閉設備
  {
    if(UpdataState[i]==3) {
				Device_Close(i+1);
    }
  }	
	
  DataObject=cJSON_CreateObject();   
  cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));	
  cJSON_AddItemToObject(DataObject, "Date",cJSON_CreateString(CurrentTime));
	if(ModeTask==Remote)
	{ 
		cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Remote"));	
  }
	else
	{
		cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Manual"));
	}
	cJSON_AddItemToObject(DataObject, "CH1", cJSON_CreateNumber(UpdataState[0]));
  cJSON_AddItemToObject(DataObject, "CH2", cJSON_CreateNumber(UpdataState[1]));
  cJSON_AddItemToObject(DataObject, "CH3", cJSON_CreateNumber(UpdataState[2]));
  cJSON_AddItemToObject(DataObject, "CH4", cJSON_CreateNumber(UpdataState[3]));
  cJSON_AddItemToObject(DataObject, "CH5", cJSON_CreateNumber(UpdataState[4]));
  cJSON_AddItemToObject(DataObject, "CH6", cJSON_CreateNumber(UpdataState[5]));
  cJSON_AddItemToObject(DataObject, "CH7", cJSON_CreateNumber(UpdataState[6]));
  cJSON_AddItemToObject(DataObject, "CH8", cJSON_CreateNumber(UpdataState[7]));
  data = cJSON_Print(DataObject);
  UART_SEND6(data);
  cJSON_Delete(DataObject);
  free(data);
}

/**
* @brief 解析日期時間字串並填入TimerSwitch結構
*
* @param _str輸入的日期時間字串，格式為 "yyyy/mm/dd hh:mm:ss"
* @param ts指向要填入數據的 TimerSwitch 結構
* @param status狀態，用於設定isOn欄位，0 表示關閉(Off)，1 表示開啟(On)
*
* 這個函式會解析傳入的日期時間字串，並將解析出的年、月、日、時、分、秒數值填入
* 傳入的TimerSwitch結構體中。同時也會根據傳入的status參數，設定結構體的isOn
* 欄位為開啟或關閉。
*/
void ParseDateTimeString(const char *str, TimerSwitch *ts,uint8_t status)
{
    int year, month, day, hour, minute, second;
	
		//解析日期時間字串
    sscanf(str, "20%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	
		ts->confirm = 1;
    ts->year = (uint8_t)year;
    ts->month = (uint8_t)month;
    ts->date = (uint8_t)day;
    ts->hour = (uint8_t)hour;
    ts->minute = (uint8_t)minute;
    ts->second = (uint8_t)second;
		if(status==On) // 根據status設定isOn欄位
			ts->isOn=On;
		else
			ts->isOn=Off;
}

/**
* @brief 比較儲存在TimerSwitch結構的時間與目前時間
*
* @param  ID為TimerSwitch 結構在陣列中的索引值
* @return 如果 TimerSwitch 的時間小於或等於目前時間，回傳 1，否則回傳 0
*
* 這個函式會比較xTimeSwitch陣列中指定索引位置的TimerSwitch結構的時間與目前的時間。
* 如果TimerSwitch結構的時間小於或等於目前時間，表示已經到達或超過設定的時間點，
* 函式將回傳 1。否則回傳 0。
*/
uint8_t TimeCompare(uint8_t ID)
{
	return xTimeSwitch[ID].year<=Now_Date.Year&&
				 xTimeSwitch[ID].month<=Now_Date.Month&&
				 xTimeSwitch[ID].date<=Now_Date.Date&&
				 xTimeSwitch[ID].hour<=Now_Time.Hours&&
				 xTimeSwitch[ID].minute<=Now_Time.Minutes&&
				 xTimeSwitch[ID].second<=Now_Time.Seconds;
}
/* USER CODE END Application */
