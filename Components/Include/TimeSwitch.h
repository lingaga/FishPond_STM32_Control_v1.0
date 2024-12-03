#ifndef _TimeSwitch_H_
#define	_TimeSwitch_H_

/* USER CODE BEGIN Includes */
#include "stdint.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Define */
#define Off  0x00U
#define On   0x01U
/* USER CODE  END  Define */

/* USER CODE  BEGIN*/
typedef struct {
    uint8_t confirm;
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t isOn;
} TimerSwitch;

/* USER CODE  END*/
#endif
