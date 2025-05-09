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
//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永无BUG

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
#include "w25qxx.h"
#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAX_ALARMS          16       // 最大鬧鐘數量
#define ALARM_DATA_MARK 0xA5
#define ALARM_VERSION   0x01
// 定義時間資料格式的頭部標記
#define TIME_DATA_MARK 0x5A
#define TIME_VERSION   0x01
//01/07
// 新增常量定義
#define LIGHT_ON  1         // 開燈
#define LIGHT_OFF 0         // 關燈
//Flash 地址規劃（4KB對齊）
#define FLASH_TIME_ADDR      0x000000    // 時間資料（使用一個扇區）存儲當前時間的地址
#define FLASH_ALARM_ADDR     0x001000    // 鬧鐘資料（使用一個扇區）存儲鬧鐘配置的地址
#define FLASH_SECTOR_SIZE    4096    // 4KB 扇區大小
#define FLASH_EXPECTED_STATE_ADDR 0x002000 // 期望狀態資料位址
#define EXPECTED_STATE_DATA_MARK 0xB6      // 期望狀態標記
#define EXPECTED_STATE_VERSION   0x01      // 期望狀態版本
typedef enum {false = 0,true = 1} bool;

/**
 * @struct AlarmData
 * @brief 輕量級鬧鐘資料結構
 */
typedef struct {
    uint8_t enabled;       ///< 啟用狀態 (0=停用, 1=啟用)
    uint8_t year;          ///< 年份 (0-99)
    uint8_t month;         ///< 月份 (1-12)
    uint8_t date;          ///< 日期 (1-31)
    uint8_t hour;          ///< 小時 (0-23)
    uint8_t minute;        ///< 分鐘 (0-59)
    uint8_t second;        ///< 秒 (0-59)
    uint8_t light_action;  ///< 燈光動作 (0=關閉, 1=開啟)
    uint8_t repeat_days[8];///< 每週重複設定 (索引0-6分別對應週日至週六, 值為0表示不重複, 1表示重複)
    uint8_t reserved;      ///< 保留位元 (用於記憶體對齊)
} AlarmData;

/**
 * @struct TimeData
 * @brief 基本時間資料結構
 */
typedef struct {
    uint8_t mark;      ///< 資料標記 (0x5A)
    uint8_t version;   ///< 資料版本號
    uint8_t year;      ///< 年份 (0-99)
    uint8_t month;     ///< 月份 (1-12)
    uint8_t date;      ///< 日期 (1-31)
    uint8_t hour;      ///< 小時 (0-23)
    uint8_t minute;    ///< 分鐘 (0-59)
    uint8_t second;    ///< 秒 (0-59)
    uint8_t weekday;   ///< 星期 (1-7, 1=星期一)
} TimeData;

/**
 * @struct Alarm_TypeDef
 * @brief 完整鬧鐘資料結構
 */
typedef struct {
    uint16_t year;         ///< 年份 (2000-2099)
    uint8_t month;         ///< 月份 (1-12)
    uint8_t date;          ///< 日期 (1-31)
    uint8_t hour;          ///< 小時 (0-23)
    uint8_t minute;        ///< 分鐘 (0-59)
    uint8_t second;        ///< 秒 (0-59)
    uint8_t enabled;       ///< 啟用狀態 (0=停用, 1=啟用)
    uint8_t weekday;       ///< 星期 (1-7, 1=星期一)
    uint8_t device_id;     ///< 設備ID (1-8)
    uint8_t repeat_days[7];///< 每週重複設定 (索引0-6分別對應週一至週日, 值為0表示不重複, 1表示重複)
    uint8_t light_action;  ///< 燈光動作 (0=關閉, 1=開啟)
} Alarm_TypeDef;

/**
 * @struct TimeData_t
 * @brief 含校驗碼的時間資料結構 (記憶體對齊優化)
 */
typedef struct __attribute__((packed)) {
    uint8_t mark;      ///< 資料標記 (固定為0x5A)
    uint8_t version;   ///< 資料版本號 (固定為0x01)
    uint8_t year;      ///< 年份 (0-99)
    uint8_t month;     ///< 月份 (1-12)
    uint8_t date;      ///< 日期 (1-31)
    uint8_t hour;      ///< 小時 (0-23)
    uint8_t minute;    ///< 分鐘 (0-59)
    uint8_t second;    ///< 秒 (0-59)
    uint8_t weekday;   ///< 星期 (1-7, 1=星期一)
    uint8_t checksum;  ///< 校驗碼 (用於驗證資料完整性)
} TimeData_t;

/**
 * @struct AlarmData_t
 * @brief 含校驗碼的鬧鐘資料結構 (16字節, 記憶體對齊優化)
 */
typedef struct __attribute__((packed)) {
    uint8_t enabled;       ///< 啟用狀態 (0=停用, 1=啟用)
    uint8_t year;          ///< 年份 (0-99)
    uint8_t month;         ///< 月份 (1-12)
    uint8_t date;          ///< 日期 (1-31)
    uint8_t hour;          ///< 小時 (0-23)
    uint8_t minute;        ///< 分鐘 (0-59)
    uint8_t second;        ///< 秒 (0-59)
    uint8_t light_action;  ///< 燈光動作 (0=關閉, 1=開啟)
    uint8_t device_id;     ///< 設備ID (1-8)
    uint8_t repeat_days[7];///< 每週重複設定 (索引0-6對應週一至週日)
    uint8_t checksum;      ///< 校驗碼 (用於驗證資料完整性)
} AlarmData_t;

/**
 * @struct AlarmHeader_t
 * @brief 鬧鐘資料頭部結構 (4字節, 記憶體對齊優化)
 */
typedef struct __attribute__((packed)) {
    uint8_t mark;      ///< 資料標記 (固定為0xA5)
    uint8_t version;   ///< 資料版本號 (固定為0x01)
    uint8_t count;     ///< 鬧鐘數量
    uint8_t checksum;  ///< 校驗碼 (用於驗證資料完整性)
} AlarmHeader_t;

/**
 * @enum DeviceSwitch
 * @brief 設備開關狀態枚舉
 */
typedef enum {
    DEVICE_OFF = 0,    ///< 設備關閉
    DEVICE_ON = 1      ///< 設備開啟
} DeviceSwitch;

/**
 * @struct DeviceState
 * @brief 設備狀態結構
 */
typedef struct {
    DeviceSwitch expectedState;    ///< APP命令期望的狀態
    DeviceSwitch currentState;     ///< 電流感測器檢測到的實際狀態
    DeviceSwitch previousState;    ///< 前一個狀態 (用於檢測變化)
    uint8_t persistentErrorState;  ///< 錯誤狀態 (0=關閉, 1=開啟, 2=命令不符, 3=意外關閉, 4=意外開啟)
    bool appCommandReceived;       ///< 是否收到APP命令的標記
    uint32_t lastCommandTime;      ///< 最後命令接收時間 (系統毫秒計數)
} DeviceState;

/**
 * @struct ExpectedStateHeader_t
 * @brief 期望狀態資料頭部結構 (4字節, 記憶體對齊優化)
 */
typedef struct __attribute__((packed)) {
    uint8_t mark;      ///< 資料標記 (固定為0xB6)
    uint8_t version;   ///< 資料版本號 (固定為0x01)
    uint8_t count;     ///< 設備數量 (固定為8)
    uint8_t checksum;  ///< 校驗碼 (用於驗證資料完整性)
} ExpectedStateHeader_t;

/**
 * @struct ExpectedStateData_t
 * @brief 期望狀態資料結構 (9字節, 記憶體對齊優化)
 */
