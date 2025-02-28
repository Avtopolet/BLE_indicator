/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 澶栬����浠庢満搴旂敤涓诲嚱鏁板強浠诲姟绯荤粺鍒濆����鍖�
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
//wchisp flash ./peripheral.hex

/******************************************************************************/
/* 澶存枃浠跺寘鍚� */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "led_task.h"

#define DC_DCDC_ENABLE
#define DC_DCDC_ENABLE = TRUE


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
__attribute__((aligned(4))) uint32_t g_cmd[64];





#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif
//static uint8_t newValueFromBLE[SIMPLEPROFILE_CHAR1_LEN];
/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   涓诲惊鐜�
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))




void Main_Circulation()
{
    GPIOA_ResetBits(GPIO_Pin_8);
    while(1)
    {
        
       // CH_LOGI(TAG,"USB PRINTF TEST");
       /*
       mDelaymS(1000);
        
        if((UINT8*)newValueFromBLE[1] == 0x22) {
			
		GPIOA_SetBits(GPIO_Pin_8);
        mDelaymS(1000);
        GPIOA_ResetBits(GPIO_Pin_8);
        mDelaymS(1000);
        GPIOA_SetBits(GPIO_Pin_8);
        mDelaymS(1000);
        GPIOA_ResetBits(GPIO_Pin_8);
        mDelaymS(1000);
//		   	
		}*/ 
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   涓诲嚱鏁�
 *
 * @return  none
 */
int main(void)
{
//#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
   PWR_DCDCCfg(ENABLE);
//#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
 // app_usb_init();
 // ch_set_log_level(CH_LOG_INFO);
  //ch_set_log_mode(CH_LOG_USB);


    //InitUSBDevPara();
	//InitCDCDevice(&MyUSBCallbackType);
	//PFIC_EnableIRQ(USB_IRQn);

    //PWR_UnitModCfg(DISABLE, UNIT_SYS_LSE);
	



#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\r\n", VER_LIB);

    
    //GPIOA_SetBits(GPIO_Pin_9);  //  settle down the TX output driver
    //GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeOut_PP_20mA); // LED pin to OutPut
    
    CH59x_BLEInit();
    HAL_Init();
    LED_Task_Init();
    GPIOA_ResetBits(GPIO_Pin_8);
    GAPRole_PeripheralInit();
    Peripheral_Init();

    Main_Circulation();
}

/******************************** endfile @ main ******************************/
