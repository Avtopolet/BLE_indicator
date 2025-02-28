#include "include/led_task.h"
#include "CH59x_common.h"

// Определение пина для светодиода
#define LED_PIN     GPIO_Pin_8  // Пример: светодиод подключен к GPIO Pin 0
#define LED_PORT    GPIOA       // Пример: светодиод подключен к порту GPIOA
#define DEMO_TASK_TMOS_EVT_TEST_1   (0x0001<<0)
#define E1000MS_EVENT               0x0080
#define E100MS_EVENT                0x0040
#define  PWM16    0
//GPIOA_SetBits(bTXD1);
// Структура задачи TMOS
tmosTaskID LED_TaskID = INVALID_TASK_ID;

static int pwm_value;
int tmp;
// Структура сообщения
typedef struct {
    uint8_t command;  // Команда
    uint8_t data;     // Дополнительные данные
} LED_Message_t;

// Инициализация задачи светодиода
void LED_Task_Init(void) {
    

    // Создание задачи в TMOS
    LED_TaskID = TMOS_ProcessEventRegister(LED_Task_Handler);

    // Запуск задачи (без интервала, так как управление будет через сообщения)
    //TMOS_EventStart(LED_TaskID, 0);
    tmos_set_event(LED_TaskID,SYS_EVENT_MSG);
    tmos_start_task(LED_TaskID,SYS_EVENT_MSG,1600);
    PRINT(" >>>>>> LED_Task initialized \n\r");

}

// Обработчик задачи светодиода


uint16_t LED_Task_Handler( uint8_t task_id, uint16_t events )