typedef struct __attribute__((packed)) {
    DeviceSwitch states[8]; ///< 8個設備的期望狀態陣列
    uint8_t checksum;       ///< 校驗碼 (用於驗證資料完整性)
} ExpectedStateData_t;



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern   uint8_t RX1Buffer[55];         ///< UART1接收緩衝區
extern   uint8_t RX1Data;               ///< UART1接收單字節
extern 	 uint8_t RX6Data;               ///< UART6接收單字節
extern   char    CopyRX6Buffer[512];    ///< UART6接收緩衝區
volatile uint8_t CurrentValue[16];      ///< 各通道電流值存儲數組，每個通道佔2字節
volatile uint8_t DeviceOpenFlag[8]={0}; ///< 設備開啟標誌數組
volatile uint8_t Restart[8]={0};        ///< 設備重啟標誌數組
volatile uint8_t AppOnFlag[9]={0};      ///< APP控制信號接收標誌
volatile uint8_t state_clear=0;         ///< APP清除顯示狀態標誌
volatile uint8_t Task1Flag=Waiting;     ///< 任務1狀態標誌
volatile uint8_t SetTime=0;             ///< 時間設置狀態標誌
volatile uint8_t ModeTask=Remote;       ///< Manual手動 、 Remote遠端
uint8_t volt[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x18, 0x45, 0xC0};///< Modbus讀取命令，用於讀取ADC通道數據
int TimeCount=0;                        ///< 計時器計數
TimerSwitch  xTimeSwitch[8];            ///< 8個定時開關設置
Alarm_TypeDef alarms[MAX_ALARMS];       ///< 鬧鐘配置數組，最大數量由MAX_ALARMS定義
extern RTC_TimeTypeDef Now_Time;        ///< RTC當前時間
extern RTC_DateTypeDef Now_Date;        ///< RTC當前日期
DeviceState deviceArray[8];             ///< 8個開關的狀態陣列
const uint32_t COMMAND_TIMEOUT = 10000;  ///< 命令超時時間(毫秒)，超過此時間未收到新命令則重置
extern osMutexId uart1Mutex;            ///< UART1互斥鎖
uint32_t timestamp[8] = {0};            ///< 時間戳記錄數組
extern volatile uint8_t controllerCommunicationInProgress;///< 控制器通訊進行中標誌
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
void Log(char* message);                        ///< 記錄日誌信息
void GetNetTime(void);                          ///< 向服務器請求網絡時間
void UpdateData(void);                          ///< 更新並發送當前設備數據(手動模式)
void UpdateState(void);                         ///< 更新並發送當前設備狀態(遠端模式，帶錯誤檢測)
void UpdateDataDebug(void);                     ///< 更新並發送原始電流值數據(Debug用)
void Flash_SaveCurrentTime(void);               ///< 保存當前系統時間到Flash
void Process_JSON_Command(const char* jsonStr); ///< 處理JSON格式控制命令
void Check_Alarms(void);                        ///< 檢查並執行到期的鬧鐘動作
bool isRepeatAlarm(Alarm_TypeDef alarm);        ///< 檢查鬧鐘是否為重複性鬧鐘
void parseAlarmDateTime(const char* dateTimeStr, Alarm_TypeDef* alarm);///< 解析日期時間字符串到鬧鐘結構體
void DeleteAlarm(int index);                    ///< 刪除指定(0 ~ MAX_ALARMS-1)的鬧鐘
void GetAlarmList();                            ///< 獲取並發送所有鬧鐘列表
bool Flash_LoadCurrentTime(void);               ///< 從Flash加載系統時間
bool Flash_SaveAlarms(void);                    ///< 保存鬧鐘設置到Flash
bool Flash_LoadAlarms(void);                    ///< 從Flash加載鬧鐘設置
void SendAlarmCount(int totalEnabled);          ///< 發送鬧鐘總數信息
void initDeviceStatus(void);                    ///< 初始化設備狀態
bool Flash_SaveExpectedStates(void);            ///< 保存設備期望狀態到Flash
bool Flash_LoadExpectedStates(void);            ///< 從Flash加載設備期望狀態
uint8_t Calculate_Checksum(const void* data, size_t len);        ///< 計算數據校驗和
void GetWeekdayString(uint8_t weekday, char *weekStr);           ///< 獲取星期幾的字符串表示
bool Should_Alarm_Trigger(const Alarm_TypeDef *alarm);           ///< 檢查鬧鐘是否應該觸發
void Execute_Alarm_Action(const Alarm_TypeDef *alarm, int index);///< 執行鬧鐘觸發動作
void SendSingleAlarm(int index, const Alarm_TypeDef *alarm);     ///< 發送單個鬧鐘信息


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
	* @author Lingaga
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
	uint8_t Index=0;
	
	HAL_UART_Receive_IT(&huart1, &RX1Data, 1);
	HAL_UART_Receive_IT(&huart6, &RX6Data, 1);	
	// 初始化Flash
	W25QXX_Init();
	Flash_LoadCurrentTime();//讀取關機時間
	osDelay(100);
    Flash_LoadAlarms();//讀取定時參數
	initDeviceStatus();//初始設備狀態
  Flash_LoadExpectedStates();
	// 從 EEPROM 讀取設備狀態

  while(Index < 8)
  {
      if (deviceArray[Index].expectedState == DEVICE_ON)
      {
          Device_Open(Index + 1); // 開啟設備
          deviceArray[Index].expectedState = DEVICE_ON;
          deviceArray[Index].appCommandReceived = true;
          deviceArray[Index].lastCommandTime = HAL_GetTick();
          Restart[Index]=1;
      }
      // 如果 expectedState 是 OFF，則不需要動作，因為 initDeviceStatus 預設是 OFF
      Index++;
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
* @author Lingaga
* 這個任務有以下主要功能:
*
* 1. 獲取網路時間
* 2. 發送資料
* 3. 處理日誌佇列
*/
/* USER CODE END Header_StartTask01 */
void StartTask01(void const * argument)
{
  /* USER CODE BEGIN StartTask01 */
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
				osDelay(3000);
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
    osDelay(100);
  }
  /* USER CODE END StartTask01 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief 資料處理和設備狀態更新任務
* @author Lingaga
* @param argument 未使用的參數
*
* 這個任務的主要功能是處理感測器資料並更新相應設備的狀態。它會執行以下操作:
*
* 1. 每兩秒會通過序列埠發送讀取ADC 8通道的要求。
* 2. 遍歷CurrentValue陣列中的 8 個通道資料:
*    - 如果通道資料超過閾值且對應設備當前未開啟,則開啟該設備並更新DeviceStatus陣列和EEPROM中的狀態。
*    - 如果通道資料未超過閾值且對應設備當前已開啟,則關閉該設備並更新DeviceStatus陣列和EEPROM中的狀態。
*    - 如果設備狀態發生變化且當前任務處於等待狀態,則設置標誌以發送資料。
*
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
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
				//if((CurrentValue[temp]==0x03 && CurrentValue[temp+1]>0xE8) || CurrentValue[temp]>0x03)//Current > 1A
				if((CurrentValue[temp]==0x00 && CurrentValue[temp+1]>0x64) || CurrentValue[temp]>0x00)//Current>=0.1A
				{
					if(!DeviceStatus[Index])
					{
						LED_ON(Index+1);
						DeviceStatus[Index]=1;
                        deviceArray[Index].currentState = DEVICE_ON;
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
                        deviceArray[Index].currentState = DEVICE_OFF ;
						if(Task1Flag==Waiting)
							Task1Flag=SendDataTask;
					}
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
* @author Lingaga
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
* @author Lingaga
* @retval None
* 這個任務主要功能是解析從序列埠接收到的JSON格式資料,並根據操作類型執行相應的操作。
* 操作包括獲取網路時間、發送資料、開啟/關閉設備以及定時設備開啟/關閉時間等。
*
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
	uint8_t DeviceID=0;
  /* Infinite loop */
  for(;;)
  {
		if(strlen(CopyRX6Buffer) > 0)
		{
			Process_JSON_Command(CopyRX6Buffer);
			memset(CopyRX6Buffer, 0, sizeof(CopyRX6Buffer));
		}
	    osDelay(100);	
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief 定時器開關任務
* @author Lingaga
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
  /* Infinite loop */
    for(;;)
    {
		GET_Time();
		if(Time_Offset())
			Task1Flag=GetNetTimeTask;
		Check_Alarms(); 
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
  /* USER CODE END vTimer01 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/** 
* @brief 獲取網路時間
* @author Lingaga
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
* @brief 更新資料並透過乙太網路模組上傳至伺服器
* @author Lingaga
* @param 無
*
* 這個函式用於將目前的資料值更新並透過乙太網路模組發送至伺服器。它會建立一個 JSON 物件，
* 並將以下資訊加入該物件:
*
* - "Operating": "UpdateData" (操作類型)
* - "Date": 目前的時間字串 (CurrentTime)
* - "CH1"~"CH8": 來自 CurrentValue 陣列的 8 個通道資料
*
* 在發送之前，它會將 CurrentValue 陣列中的 16 位元資料解譯為 8 個整數值，
* 並存放在 UpdataData 陣列中。然後將 JSON 物件序列化為字串，
* 並透過乙太網路模組上傳至伺服器。
*
*/
//手動模式看電流值
void UpdateDataDebug(void)
{
  cJSON * DataObject;
	char *data;
	uint8_t Index;
	int UpdataData[8]={0};
	
	//從CurrentValue 陣列中取得出8個通道的資料
	for(int i=0;i<8;i++)
	{	
		Index=i<<1;
		UpdataData[i]=((uint16_t)CurrentValue[Index]<<8)|CurrentValue[Index+1];
	}

  DataObject=cJSON_CreateObject();   
	cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));	
	cJSON_AddItemToObject(DataObject, "Date",cJSON_CreateString(CurrentTime));
	cJSON_AddNumberToObject(DataObject, "Weekday", Now_Date.WeekDay);
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

//手動模式
void UpdateData(void)
{
    cJSON *DataObject, *CHArray;
    char *data;
    uint8_t Index;
    int DeviceStateValue[8] = {0};          // 發送給伺服器的設備狀態值 (0, 1, 2, 3, 4)
    int CurrentValues[8] = {0};             // 電流值
     uint32_t currentTime = HAL_GetTick();
    // 創建主要的JSON物件
    DataObject = cJSON_CreateObject();

    // 添加Operating欄位
    cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));
    
    // 直接添加日期時間字串，格式為："YYYY/MM/DD HH:MM:SS"
    cJSON_AddStringToObject(DataObject, "dateTimeString", CurrentTime);
    
    // 添加weekday
    cJSON_AddNumberToObject(DataObject, "weekday", Now_Date.WeekDay);
    
    // 添加Mode欄位
    if(ModeTask == Remote)
    { 
        cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Remote"));    
    }
    else
    {
        cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Manual"));
    }
    
    // 獲取每個通道的狀態和電流值，並執行錯誤檢測
    for(int i = 0; i < 8; i++)
    {    
        Index = i << 1;
        CurrentValues[i] = ((uint16_t)CurrentValue[Index] << 8) | CurrentValue[Index + 1];
           
        // 更新實際狀態 (假設 Task02 已更新 deviceArray[i].currentState)
        
        // 檢查命令是否超時
        if(deviceArray[i].appCommandReceived && (currentTime - deviceArray[i].lastCommandTime > COMMAND_TIMEOUT) && Restart[i] != 1) 
        {
            deviceArray[i].appCommandReceived = false;
        }
            
        deviceArray[i].persistentErrorState = deviceArray[i].currentState; // 重置為當前狀態 (0 或 1)
				DeviceStateValue[i] = deviceArray[i].persistentErrorState; // 要發送的狀態碼
        // 為每個通道創建包含狀態和電流值的陣列
        CHArray = cJSON_CreateArray();
        cJSON_AddItemToArray(CHArray, cJSON_CreateNumber(DeviceStateValue[i]));
        cJSON_AddItemToArray(CHArray, cJSON_CreateNumber(CurrentValues[i]));
        
        // 添加到主物件中
        char channelName[5];
        sprintf(channelName, "CH%d", i+1);
        cJSON_AddItemToObject(DataObject, channelName, CHArray);
        Restart[i]=0;
        // 更新 previousState 以供下次比較
        deviceArray[i].previousState = deviceArray[i].currentState;
    }
    
    // 轉換為字串並發送
    data = cJSON_Print(DataObject);
    UART_SEND6(data);
    
    // 清理記憶體
    cJSON_Delete(DataObject);
    free(data);
}

