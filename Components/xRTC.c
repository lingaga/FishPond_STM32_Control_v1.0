/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "xRTC.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
RTC_TimeTypeDef Now_Time = {0};
RTC_DateTypeDef Now_Date = {0};
RTC_TimeTypeDef CheckTime= {0};
RTC_AlarmTypeDef sAlarm = {0};

RTC_HandleTypeDef xhrtc;

char CurrentTime[20]; //YYYY/MM/DD HH:MM:SS
char NetTime[20];     //YYYY/MM/DD HH:MM:SS
char CurrentWeek[40];//W:1 2 3 4 5 6 7->一~日
/* USER CODE END Variables */

/* USER CODE BEGIN Function */
void RTC_Init(void)
{
  xhrtc.Instance = RTC;
  xhrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  xhrtc.Init.AsynchPrediv = 127;
  xhrtc.Init.SynchPrediv = 255;
  xhrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  xhrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  xhrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&xhrtc) != HAL_OK)
  {
    Error_Handler();
  }
	
	Now_Time.Hours   = (NetTime[11]-'0')*10+NetTime[12]-'0';
  Now_Time.Minutes = (NetTime[14]-'0')*10+NetTime[15]-'0';
  Now_Time.Seconds = (NetTime[17]-'0')*10+NetTime[18]-'0';
	CheckTime.Hours  =Now_Time.Hours;
	CheckTime.Minutes=Now_Time.Minutes;
	CheckTime.Seconds=Now_Time.Seconds;
  Now_Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  Now_Time.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&xhrtc, &Now_Time, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
	
	//HAL_RTCEx_SetSmoothCalib(&xhrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, 12);
	//HAL_RTCEx_SetSynchroShift(&xhrtc, RTC_SHIFTADD1S_RESET, 128); 
	
  Now_Date.Month = (NetTime[5]-'0')*10+NetTime[6]-'0';
  Now_Date.Date  = (NetTime[8]-'0')*10+NetTime[9]-'0';
  Now_Date.Year  = (NetTime[2]-'0')*10+NetTime[3]-'0';
	
  if (HAL_RTC_SetDate(&xhrtc, &Now_Date, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
	
	SetTime=HAL_OK;
	/*sAlarm.AlarmTime.Seconds = Now_Time.Seconds+1;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&xhrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }	*/
}
/**
 * @brief 獲取當前的日期和時間
 *
 * 此函式會從RTC獲取當前的日期和時間，並將其格式化為一個字串存入全域變量CurrentTime中。
 *
 * 步驟如下:
 * 1. 使用HAL_RTC_GetTime從RTC獲取當前時間，並存入Now_Time變量中。
 * 2. 使用HAL_RTC_GetDate從RTC獲取當前日期，並存入Now_Date變量中。
 * 3. 使用sprintf函式將日期和時間格式化為字串"20%d/%02d/%02d %02d:%02d:%02d"
 *    的格式，並存入CurrentTime變量中。
 *
*/
void GET_Time(void)
{
	HAL_RTC_GetTime(&xhrtc,&Now_Time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&xhrtc,&Now_Date,RTC_FORMAT_BIN);
	sprintf(CurrentTime,"20%d/%02d/%02d %02d:%02d:%02d",Now_Date.Year,Now_Date.Month,Now_Date.Date,Now_Time.Hours,Now_Time.Minutes,Now_Time.Seconds);
}
/**
 * @brief 檢查當前時間是否需要進行時區偏移調整
 *
 * @return uint8_t
 *         - 1: 需要進行時區偏移調整
 *         - 0: 不需要進行時區偏移調整
 *
 * 此函式每兩個小時會做本地時間修正，若當前小時小於2，則返回1，接著讀取網路時間做校準
 *
 */
uint8_t Time_Offset(void)
{
	if((CheckTime.Hours+2)%24==Now_Time.Hours)
	{
		if(Now_Time.Minutes==CheckTime.Minutes&&Now_Time.Seconds==(CheckTime.Seconds+1))
		{
			if(Now_Time.Hours<2)
				return 1;
			else
			{
				Now_Time.Hours   = (CheckTime.Hours+2)%24;
				Now_Time.Minutes = CheckTime.Minutes;
				Now_Time.Seconds = CheckTime.Seconds;
				if (HAL_RTC_SetTime(&xhrtc, &Now_Time, RTC_FORMAT_BIN) != HAL_OK)
				{
					Error_Handler();
				}			
			}
			CheckTime.Hours=(CheckTime.Hours+2)%24;
		}
	}
	return 0;
}
void SET_Alarm(void)
{
	sAlarm.AlarmTime.Seconds = Now_Time.Seconds+1;
	if(sAlarm.AlarmTime.Seconds==60)sAlarm.AlarmTime.Seconds=0;
  //HAL_RTC_SetAlarm_IT(&xhrtc, &sAlarm, RTC_FORMAT_BIN);
}
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *xhrtc)
{
	UNUSED(xhrtc);
	GET_Time();
	SET_Alarm();
}
void parseTimeString(const char* timeStr) 
{
    int year, month, date, hour, minute, second;
    sscanf(timeStr, "%d-%d-%d %d:%d:%d", 
           &year, &month, &date, &hour, &minute, &second);
    // 更新時間
    Now_Time.Hours = hour;
    Now_Time.Minutes = minute;
    Now_Time.Seconds = second;
    Now_Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    Now_Time.StoreOperation = RTC_STOREOPERATION_RESET;
    
    // 更新日期
    Now_Date.Year = year % 100;  // 只取後兩位
    Now_Date.Month = month;
    Now_Date.Date = date;
	  
		
    // 設置RTC
    HAL_RTC_SetTime(&xhrtc, &Now_Time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&xhrtc, &Now_Date, RTC_FORMAT_BIN);
    
    // 更新檢查時間
    CheckTime.Hours = Now_Time.Hours;
    CheckTime.Minutes = Now_Time.Minutes;
    CheckTime.Seconds = Now_Time.Seconds;
}
/* USER CODE  END  Function */
