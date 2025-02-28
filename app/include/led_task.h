#ifndef LED_TASK_H
#define LED_TASK_H

#include "CONFIG.h"  // Заголовочный файл для CH592


// Определение сообщений
#define MSG_LED_ON  0x01  // Сообщение для включения светодиода
#define MSG_LED_OFF 0x02  // Сообщение для выключения светодиода

// Прототипы функций
void LED_Task_Init(void);
uint8_t Send_LED_Message( uint8_t *data , uint16_t length );
uint16_t LED_Task_Handler(uint8_t taskID, uint16_t event);




#endif // LED_TASK_H