//遠端模式->錯誤偵測                                                             
void UpdateState(void)
{
    cJSON *DataObject, *CHArray;
    char *data;
    uint8_t Index;
    int DeviceStateValue[8] = {0};    // 發送給伺服器的設備狀態值 (0, 1, 2, 3, 4)
    int CurrentValues[8] = {0};  // 電流值
     uint32_t currentTime = HAL_GetTick();
    // 創建主要的JSON物件
    DataObject = cJSON_CreateObject();
    
    // 添加Operating欄位
    cJSON_AddItemToObject(DataObject, "Operating", cJSON_CreateString("UpdateData"));
    
    // 直接添加日期時間字串，格式為："YYYY/MM/DD HH:MM:SS"
    cJSON_AddStringToObject(DataObject, "dateTimeString", CurrentTime);
    
    // 添加weekday
    cJSON_AddNumberToObject(DataObject, "weekday", Now_Date.WeekDay);
    
    // 添加Mode欄位
    if(ModeTask == Remote)
    { 
        cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Remote"));    
    }
    else
    {
        cJSON_AddItemToObject(DataObject, "Mode", cJSON_CreateString("Manual"));
    }
    
    // 獲取每個通道的狀態和電流值，並執行錯誤檢測
    for(int i = 0; i < 8; i++)
    {    
        Index = i << 1;
        CurrentValues[i] = ((uint16_t)CurrentValue[Index] << 8) | CurrentValue[Index + 1];
           
        // 更新實際狀態 (假設 Task02 已更新 deviceArray[i].currentState)

        // 檢查命令是否超時
        if(deviceArray[i].appCommandReceived && (currentTime - deviceArray[i].lastCommandTime > COMMAND_TIMEOUT) && Restart[i] != 1) 
        {
          deviceArray[i].appCommandReceived = false;
        }
            
        // 根據設備狀態判斷來設置DeviceStateValue
        if(deviceArray[i].appCommandReceived) {
            // 收到 APP 命令，重置持續錯誤狀態，並檢查錯誤類型 2
            if((deviceArray[i].expectedState != deviceArray[i].currentState) && (currentTime - deviceArray[i].lastCommandTime >3000)) 
            {
                deviceArray[i].persistentErrorState = 2; // 錯誤類型1: 命令與狀態不符
                Device_Close(i+1);
            } 
            else 
            {
                deviceArray[i].persistentErrorState = deviceArray[i].currentState; // 重置為當前狀態 (0 或 1)
            }
        } 
        else 
        {
            // 未收到 APP 命令或命令已超時
            if (deviceArray[i].persistentErrorState <= 1) 
            { // 僅在沒有持續錯誤時檢查新的瞬時錯誤 (類型 3, 4)
                if(deviceArray[i].previousState == DEVICE_ON &&  deviceArray[i].currentState == DEVICE_OFF) 
                {
                    deviceArray[i].persistentErrorState = 3; // 錯誤類型2: 意外關閉
                }
                else if(deviceArray[i].previousState == DEVICE_OFF && deviceArray[i].currentState == DEVICE_ON) 
                {
                    deviceArray[i].persistentErrorState = 4; // 錯誤類型3: 意外開啟
                }
                else 
                {
                     // 如果狀態未變，且沒有持續錯誤，則保持當前狀態
                     deviceArray[i].persistentErrorState = deviceArray[i].currentState;
                 }
            }
            // 如果 persistentErrorState 已經是 2, 3 或 4，則保持該錯誤狀態，直到收到新命令
        }
        DeviceStateValue[i] = deviceArray[i].persistentErrorState; // 要發送的狀態碼
        // 為每個通道創建包含狀態和電流值的陣列
        CHArray = cJSON_CreateArray();
        cJSON_AddItemToArray(CHArray, cJSON_CreateNumber(DeviceStateValue[i]));
        cJSON_AddItemToArray(CHArray, cJSON_CreateNumber(CurrentValues[i]));
        
        // 添加到主物件中
        char channelName[5];
        sprintf(channelName, "CH%d", i+1);
        cJSON_AddItemToObject(DataObject, channelName, CHArray);
        Restart[i]=0;
        // 更新 previousState 以供下次比較
        deviceArray[i].previousState = deviceArray[i].currentState;
    }
    
    // 轉換為字串並發送
    data = cJSON_Print(DataObject);
    UART_SEND6(data);
    // 清理記憶體
    cJSON_Delete(DataObject);
    free(data);
}