{
    uint8_t message;
    uint8_t *pMsg;
    uint8_t flag;
    


    if ( events & SYS_EVENT_MSG ) 
    {
       
        if ( (pMsg = tmos_msg_receive( task_id )) != NULL )  
        {

         /*   
        mDelaymS(del);
        GPIOA_SetBits(GPIO_Pin_8);
        mDelaymS(del);
        GPIOA_ResetBits(GPIO_Pin_8);
        mDelaymS(del);
        GPIOA_SetBits(GPIO_Pin_8);
        mDelaymS(del);
        GPIOA_ResetBits(GPIO_Pin_8);
        mDelaymS(del);
        GPIOA_SetBits(GPIO_Pin_8);
         */
        PRINT(" >>>>>> MEssage recived in msg event %x \n\r",*pMsg);
        PRINT(" >>>>>> MEssage recived in msg event (d key) %d \n\r",*pMsg);
        PRINT(" >>>>>> MEssage recived in msg event pointer %x \n\r",pMsg);
        //PRINT(" >>>>>> MEssage recived in msg event condition test %x \n\r",(UINT8*)pMsg[1]);
        
        if(*pMsg == 34) {
            PRINT(" >>>>>>>>>>>> in condition %x \n\r",*pMsg);
            PRINT(" >>>>>>>>>>>> in condition second element %x \n\r",*(pMsg+1));
            PRINT(" >>>>>>>>>>>> in condition as array first element - %x, second element - %x  \n\r",pMsg[0],pMsg[1]); 
            
            int tmp = ((pMsg[0] << 8) + (pMsg[1]));
            PRINT(" >>>>>>>>>>>> in condition tmp - %d\n\r",tmp);

            tmos_start_task(LED_TaskID,E1000MS_EVENT,1600);
            GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeOut_PP_20mA);
            GPIOA_InverseBits(GPIO_Pin_8);

            }
        if(*pMsg == 35) {
            PRINT(" >>>>>>>>>>>> in condition %x, stop task \n\r",*pMsg);

            GPIOA_ResetBits(GPIO_Pin_8);
            tmos_stop_task(LED_TaskID,E1000MS_EVENT);          
            }
        if(*pMsg == 36) {
                PRINT(" >>>>>>>>>>>> in condition %x \n\r",*pMsg);
                tmp = ((pMsg[1] << 8) + (pMsg[2])); 
                
                tmos_start_task(LED_TaskID,E100MS_EVENT,160);

    
                }  
        
        


            tmos_msg_deallocate( pMsg );
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);   //返回未处理的事件,使用位异或操作符 ^ 将 SYS_EVENT_MSG 从 events 中移除。
    }

    if(events & E1000MS_EVENT) {
        PRINT(" >>>>>> E1000MS_EVENT reached \n\r");
        GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeOut_PP_20mA);
        GPIOA_InverseBits(GPIO_Pin_8);
        tmos_start_task(LED_TaskID, E1000MS_EVENT, 1600);
        //GPIOA_SetBits(GPIO_Pin_8);
        return(events ^ E1000MS_EVENT);
    }

    if(events & E100MS_EVENT) {
            PRINT(" >>>>>> E100MS_EVENT reached \n\r");
/*
            GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeOut_PP_20mA);
            
            static uint8_t LED_State;
            if(LED_State)
            {
                LED_State = 0;
                GPIOA_ResetBits(GPIO_Pin_8);
            }
            else {
                LED_State = 1;
                GPIOA_SetBits(GPIO_Pin_8);
               
            }
            
            */
            //static int pwm_value;
            GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA); // PA12 - PWM4
            
            PWMX_CLKCfg(4);                                   // cycle = 4/Fsys
            PWMX_16bit_CycleCfg(60000);                       // 16 ���ݿ���ʱ��PWMʱ������
            
            
            //int tmp = ((pMsg[1] << 8) + (pMsg[2]));

            pwm_value=tmp;
            

            PWMX_16bit_ACTOUT(CH_PWM4, pwm_value, High_Level, ENABLE);  // 50%ռ�ձ�
            PRINT(" >>>>>>>>>>>> in condition 24 pwm_value - %d\n\r",pwm_value);
            PRINT(" >>>>>>>>>>>> in condition 24 array first element - %x, second element - %x  \n\r",pMsg[0],pMsg[1]);
            /*
            PRINT(" >>>>>>>>>>>> in condition flag - %d\n\r",tmp);
            pwm_value=pwm_value+100;
            if (pwm_value == 23000) {pwm_value=0;}
            PRINT(" >>>>>> PWM value %d \n\r",pwm_value);
            */
            /*
            PWMX_16bit_ACTOUT(CH_PWM5, 15000, Low_Level, ENABLE);  // 25%ռ�ձ�
            PWMX_16bit_ACTOUT(CH_PWM6, 45000, Low_Level, ENABLE);  // 75%ռ�ձ�
            PWMX_16bit_ACTOUT(CH_PWM7, 30000, High_Level, ENABLE); // 50%ռ�ձ�
            PWMX_16bit_ACTOUT(CH_PWM8, 15000, High_Level, ENABLE); // 25%ռ�ձ�
            PWMX_16bit_ACTOUT(CH_PWM9, 45000, High_Level, ENABLE); // 75%ռ�ձ�
            */
            tmos_start_task(LED_TaskID, E100MS_EVENT, 160);
            //GPIOA_SetBits(GPIO_Pin_8);
            return(events ^ E100MS_EVENT);


    }
    // Discard unknown events
    return 0;
}










// Функция для отправки сообщения в задачу светодиода



uint8_t Send_LED_Message( uint8_t *data , uint16_t length )
{
    uint8_t *p_data;
    if ( LED_TaskID != TASK_NO_TASK )
    {
        // Send the address to the task
        p_data = tmos_msg_allocate(length); 

        if ( p_data )  
        {
            tmos_memcpy(p_data, data, length);  //将 data 指针指向的数据拷贝到 p_data 指向的缓冲区中
            tmos_msg_send( LED_TaskID, p_data );  //向目标任务发送消息
            PRINT("Data sended \n\r");
            PRINT("Send_LED_Message value >>> %x \r\n",*p_data);
            PRINT("Send_LED_Message pointer >>> %x \r\n",p_data);
            return ( SUCCESS );  //发送成功
        }
    }
    return ( FAILURE );  //发送失败
}
