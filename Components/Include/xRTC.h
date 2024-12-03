#ifndef _xRTC_H_
#define	_xRTC_H_

/* USER CODE BEGIN Includes */
#include "rtc.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
extern RTC_HandleTypeDef xhrtc;
extern RTC_TimeTypeDef Now_Time;
extern RTC_DateTypeDef Now_Date;
extern char CurrentTime[20];
extern char NetTime[20];
/* USER CODE END Variables */

/* USER CODE BEGIN FunctionPrototypes */
void RTC_Init(void);
void GET_Time(void);
void SET_Alarm(void);
uint8_t Time_Offset(void);
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);
/* USER CODE END FunctionPrototypes */

#endif