//

/**
* @brief 將當前系統時間保存到外部Flash記憶體中
* @author Lingaga
* @param None
* @return None
*
* 這個函式負責將系統當前的時間日期資訊保存到W25QXX Flash記憶體中。
* 儲存的資訊包含：
* - 特殊標記（用於驗證資料有效性）
* - 版本資訊
* - 年、月、日
* - 時、分、秒
* - 星期
* 同時會計算並儲存校驗和（checksum）以確保資料完整性。
*
* 函式執行流程：
* 1. 建立TimeData_t結構體實例並使用當前系統時間初始化
*    - 設置特殊標記TIME_DATA_MARK用於識別有效資料
*    - 設置版本號TIME_VERSION
*    - 從Now_Date結構體獲取並設置年、月、日、星期資訊
*    - 從Now_Time結構體獲取並設置時、分、秒資訊
*
* 2. 計算資料校驗和
*    - 使用Calculate_Checksum函式計算整個結構體的校驗和
*    - 將計算得到的校驗和儲存在結構體中
*    - 校驗和用於後續讀取時驗證資料完整性
*
* 3. 將資料寫入Flash記憶體
*    - 首先擦除目標扇區（FLASH_TIME_ADDR位址所在的扇區）
*    - 將完整的時間資料結構寫入指定的Flash位址
*    - 使用W25QXX_Write函式執行實際的寫入操作
*
* 注意事項：
* - Flash寫入前必須先擦除，否則可能導致資料寫入失敗
* - FLASH_TIME_ADDR必須按照FLASH_SECTOR_SIZE對齊
* - 確保TimeData_t結構體大小不超過一個Flash扇區的大小
* - 寫入操作可能需要一定時間，調用時需注意系統時序
*/
void Flash_SaveCurrentTime(void)
{
    TimeData_t timeData = {
        .mark = TIME_DATA_MARK,
        .version = TIME_VERSION,
        .year = Now_Date.Year,
        .month = Now_Date.Month,
        .date = Now_Date.Date,
        .hour = Now_Time.Hours,
        .minute = Now_Time.Minutes,
        .second = Now_Time.Seconds,
        .weekday = Now_Date.WeekDay
    };
    
    timeData.checksum = Calculate_Checksum(&timeData, sizeof(TimeData_t));
    
    // 擦除扇區
    W25QXX_Erase_Sector(FLASH_TIME_ADDR / FLASH_SECTOR_SIZE);
    // 寫入資料
    W25QXX_Write((uint8_t*)&timeData, FLASH_TIME_ADDR, sizeof(TimeData_t));
}
/**
* @brief 從Flash記憶體讀取時間資料並設置到RTC
* @author Lingaga
* @param None
* @return bool - 讀取並設置時間是否成功
*                true: 成功讀取並設置時間
*                false: 資料無效或驗證失敗
*
* 這個函式負責從W25QXX Flash記憶體中讀取之前保存的時間資料，
* 驗證其有效性後設置到STM32的RTC中。
*
* 注意事項：
* - 讀取的資料必須通過三重驗證（標記、版本、校驗和）才會被使用
* - RTC時間格式使用二進制格式（RTC_FORMAT_BIN）
* - 如果驗證失敗，函式會立即返回false，不會修改RTC的設置
* - 時間設置使用HAL庫函式，確保了與硬體的正確交互
*/

bool Flash_LoadCurrentTime(void)
{
    TimeData_t timeData;
    
    // 讀取資料
    W25QXX_Read((uint8_t*)&timeData, FLASH_TIME_ADDR, sizeof(TimeData_t));
    
    // 驗證資料
    uint8_t checksum = Calculate_Checksum(&timeData, sizeof(TimeData_t));
    if(timeData.mark == TIME_DATA_MARK && timeData.version == TIME_VERSION && timeData.checksum == checksum)
    {
        RTC_TimeTypeDef sTime = {0};
        RTC_DateTypeDef sDate = {0};
        
        sTime.Hours = timeData.hour;
        sTime.Minutes = timeData.minute;
        sTime.Seconds = timeData.second;
        
        sDate.Year = timeData.year;
        sDate.Month = timeData.month;
        sDate.Date = timeData.date;
        sDate.WeekDay = timeData.weekday;
        
        HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        
        return true;
    }
    return false;
}
/**
* @brief 將鬧鐘設定資料保存到外部Flash記憶體
* @author Lingaga
* @param None
* @return bool - 保存操作是否成功
*                true: 成功保存所有鬧鐘資料
*
* 這個函式負責將系統中的鬧鐘設定資料（最多MAX_ALARMS個）保存到W25QXX Flash記憶體中。
* 儲存的資料包含兩個主要部分：
* 1. 頭部資料（AlarmHeader_t）
* 2. 鬧鐘資料陣列（AlarmData_t × MAX_ALARMS）
*
* 注意事項：
* - 函式中多次呼叫HAL_IWDG_Refresh以防止看門狗重置
* - Flash寫入前會先擦除整個扇區
* - 資料寫入採用連續位址方式，使用offset計算每個鬧鐘資料的儲存位置
* - FLASH_ALARM_ADDR必須按照FLASH_SECTOR_SIZE對齊
* - 確保整體資料大小（頭部+所有鬧鐘資料）不超過一個Flash扇區
*/

bool Flash_SaveAlarms(void)
{
    AlarmHeader_t header;
    header.mark = ALARM_DATA_MARK;
    header.version = ALARM_VERSION;
    header.count = MAX_ALARMS;
    
    // 計算檢查碼
    header.checksum = Calculate_Checksum(&header, sizeof(AlarmHeader_t));
    
    // 寫入頭部資料
    W25QXX_Write((uint8_t*)&header, FLASH_ALARM_ADDR, sizeof(AlarmHeader_t));
    HAL_IWDG_Refresh(&hiwdg);
    
    uint32_t offset = sizeof(AlarmHeader_t);
    
    // 寫入每個鬧鐘資料
    for(int i = 0; i < MAX_ALARMS; i++)
    {
        AlarmData_t alarmData = {
            .enabled = alarms[i].enabled,
            .year = alarms[i].year,
            .month = alarms[i].month,
            .date = alarms[i].date,
            .hour = alarms[i].hour,
            .minute = alarms[i].minute,
            .second = alarms[i].second,
            .light_action = alarms[i].light_action,
			.device_id = alarms[i].device_id  
        };
        
        // 複製重複日期資料
        memcpy(alarmData.repeat_days, alarms[i].repeat_days, sizeof(alarmData.repeat_days));
        
        // 計算檢查碼
        alarmData.checksum = Calculate_Checksum(&alarmData, sizeof(AlarmData_t));
        
        // 寫入資料
        W25QXX_Write((uint8_t*)&alarmData,FLASH_ALARM_ADDR + offset, sizeof(AlarmData_t));
        offset += sizeof(AlarmData_t);
		HAL_IWDG_Refresh(&hiwdg);
    }
    
    HAL_IWDG_Refresh(&hiwdg);
    return true;
}

