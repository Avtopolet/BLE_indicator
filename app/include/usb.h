
#pragma once
#ifndef USB_H
#define USB_H

/*
 Пример CDC USB-устройства
 Полезно для вывода логов через USB и не только
*/
#include "CH59x_common.h"

//  3.1 Requests---Abstract Control Model
#define DEF_SET_LINE_CODING          0x20   // Configures DTE rate, stop-bits, parity, and number-of-character
#define DEF_GET_LINE_CODING          0x21   // This request allows the host to find out the currently configured line coding.
#define DEF_SET_CONTROL_LINE_STATE   0x22

//  3.2 Notifications---Abstract Control Model
#define DEF_SERIAL_STATE             0x20



#define ENDPOINT_0_SIZE         64



typedef struct DevInfo {
	UINT8 UsbConfig; // USB configuration flag
	UINT8 UsbAddress; // USB device address
	UINT8 gSetupReq; /* USB control transmission command code */
	UINT8 gSetupLen; /* USB control transfer transfer length */
	UINT8 gUsbInterCfg; /* USB device interface configuration */
	UINT8 gUsbFlag; /* Various operation flags of USB devices, bit 0=bus reset, bit 1=get device descriptor, bit 2=set address, bit 3=get configuration descriptor, bit 4=set configuration */
} DevInfo_Parm;



//Line Code structure
typedef struct LINE_CODE {       //typedef struct __PACKED _LINE_CODE
	UINT32 BaudRate; /* baud rate */
	UINT8 StopBits; /* Stop bit count, 0: 1 stop bit, 1: 1.5 stop bit, 2: 2 stop bits */
	UINT8 ParityType; /* Check digit, 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space */
	UINT8 DataBits; /* Data bit count: 5, 6, 7, 8, 16 */
} LINE_CODE, *PLINE_CODE;



typedef struct _USB_SETUP_REQ_ {
	UINT8 bRequestType;
	UINT8 bRequest;
	UINT8 wValueL;
	UINT8 wValueH;
	UINT8 wIndexL;
	UINT8 wIndexH;
	UINT8 wLengthL;
	UINT8 wLengthH;
} USB_SETUP_REQ_t;

#define UsbSetupBuf     ((USB_SETUP_REQ_t *)Ep0Buffer) //USB_SETUP_REQ_t USB_SETUP_REQ_t


/**********************************************************/


/* Character Size */
#define HAL_UART_8_BITS_PER_CHAR             8

/* Stop Bits */
#define HAL_UART_ONE_STOP_BIT                1

/* Parity settings */
#define HAL_UART_NO_PARITY                   0x00

void USBDevEPnINSetStatus(UINT8 ep_num, UINT8 type, UINT8 sta);

#define ENDP0                           0x00

/* ENDP x Type */
#define ENDP_TYPE_IN                    0x00                                    /* ENDP is IN Type */

/* IN */
#define IN_ACK                          0
#define IN_NAK                          2

#define DEF_BIT_USB_ADDRESS             0x04
#define DEF_BIT_USB_SET_CFG             0x10

void SendUSBData();
void log2(UINT8 *p_send_dat, UINT8 len);

void USB_IRQHandler(void);

typedef void (*USBCallbackType)(UINT8 *p_send_dat, UINT8 len);

/*******************************************************************************
 * Function Name  : USBDevEPnINSetStatus
 * Description    : �˵�״̬���ú���
 * Input          : ep_num���˵��
 type���˵㴫������
 sta���л��Ķ˵�״̬
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USBDevEPnINSetStatus(UINT8 ep_num, UINT8 type, UINT8 sta);

/*******************************************************************************
 * Function Name  : InitCDCDevice
 * Description    : ��ʼ��CDC�豸
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitCDCDevice(USBCallbackType callback);




/*******************************************************************************
 * Function Name  : InitUSBDevPara
 * Description    : USB��صı�����ʼ��
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitUSBDevPara(void);

/*******************************************************************************
 * Function Name  : SendUSBData
 * Description    : �������ݴ���
 * Input          : p_send_dat�����͵�����ָ��
 send_len�����͵�״̬
 * Output         : None
 * Return         : ���͵�״̬
 *******************************************************************************/
void SendUSBData();

void log2(UINT8 *p_send_dat, UINT8 len);

void my_log(const char *p_send_dat, ...);
#endif /* GRANDFATHER_H */