/**
* @brief 從Flash記憶體讀取鬧鐘設定資料，如果沒有有效資料則初始化預設值
* @author Lingaga
* @param None
* @return bool - 讀取操作的結果
*                true: 成功讀取資料或成功初始化預設資料
*                false: 操作失敗
*
* 這個函數負責從W25QXX Flash記憶體中讀取之前保存的鬧鐘設定資料，
* 並將有效的資料載入到系統的全域陣列alarms中。
* 若Flash中沒有有效的鬧鐘資料，則進行初始化並保存預設的鬧鐘配置。
*
* 預設鬧鐘配置:
* - 共16個鬧鐘，分配給8個設備(每個設備有開/關兩個動作)
* - 所有鬧鐘預設設為2099/12/31 00:00:00，不啟用，無重複
* - 偶數索引(0,2,4...)為關閉動作(Switch=0)
* - 奇數索引(1,3,5...)為開啟動作(Switch=1)
*
* 錯誤處理：
* - 頭部資料無效時，初始化預設值
* - 單個鬧鐘資料無效時，只清空對應的陣列元素
*/

bool Flash_LoadAlarms(void)
{
    uint32_t startTick = HAL_GetTick();
    AlarmHeader_t header;
    bool needInitialization = false;
    
    // 讀取頭部資料
    W25QXX_Read((uint8_t*)&header, FLASH_ALARM_ADDR, sizeof(AlarmHeader_t));
    HAL_IWDG_Refresh(&hiwdg);
    
    // 驗證頭部資料
    uint8_t headerChecksum = Calculate_Checksum(&header, sizeof(AlarmHeader_t));
    if(header.mark != ALARM_DATA_MARK || header.version != ALARM_VERSION || header.checksum != headerChecksum || header.count == 0)
    {
        // 資料無效或為空，需要初始化
        needInitialization = true;
    }
    else
    {
        // 讀取鬧鐘資料
        uint32_t offset = sizeof(AlarmHeader_t);
        for(int i = 0; i < header.count && i < MAX_ALARMS; i++)
        {
            if(HAL_GetTick() - startTick >= 100)
            {
                HAL_IWDG_Refresh(&hiwdg);
                startTick = HAL_GetTick();
            }
            
            AlarmData_t alarmData;
            W25QXX_Read((uint8_t*)&alarmData, FLASH_ALARM_ADDR + offset, sizeof(AlarmData_t));
            // 驗證資料
            uint8_t checksum = Calculate_Checksum(&alarmData, sizeof(AlarmData_t));
            if(checksum == alarmData.checksum)
            {
                // 轉換為運行時格式
                alarms[i].enabled = alarmData.enabled;
                alarms[i].year = alarmData.year;
                alarms[i].month = alarmData.month;
                alarms[i].date = alarmData.date;
                alarms[i].hour = alarmData.hour;
                alarms[i].minute = alarmData.minute;
                alarms[i].second = alarmData.second;
                alarms[i].light_action = alarmData.light_action;
                alarms[i].device_id = alarmData.device_id;  // 設備ID
                memcpy(alarms[i].repeat_days, alarmData.repeat_days, sizeof(alarms[i].repeat_days));
            }
            else
            {
                // 資料無效，清空該條目
                memset(&alarms[i], 0, sizeof(Alarm_TypeDef));
                needInitialization = true; // 如果有任何一條鬧鐘資料無效，也需要初始化
            }
            
            offset += sizeof(AlarmData_t);
        }
    }
    
    // 如果需要初始化，則設定預設鬧鐘配置
    if(needInitialization)
    {
        // 清空所有鬧鐘配置
        memset(alarms, 0, sizeof(alarms));
        
        // 設定16個預設鬧鐘
        for(int i = 0; i < MAX_ALARMS; i++)
        {
            alarms[i].year = 2099;            // 年份：2099
            alarms[i].month = 12;             // 月份：12月
            alarms[i].date = 31;              // 日期：31日
            alarms[i].hour = 0;               // 小時：0
            alarms[i].minute = 0;             // 分鐘：0
            alarms[i].second = 0;             // 秒：0
            alarms[i].enabled = 0;            // 啟用狀態：停用
            alarms[i].weekday = 0;            // 星期幾：不設定
            alarms[i].device_id = (i / 2) + 1;// 設備ID：每兩個鬧鐘對應一個設備
            alarms[i].light_action = i % 2;   // 燈光動作：偶數索引=關閉，奇數索引=開啟
            
            // 清空重複日期
            memset(alarms[i].repeat_days, 0, sizeof(alarms[i].repeat_days));
        }
        
        // 將初始化後的鬧鐘配置保存到Flash
        if(!Flash_SaveAlarms())
        {
            // 保存失敗
            return false;
        }
    }
    HAL_IWDG_Refresh(&hiwdg);
    return true;
}

/**
* @brief 定期檢查並觸發到時的鬧鐘
* @author Lingaga
* @param None
* @return None
*
* 這個函數負責定期檢查所有已啟用的鬧鐘，並在鬧鐘時間到達時執行相應的動作。
* 函數設計為每秒執行一次檢查，使用靜態變數記錄上次檢查時間以控制檢查頻率。
*
* 執行流程：
* 1. 時間控制
* 2. 時間更新
* 3. 鬧鐘檢查
* 4. 鬧鐘觸發處理
* 5. 一次性鬧鐘處理

* 注意事項：
* - 函數應在主迴圈中定期調用
* - 使用RTC_FORMAT_BIN確保時間格式一致性
* - Flash寫入操作可能會耗時，需要注意系統響應時間
* - 鬧鐘索引號i用於在Execute_Alarm_Action中識別具體的鬧鐘

* 相關函數：
* - Should_Alarm_Trigger: 檢查鬧鐘是否應該觸發
* - Execute_Alarm_Action: 執行鬧鐘對應的動作
* - isRepeatAlarm: 判斷是否為重複性鬧鐘
* - Flash_SaveAlarms: 保存鬧鐘設定到Flash
*/

void Check_Alarms(void)
{
    static uint32_t lastCheckTime = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // 每秒檢查一次
    if(currentTime - lastCheckTime < 1000)
    {
        return;
    }
    lastCheckTime = currentTime;
    
    // 更新當前時間
    HAL_RTC_GetTime(&hrtc, &Now_Time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &Now_Date, RTC_FORMAT_BIN);
    
    for(int i = 0; i < MAX_ALARMS; i++)
    {
        if(!alarms[i].enabled) continue;
        
        if(Should_Alarm_Trigger(&alarms[i]))
        {
            // 執行鬧鐘動作
            Execute_Alarm_Action(&alarms[i], i);
            
            // 如果是一次性鬧鐘，禁用它
            if(!isRepeatAlarm(alarms[i]))
            {
                alarms[i].enabled = 0;
                Flash_SaveAlarms();
            }
        }
    }
}
/**
* @brief 處理JSON格式的命令字串
* @author Lingaga
* @param jsonStr 輸入的JSON格式命令字串
* @return None
*
* 這個函式負責解析並執行從串口接收到的JSON格式命令。
* 支援多種操作類型，包括時間設定、鬧鐘管理、裝置控制和模式切換等。
*
* 支援的命令類型：
* 1. 時間更新（ReturnTime）
*    - 設定系統當前時間和日期
*    - 參數：CurrentTime（時間字串）, Weekday（星期）
*    - 更新後自動保存到Flash
*
* 2. 鬧鐘設定（SetAlarm）
*    - 設定或修改指定索引的鬧鐘
*    - 參數：
*      * Index：鬧鐘索引（0 ~ MAX_ALARMS-1）
*      * DateTime：時間設定
*      * DeviceID：裝置ID（1-8）
*      * Switch：開關狀態
*      * Repeat_Days：重複日期陣列
*      * Enabled：啟用狀態
*
* 3. 鬧鐘管理
*    - GetAlarms：獲取所有鬧鐘列表
*    - DeleteAlarm：刪除指定索引的鬧鐘
*
* 4. 裝置控制
*    - DeviceOpen：開啟指定裝置
*    - DeviceClose：關閉指定裝置
*    - 參數：DeviceID（1-8）
* 5. 模式設定
*    - Manual：手動模式
*    - Remote：遠端模式

* 注意事項：
* 1. 所有回應都以\r\n結尾
* 2. 裝置ID範圍檢查（1-8）
* 3. 鬧鐘索引範圍檢查（0 ~ MAX_ALARMS-1）
* 4. 模式切換時需要重置相關狀態
* 5. 重要操作都會保存到Flash
*
* 相關函數：
* - parseTimeString：解析時間字串
* - parseAlarmDateTime：解析鬧鐘時間字串
* - Flash_SaveCurrentTime：保存當前時間
* - Flash_SaveAlarms：保存鬧鐘設定
* - Device_Open/Close：控制裝置開關
*/

void Process_JSON_Command(const char* jsonStr)
{
    char response[256] = {0};
    cJSON *root = cJSON_Parse(jsonStr);
    
    if(root == NULL)
    {
        sprintf(response, "{\"status\":\"error\",\"msg\":\"Invalid JSON format\"}\r\n");
        UART_SEND6(response);
        return;
    }

    cJSON *cmd = cJSON_GetObjectItem(root, "Operating");
    cJSON *modeCmd = cJSON_GetObjectItem(root, "Mode");
    
    // 更新時間命令
    if(strcmp(cmd->valuestring, "ReturnTime") == 0)
    {
        cJSON *dateTime = cJSON_GetObjectItem(root, "CurrentTime");
        cJSON *weekday = cJSON_GetObjectItem(root, "Weekday");
        
        if(dateTime && weekday)
        {
            // 解析並設置時間
            RTC_Init();
            parseTimeString(dateTime->valuestring);
            Now_Date.WeekDay = atoi(weekday->valuestring);	
            SetTime = HAL_OK;
            // 保存到Flash
            Flash_SaveCurrentTime();
            sprintf(response, "{\"status\":\"ok\",\"msg\":\"Time updated\"}\r\n");
            Flash_LoadCurrentTime();//更新時間
        }
    }
    // 設置鬧鐘命令
    else if(strcmp(cmd->valuestring, "SetAlarm") == 0)
    {
        cJSON *index = cJSON_GetObjectItem(root, "Index");
        cJSON *dateTime = cJSON_GetObjectItem(root, "DateTime");
        cJSON *deviceId = cJSON_GetObjectItem(root, "DeviceID");				
        cJSON *light = cJSON_GetObjectItem(root, "Switch");
        cJSON *repeat = cJSON_GetObjectItem(root, "Repeat_Days");
        cJSON *enabled = cJSON_GetObjectItem(root, "Enabled");
        
        if(index && dateTime && deviceId && enabled && light && repeat)
        {
            int idx = index->valueint;
            if(idx >= 0 && idx < MAX_ALARMS)
            {
                // 驗證 DeviceID
                int device_id = deviceId->valueint;
                if(device_id >= 1 && device_id <= 8)
                {
                    // 設置鬧鐘參數
                    parseAlarmDateTime(dateTime->valuestring, &alarms[idx]);
                    alarms[idx].enabled = enabled->valueint;
                    alarms[idx].device_id = device_id;
                    alarms[idx].light_action = light->valueint;
                    
                    // 設置重複日期
                    memset(alarms[idx].repeat_days, 0, sizeof(alarms[idx].repeat_days));
                    if(repeat && (repeat->type & cJSON_Array))
                    {
                        for(int i = 0; i < cJSON_GetArraySize(repeat); i++)
                        {
                            cJSON *day = cJSON_GetArrayItem(repeat, i);
                            if(day && day->valueint >= 1 && day->valueint <= 7)
                            {
                                alarms[idx].repeat_days[day->valueint-1] = 1;
                            }
                        }
                    }
                    
                    // 保存到Flash
                    Flash_SaveAlarms();
                    
                    sprintf(response, "{\"status\":\"ok\",\"msg\":\"Alarm set\"}\r\n");
                }
                else
                {
                    sprintf(response, "{\"status\":\"error\",\"msg\":\"Invalid DeviceID (1-8)\"}\r\n");
                }
            }
            else
            {
                sprintf(response, "{\"status\":\"error\",\"msg\":\"Invalid alarm index\"}\r\n");
            }
        }
        else
        {
            sprintf(response, "{\"status\":\"error\",\"msg\":\"Missing required parameters\"}\r\n");
        }
    }
    else if(strcmp(cmd->valuestring, "GetAlarms") == 0)
    {
        GetAlarmList();
    }
    else if(strcmp(cmd->valuestring, "DeleteAlarm") == 0)
    {
        cJSON *index = cJSON_GetObjectItem(root, "Index");
        if(index && index->type == cJSON_Number)
        {
            DeleteAlarm(index->valueint);
        }
    }
    else if(strcmp(cmd->valuestring, "IntAlarms") == 0)
    {
        // 清空所有鬧鐘配置
        memset(alarms, 0, sizeof(alarms));
        
        // 設定16個預設鬧鐘
        for(int i = 0; i < MAX_ALARMS; i++)
        {
            alarms[i].year = 2099;            // 年份：2099
            alarms[i].month = 12;             // 月份：12月
            alarms[i].date = 31;              // 日期：31日
            alarms[i].hour = 0;               // 小時：0
            alarms[i].minute = 0;             // 分鐘：0
            alarms[i].second = 0;             // 秒：0
            alarms[i].enabled = 0;            // 啟用狀態：停用
            alarms[i].weekday = 0;            // 星期幾：不設定
            alarms[i].device_id = (i / 2) + 1;// 設備ID：每兩個鬧鐘對應一個設備
            alarms[i].light_action = i % 2;   // 燈光動作：偶數索引=關閉，奇數索引=開啟
            
            // 清空重複日期
            memset(alarms[i].repeat_days, 0, sizeof(alarms[i].repeat_days));
        }
        // 將初始化後的鬧鐘配置保存到Flash
        Flash_SaveAlarms();
    }
    else if(strcmp(cmd->valuestring, "GetData") == 0)
    {
        if(Task1Flag == Waiting)
            Task1Flag = SendDataTask;
    }
    // 開啟裝置
    else if(strcmp(cmd->valuestring, "DeviceOpen") == 0)
    {
        cJSON *deviceId = cJSON_GetObjectItem(root, "DeviceID");
        if(deviceId)
        {
            int DeviceID = deviceId->valuestring[0] - '0';
            if(DeviceID >= 1 && DeviceID <= 8)
            {
                Device_Open(DeviceID);
                deviceArray[DeviceID-1].expectedState = DEVICE_ON;
                deviceArray[DeviceID-1].appCommandReceived = true;
                deviceArray[DeviceID-1].lastCommandTime = HAL_GetTick();
                Flash_SaveExpectedStates();
            }
        }
    }
    // 關閉裝置
    else if(strcmp(cmd->valuestring, "DeviceClose") == 0)
    {
        cJSON *deviceId = cJSON_GetObjectItem(root, "DeviceID");
        if(deviceId)
        {
            int DeviceID = deviceId->valuestring[0] - '0';
            if(DeviceID >= 1 && DeviceID <= 8)
            {
                Device_Close(DeviceID);
                deviceArray[DeviceID-1].expectedState = DEVICE_OFF;
                deviceArray[DeviceID-1].appCommandReceived = true;
                deviceArray[DeviceID-1].lastCommandTime = HAL_GetTick();
                Flash_SaveExpectedStates();
            }
        }
    }
    else if(strcmp(cmd->valuestring, "ClearState") == 0)
    {
        state_clear = 1;
    }
    // 模式設定
    else if(modeCmd != NULL)
    {
        if(strcmp(modeCmd->valuestring, "Manual") == 0)
        {
            ModeTask = Manual;
        }
        else if(strcmp(modeCmd->valuestring, "Remote") == 0)
        {
            ModeTask = Remote;
            for(int i = 0; i < 8; i++)
            {
                //AppOnFlag[i] = (DeviceStatus[i] == 1) ? 1 : 0;
            }
        }
    }

    if(strlen(response) > 0)
    {
        UART_SEND6(response);
    }
    cJSON_Delete(root);
}
/**
* @brief 檢查鬧鐘是否為重複性鬧鐘
*
* @param alarm 要檢查的鬧鐘結構體
* @return bool 
*         - true: 鬧鐘設定為重複性鬧鐘（至少有一天被設定為重複）
*         - false: 鬧鐘為一次性鬧鐘（沒有設定重複日期）
*
* @note 陣列大小固定為7，對應一週七天
*/

bool isRepeatAlarm(Alarm_TypeDef alarm)
{
    for(int i = 0; i < 7; i++)
    {
        if(alarm.repeat_days[i]) return true;
    }
    return false;
}
/**
* @brief 解析日期時間字串並設定鬧鐘結構體
* @author Lingaga
* @param dateTimeStr 日期時間字串，格式為 "YYYY/MM/DD HH:MM:SS"
* @param alarm 指向要設定的鬧鐘結構體的指針
* @return void
* @note
* 1. 使用sscanf進行字串解析，需要確保輸入字串格式正確
* 2. 年份只保存後兩位，因此不適合處理跨世紀的日期
* 3. 函式不會驗證日期的有效性（如2月30日）
* 4. 如果解析失敗，鬧鐘結構體的值將保持不變
*/
void parseAlarmDateTime(const char* dateTimeStr, Alarm_TypeDef* alarm)
{
    int year, month, date, hour, minute, second;
    if(sscanf(dateTimeStr, "%d-%d-%d %d:%d:%d", &year, &month, &date, &hour, &minute, &second) == 6)
    {
        alarm->year = year % 100;
        alarm->month = month;
        alarm->date = date;
        alarm->hour = hour;
        alarm->minute = minute;
        alarm->second = second;
    }
}
/**
* @brief 獲取並發送所有鬧鐘列表
* @author Lingaga
* @details 此函數負責將系統中的鬧鐘資訊通過UART發送給客戶端。
* 發送過程分為三個階段：
* 1. 發送鬧鐘總數
* 2. 發送各個鬧鐘的詳細資訊
* 3. 發送結束標記
*
* 相關函數：
* - SendAlarmCount(): 發送鬧鐘總數
* - SendSingleAlarm(): 發送單個鬧鐘詳細資訊
* @warning 
* 1. 需要確保有足夠的堆空間用於JSON字串處理
* 2. UART傳輸速率需要匹配延遲時間設定
*/
void GetAlarmList()
{
    int enabledCount = 0;
    
    // 首先計算啟用的鬧鐘數量
    for(int i = 0; i < MAX_ALARMS; i++)
    {
		enabledCount++;
    }
    // 發送鬧鐘總數信息
    SendAlarmCount(enabledCount);
    osDelay(10);
    for(int i = 0; i < MAX_ALARMS; i++)
    {
		SendSingleAlarm(i, &alarms[i]);     
		osDelay(10);
    }
    cJSON *endMark = cJSON_CreateObject();
    if(endMark)
    {
        cJSON_AddStringToObject(endMark, "Operating", "AlarmListEnd");
        char *jsonStr = cJSON_Print(endMark);
        if(jsonStr)
        {
            char *response = malloc(strlen(jsonStr) + 3);
            if(response)
            {
                strcpy(response, jsonStr);
                strcat(response, "\r\n");
                UART_SEND6(response);
                free(response);
            }
            free(jsonStr);
        }
        cJSON_Delete(endMark);
    }
}
/**
* @brief 鬧鐘資訊傳輸處理模組
* @date 2024-01-20
* @author Lingaga
* 此模組負責處理單個鬧鐘資訊的 JSON 格式化與傳輸。
* 使用 cJSON 函式庫處理 JSON 資料結構，透過 UART 傳輸。

* @warning
* 1. 需確保傳入的 alarm 指標非 NULL
* 2. 需注意動態記憶體分配可能失敗的情況
* 3. UART 傳輸速率需要匹配系統設定
*/
// 發送單個鬧鐘資料的函數
void SendSingleAlarm(int index, const Alarm_TypeDef *alarm)
{
    if (!alarm) return;
    
    cJSON *root = cJSON_CreateObject();
    if (!root) return;
    
    // 添加基本信息
    cJSON_AddStringToObject(root, "Operating", "AlarmInfo");
    cJSON_AddNumberToObject(root, "Index", index);  // 修改: 大寫 I
    
    // 格式化日期時間字符串
    char dateTimeStr[32];
    snprintf(dateTimeStr, sizeof(dateTimeStr),"20%02d/%02d/%02d %02d:%02d:%02d",alarm->year, alarm->month, alarm->date,alarm->hour, alarm->minute, alarm->second);
    cJSON_AddStringToObject(root, "DateTime", dateTimeStr);
    
    // 添加設備ID
    cJSON_AddNumberToObject(root, "DeviceID", alarm->device_id);
    
    // 添加其他屬性
    
    cJSON_AddNumberToObject(root, "Switch", alarm->light_action);
    
    // 創建重複日期數組
    cJSON *repeatArray = cJSON_CreateArray();
    if (repeatArray)
    {
        for(int j = 0; j < 7; j++)
        {
            if(alarm->repeat_days[j])
            {
                cJSON_AddItemToArray(repeatArray, cJSON_CreateNumber(j+1));
            }
        }
        cJSON_AddItemToObject(root, "Repeat_Days", repeatArray);  // 修改: 大寫 R_D
    }
    cJSON_AddNumberToObject(root, "Enabled", alarm->enabled);  // 修改: 大寫 E
    // 轉換為字符串並發送
    char *jsonStr = cJSON_Print(root);
    if(jsonStr)
    {
        char *response = malloc(strlen(jsonStr) + 3);
        if(response)
        {
            strcpy(response, jsonStr);
            strcat(response, "\r\n");
            UART_SEND6(response);
            free(response);
        }
        free(jsonStr);
    }
    
    cJSON_Delete(root);
}

/**
* @file alarm_count_handler.c (建議的檔案名稱)
* @brief 鬧鐘數量資訊傳輸處理模組
* @author Lingaga
* @date 2024-01-20
*
* @details
* 此模組負責處理鬧鐘總數的 JSON 格式化與傳輸。
* 使用 cJSON 函式庫處理 JSON 資料結構，透過 UART 進行資料傳輸。
*/
void SendAlarmCount(int totalEnabled)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) return;
    
    cJSON_AddStringToObject(root, "Operating", "AlarmCount");
    cJSON_AddNumberToObject(root, "Total", totalEnabled);
    
    char *jsonStr = cJSON_Print(root);
    if(jsonStr)
    {
        char *response = malloc(strlen(jsonStr) + 3);
        if(response)
        {
            strcpy(response, jsonStr);
            strcat(response, "\r\n");
            UART_SEND6(response);
            free(response);
        }
        free(jsonStr);
    }
    
    cJSON_Delete(root);
}
// 刪除鬧鐘
void DeleteAlarm(int index)
{
    if(index >= 0 && index < MAX_ALARMS)
    {
        memset(&alarms[index], 0, sizeof(Alarm_TypeDef));
        Flash_SaveAlarms();
        
        char response[64];
        sprintf(response, "{\"status\":\"ok\",\"msg\":\"Alarm %d deleted\"}\r\n", index);
        UART_SEND6(response);
    }
    else
    {
        UART_SEND6("{\"status\":\"error\",\"msg\":\"Invalid alarm index\"}\r\n");
    }
}
/**
* @file checksum.c (建議的檔案名稱)
* @brief 校驗和計算模組
* @author Lingaga
* @date 2024-01-20
*
* @details
* 此模組提供一個簡單的校驗和計算函數，用於資料完整性驗證。
* 計算方式為對輸入資料除最後一個位元組外的所有位元組進行簡單加總。
*/
static uint8_t Calculate_Checksum(const void* data, size_t len)
{
    uint8_t sum = 0;
    const uint8_t* bytes = (const uint8_t*)data;
    for(size_t i = 0; i < len - 1; i++)
    {
        sum += bytes[i];
    }
    return sum;
}

/**
* @file weekday_converter.c (建議的檔案名稱)
* @brief 星期數值轉換字串模組
* @author Lingaga
* @date 2024-01-20
*
* @details
* 此模組提供星期數值（0-6）轉換為對應英文字串的功能。
*/
void GetWeekdayString(uint8_t weekday, char *weekStr)
{
    const char *weekdays[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    
    if(weekday < 7)
    {
        strcpy(weekStr, weekdays[weekday]);
    }
    else
    {
        strcpy(weekStr, "Unknown");
    }
}

/**
* @file alarm_trigger.c (建議的檔案名稱)
* @brief 鬧鐘觸發條件判斷模組
* @author Lingaga
* @date 2024-01-20
*
* @details
* 此模組負責判斷鬧鐘是否應該在當前時間點觸發。
* 支援兩種鬧鐘模式：
* 1. 重複性鬧鐘（按星期重複）
* 2. 一次性鬧鐘（指定具體日期）
*
* 觸發條件：
* 1. 時、分、秒必須完全匹配當前時間
* 2. 對於重複性鬧鐘，當天必須在重複日期內
* 3. 對於一次性鬧鐘，年月日必須完全匹配
*/
bool Should_Alarm_Trigger(const Alarm_TypeDef *alarm)
{
    // 檢查基本時間匹配
    if(alarm->hour != Now_Time.Hours || alarm->minute != Now_Time.Minutes || alarm->second != Now_Time.Seconds)
    {
        return false;
    }
    
    // 檢查重複模式
    if(alarm->repeat_days[Now_Date.WeekDay-1])
    {
        return true;
    }
    // 檢查一次性鬧鐘
    if(!isRepeatAlarm(*alarm) && alarm->year == Now_Date.Year && alarm->month == Now_Date.Month && alarm->date == Now_Date.Date)
    {
        return true;
    }
    return false;
}

/**
* @file alarm_executor.c (建議的檔案名稱)
* @brief 鬧鐘觸發動作執行模組
* @author Lingaga
* @date 2024-01-20
*
* @details
* 此模組負責執行鬧鐘觸發時的相關動作，包括：
* 1. 控制指定設備的開關狀態
* 2. 更新設備狀態標誌
* 3. 發送 JSON 格式的觸發通知
*/
void Execute_Alarm_Action(const Alarm_TypeDef *alarm, int index)
{
    // 根據設備ID執行相應的動作
		int Alarm_DeviceID = alarm->device_id;
    if(alarm->device_id >= 1 && alarm->device_id <= 8)  // 確保設備ID在有效範圍內
    {
        // 根據 light_action 執行相應操作
        if(alarm->light_action == LIGHT_ON)
        {
            // 開啟設備
            Device_Open(Alarm_DeviceID);
            deviceArray[Alarm_DeviceID-1].expectedState = DEVICE_ON;
            deviceArray[Alarm_DeviceID-1].appCommandReceived = true;
            deviceArray[Alarm_DeviceID-1].lastCommandTime = HAL_GetTick();
        }
        else if(alarm->light_action == LIGHT_OFF)
        {
            // 關閉設備
            Device_Close(Alarm_DeviceID);
            deviceArray[Alarm_DeviceID-1].expectedState = DEVICE_OFF;
            deviceArray[Alarm_DeviceID-1].appCommandReceived = true;
            deviceArray[Alarm_DeviceID-1].lastCommandTime = HAL_GetTick();
        }   
    }
}

void initDeviceStatus(void) {
    for(int i = 0; i < 8; i++) {
        deviceArray[i].expectedState = DEVICE_OFF;
        deviceArray[i].currentState = DEVICE_OFF;
        deviceArray[i].previousState = DEVICE_OFF;
        deviceArray[i].persistentErrorState = DEVICE_OFF; // 初始化為 OFF 狀態 (0)
        deviceArray[i].appCommandReceived = false;
        deviceArray[i].lastCommandTime = 0;
    }
}



/**
* @brief 將目前所有設備的期望狀態保存到 Flash
* @return bool - 保存操作是否成功
*/
bool Flash_SaveExpectedStates(void)
{
    ExpectedStateHeader_t header = {
        .mark = EXPECTED_STATE_DATA_MARK,
        .version = EXPECTED_STATE_VERSION,
        .count = 8 // 固定為 8 個設備
    };
    ExpectedStateData_t data;

    // 1. 填充狀態資料
    for(int i = 0; i < 8; i++) 
    {
        data.states[i] = deviceArray[i].expectedState;
    }

    // 2. 計算校驗和
    header.checksum = Calculate_Checksum(&header, sizeof(ExpectedStateHeader_t));
    data.checksum = Calculate_Checksum(&data, sizeof(ExpectedStateData_t));

    // 3. 擦除目標扇區
    W25QXX_Erase_Sector(FLASH_EXPECTED_STATE_ADDR / FLASH_SECTOR_SIZE);
    HAL_IWDG_Refresh(&hiwdg); // 防止看門狗超時

    // 4. 寫入頭部
    W25QXX_Write((uint8_t*)&header, FLASH_EXPECTED_STATE_ADDR, sizeof(ExpectedStateHeader_t)) ;

    // 5. 寫入資料
    W25QXX_Write((uint8_t*)&data, FLASH_EXPECTED_STATE_ADDR + sizeof(ExpectedStateHeader_t), sizeof(ExpectedStateData_t));

    HAL_IWDG_Refresh(&hiwdg);
    return true;
}

/**
* @brief 從 Flash 讀取設備的期望狀態到 deviceArray
* @return bool - 讀取並驗證是否成功
*/
bool Flash_LoadExpectedStates(void)
{
    ExpectedStateHeader_t header;
    ExpectedStateData_t data;
    uint32_t readAddr = FLASH_EXPECTED_STATE_ADDR;

    // 1. 讀取頭部
    W25QXX_Read((uint8_t*)&header, readAddr, sizeof(ExpectedStateHeader_t));
    HAL_IWDG_Refresh(&hiwdg);

    // 2. 驗證頭部
    uint8_t headerChecksum = Calculate_Checksum(&header, sizeof(ExpectedStateHeader_t));
    if (header.mark != EXPECTED_STATE_DATA_MARK ||
        header.version != EXPECTED_STATE_VERSION ||
        header.count != 8 || // 確保是 8 個設備
        header.checksum != headerChecksum)
    {
        // 頭部無效，可能尚未儲存過或資料損壞
        // 可以選擇在此處將所有 expectedState 設為預設值 (OFF)
        for(int i = 0; i < 8; i++) {
             deviceArray[i].expectedState = DEVICE_OFF; // 設為預設關閉
        }
        return false; // 表示讀取失敗或資料無效
    }

    // 3. 讀取資料
    readAddr += sizeof(ExpectedStateHeader_t);
    W25QXX_Read((uint8_t*)&data, readAddr, sizeof(ExpectedStateData_t));
    HAL_IWDG_Refresh(&hiwdg);

    // 4. 驗證資料
    uint8_t dataChecksum = Calculate_Checksum(&data, sizeof(ExpectedStateData_t));
    if (data.checksum != dataChecksum) {
        // 資料校驗失敗
        for(int i = 0; i < 8; i++) {
             deviceArray[i].expectedState = DEVICE_OFF; // 設為預設關閉
        }
        return false;
    }

    // 5. 將讀取的狀態應用到 deviceArray
    for(int i = 0; i < 8; i++) {
        // 確保讀取到的值是有效的狀態 (0 或 1)
        if (data.states[i] == DEVICE_OFF || data.states[i] == DEVICE_ON) {
             deviceArray[i].expectedState = data.states[i];
        } else {
             // 如果讀取到無效值，設為預設 OFF
             deviceArray[i].expectedState = DEVICE_OFF;
        }
    }

    return true; // 讀取成功
}
/* USER CODE END Application */
