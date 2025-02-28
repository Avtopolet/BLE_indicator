/*
 Пример CDC USB-устройства
 Полезно для вывода логов через USB и не только
*/
#include <stdio.h>
#include <stdarg.h>
#include "CH59x_common.h"
#include "include/usb.h"

// Дескрипторы устройства
const UINT8 TAB_USB_CDC_DEV_DES[18] = {
        0x12,       // bLength - Размер дескриптора в байтах (для дескриптора устройства размер 18 байт)
		0x01,       // bDescriptorType - Тип - Device Descriptor (0x01)
		0x10, 0x01, // bcdUSB - Номер спецификации USB, с которой совместимо устройство.
					//    Например,USB 2.0 кодируется как 0x0200, USB 1.1 как 0x0110 и USB 1.0 как 0x0100.
		0x02,       // bDeviceClass - Код класса (назначается организацией USB Org) (0x02 = Communications Device Class)
			        //    Если равно 0, то каждый интерфейс указывает свой собственный код класса.
			        //    Если равен 0xFF, то код класса определяется вендором.
			        //    Иначе поле содержит код стандартного класса.
		0x00,       // bDeviceSubClass - Код подкласса (назначается организацией USB Org)
		0x00,       // bDeviceProtocol - Код протокола (назначается организацией USB Org)
		0x40,       // bMaxPacketSize - Максимальный размер пакета для конечной точки 0. Допустимые размеры 8, 16, 32, 64
		0x86, 0x1a, // idVendor - Vendor ID, VID (назначается организацией USB Org)
		0x40, 0x80, // idProduct - Product ID, PID (назначается организацией - производителем)
		0x00, 0x30, // bcdDevice - Device Release Number (номер версии устройства)
		0x01,       // iManufacturer - Индекс строки, описывающей производителя
		0x02,       // iProduct - Индекс строки, описывающей продукт
		0x03,       // iSerialNumber - Индекс строки, содержащей серийный номер
		0x01        // bNumConfigurations - Количество возможных конфигураций
		};

/* Configuration descriptor */
const UINT8 TAB_USB_CDC_CFG_DES[] = {
        0x09,       // bLength - Размер дескриптора в байтах
		0x02,       // bDescriptorType - Дескриптор конфигурации (0x02)
		0x43, 0x00, // wTotalLength - Полная длина возвращаемых данных в байтах
		0x02,       // bNumInterfaces - Количество интерфейсов
		0x01,       // bConfigurationValue - Величина, используемая как аргумент для выбора этой конфигурации
		0x00,       // iConfiguration - Индекс строкового дескриптора, описывающего эту конфигурацию
		0x80,       // bmAttributes - bitmap
				    //    D7 зарезервировано, установлено в 1. (USB 1.0 Bus Powered)
				    //    D6 самозапитываемое (Self Powered)
				    //    D5 удаленное пробуждение (Remote Wakeup)
				    //    D4..0 зарезервировано, установлено в 0.
		0x30,       // bMaxPower - Максимальное энергопотребление в единицах 2 мА

		// The following is the descriptor of interface 0 (CDC interface)
		// Дескрипторы интерфейса 0
		0x09,       // bLength - Размер дескриптора в байтах (9 байт)
		0x04,       // bDescriptorType - Дескриптор интерфейса (0x04)
		0x00,       // bInterfaceNumber - Индекс (порядковый номер) интерфейса (интерфейс 0)
		0x00,       // bAlternateSetting - Величина, используемая для выбора альтернативной установки
		0x01,       // bNumEndpoints - Количество конечных точек, используемых в интерфейсе
		0x02,       // bInterfaceClass - Код класса (назначается организацией, следящей за стандартами USB) (0x02 = Communications Interface Class)
		0x02,       // bInterfaceSubClass - Код подкласса (назначается организацией, следящей за стандартами USB) (0x02 = Abstract Control Model)
		0x01,       // bInterfaceProtocol - Код протокола (назначается организацией, следящей за стандартами USB) (01h = AT Commands: V.250 etc)
		0x00,       // iInterface - Индекс строкового дескриптора, описывающего этот интерфейс

		// A value of 0x24 indicates it is a class-specific interface descriptor.
		// Please see the definition of CS_INTERFACE in Table 12 in the CDC 1.20 specification
		0x05,       // bFunctionLength - Size of this functional descriptor, in bytes.
		0x24,       // bDescriptorType - CS_INTERFACE
		0x00,       // bDescriptorSubtype - Header. This is defined in Table 13, which defines this as a header.
		0x10, 0x01, // bcdCDC - 0110h (USB Class Definitions for Communications Devices Specification release number in binary-coded decimal.)

		0x04,       // bFunctionLength - Size of this functional descriptor, in bytes.
		0x24,       // bDescriptorType - CS_INTERFACE
		0x02,       // Abstract Control Management Functional Descriptor.
		0x02,       // ?

		0x05,       // bFunctionLength - Size of this functional descriptor, in bytes.
		0x24,       // bDescriptorType - CS_INTERFACE
		0x06,       // Union Functional Descriptor
		0x00,       // bControlInterface - Interface number of the control (Communications Class) interface
		0x01,

		0x05,       // bFunctionLength - Size of this functional descriptor, in bytes.
		0x24,       // bDescriptorType - CS_INTERFACE
		0x01,       // bDescriptorSubtype - Call Management Functional Descriptor.
		0x01,       // bmCapabilities - Indicate that the device handles call management itself (bit D0 is set),
			        //    and will process commands multiplexed over the data interface in addition to commands sent
			        //    using SendEncapsulatedCommand (bit D1 is set).
		0x00,       // bDataInterface - Indicates that multiplexed commands are handled via data interface 01h
			        //    (same value as used in the UNION Functional Descriptor)

		// Дескрипторы конечной точки
		// Interrupt upload endpoint descriptor
		0x07,       // bLength - Размер дескриптора в байтах (7 байт)
		0x05,       // bDescriptorType - Дескриптор конечной точки (0x05)
		0x84,       // bEndpointAddress - Адрес конечной точки (конечная точка 4, бит 7 = 1 -> In)
                    //    биты 0..3 номер конечной точки
			        //    биты 4..6 зарезервированы, установлены в 0
			        //    бит 7 направление 0 = Out, 1 = In (для конечных точек игнорируется)
		0x03,       // bmAttributes - Bitmap (0x03 = 00000011 -> Interrupt)
				    //       указывает тип передачи.
				    //       Это могут быть передачи Control, Interrupt, Isochronous или Bulk.
				    //       Если указана контрольная точка изохронного типа, могут быть выбраны дополнительные атрибуты,
				    //       такие как синхронизация и типы использования.
				    //    биты 0..1 тип передачи
				    //       00 = Control
				    //       01 = Isochronous
				    //       10 = Bulk
				    //       11 = Interrupt
				    //    биты 2..7 зарезервированы. Если конечная точка изохронная, то:
				    //    биты 3..2 = тип синхронизации (режим Iso)
				    //       00 = No Synchronisation
				    //       01 = Asynchronous
				    //       10 = Adaptive
				    //       11 = Synchronous
				    //    биты 5..4 = тип использования Usage Type (режим Iso)
				    //       00 = конечная точка данных
				    //       01 = конечная точка обратной связи (Feedback Endpoint)
				    //       10 = явная конечная точка обратной связи данных (Explicit Feedback Data Endpoint)
				    //       11 = зарезервировано
		0x08, 0x00, // wMaxPacketSize - Максимальный размер пакета этой конечной точки, подходящий для отправки или приема (8 байта)
		0x01,       // bInterval - Интервал для того, чтобы опросить передачи данных конечной точки.
			        //    Указывается в количестве фреймов. Поле игнорируется для конечных точек Bulk и Control.
			        //    Для конечных точек Isochronous должно быть равно 1 и для конечных точек interrupt
			        //    может лежать в диапазоне 1..255.

		//The following is the descriptor of interface 1 (data interface)
		// Дескрипторы интерфейса 1 (data interface)
		0x09,       // bLength - Размер дескриптора в байтах (9 байт)
		0x04,       // bDescriptorType - Дескриптор интерфейса (0x04)
		0x01,       // bInterfaceNumber - Индекс (порядковый номер) интерфейса (интерфейс 1)
		0x00,       // bAlternateSetting - Величина, используемая для выбора альтернативной установки
		0x02,       // bNumEndpoints - Количество конечных точек, используемых в интерфейсе
		0x0a,       // bInterfaceClass - Код класса (назначается организацией, следящей за стандартами USB)
		0x00,       // bInterfaceSubClass - Код подкласса (назначается организацией, следящей за стандартами USB)
		0x00,       // bInterfaceProtocol - Код протокола (назначается организацией, следящей за стандартами USB)
		0x00,       // iInterface - Индекс строкового дескриптора, описывающего этот интерфейс

		// Дескрипторы конечной точки
		0x07,       // bLength - Размер дескриптора в байтах (7 байт)
		0x05,       // bDescriptorType - Дескриптор конечной точки (0x05)
		0x01,       // bEndpointAddress - Адрес конечной точки (конечная точка 1, бит 7 = 0 -> Out)
		0x02,       // bmAttributes - Bitmap (0x02 = 00000010 -> Bulk)
		0x40, 0x00, // wMaxPacketSize - Максимальный размер пакета этой конечной точки, подходящий для отправки или приема (64 байта)
		0x00,       // Поле игнорируется для конечных точек Bulk и Control.

		// Дескрипторы конечной точки
		0x07,       // bLength - Размер дескриптора в байтах (7 байт)
		0x05,       // bDescriptorType - Дескриптор конечной точки (0x05)
		0x81,       // bEndpointAddress - Адрес конечной точки (конечная точка 1, бит 7 = 1 -> In)
		0x02,       // bmAttributes - Bitmap (0x02 = 00000010 -> Bulk)
		0x40, 0x00, // wMaxPacketSize - Максимальный размер пакета этой конечной точки, подходящий для отправки или приема (64 байта)
		0x00,       // Поле игнорируется для конечных точек Bulk и Control.
		};

/* Device qualified descriptor */
const UINT8 My_QueDescr[] = { 0x0A, 0x06, 0x00, 0x02, 0xFF, 0x00, 0xFF, 0x40,
		0x01, 0x00 };

const UINT8 TAB_USB_LID_STR_DES[] = { 0x04, 0x03, 0x09, 0x04 };

const UINT8 USB_DEV_PARA_CDC_SERIAL_STR[] = "WCH121212TS1";
const UINT8 USB_DEV_PARA_CDC_PRODUCT_STR[] = "USB2.0 To Serial Port";
const UINT8 USB_DEV_PARA_CDC_MANUFACTURE_STR[] = "wch.cn";

UINT8 SetupReqCode, SetupLen;

//__aligned(4) 
UINT8 Ep0Buffer[ENDPOINT_0_SIZE];
//__aligned(4) 
UINT8 Ep1Buffer[ENDPOINT_0_SIZE];     //IN

DevInfo_Parm devinf;

LINE_CODE Uart0Para;

//__aligned(4) 
UINT8 Ep1OUTDataBuf[ENDPOINT_0_SIZE];



/* Save USB interrupt status -> change to several groups of operation methods */

// Круговые буферы, вдруг прерывания быстрее приходят чем обработка в основном цикле программы.

UINT8 ep0_send_buf[256];

UINT8 log_buf[1000];
volatile int start = 0;
volatile int end = 0;

const UINT8 *pDescr;



USBCallbackType g_USBCallbackType;

/*
Вообще можно все реализовать только в методе перывания избавив основной цикл программы
от работы с USB, но видимо разделили чтоб код прерывания отрабатавал мксимально быстро
а реальная обработка проходила в основном цикле программы.
В прерывании копиуем пришедшие данные.
Слушает ендпоиинты 1 (CDC) и 2 (VENDOR DEVICE) и копирует пришедшие по DMA данные в ОЗУ для дальнейшего вывода
в методе USB_IRQProcessHandler
А так же обрабатывает endpoint 0 (обрабатывает соединения? че делает? просто ответы шлет? даже не копирует?)
 */
//__attribute__((interrupt("WCH-Interrupt-fast")))
//__attribute__((section(".highcode")))
void USB_IRQHandler(void) {
	UINT8 j;
	static PUINT8 pDescr;
	UINT8 len;
	UINT8 data_dir = 0;
	UINT8 i;

	// & 0x02, USB transmission completion trigger
	if (R8_USB_INT_FG & RB_UIF_TRANSFER) {
	    // Если мы тут, данные пришли и с ними можно работать
		if ((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) { // & 0x30 = 110000b, то есть подходит все кроме 11=idle,
		    // Мы тут только если не idle
            /*
            MASK_UIS_TOKEN (4 и 5 биты в R8_USB_INT_ST) определяют тип пакета:
               00 - OUT пакет (значит данные пришли к нам на устройство от host)
               10 - IN пакет (значит host запрашиват нет ли на устройстве данных для него)
               01 - не определен
               11 - idle - состояние линии между пакетами

               А где SETUP? А для SETUP отдельный флаг RB_UIS_SETUP_ACT
               который означает
               indicate SETUP token & 8 bytes setup request received for USB device mode
               А то что мы добавляем UIS_TOKEN_SETUP это видимо для больей похожести на стандарт
               где SETUP это отдельный PID на равне с IN, OUT и SOF
            */


			// Сохраним данные пакета для обработки в бесконечном цикле в функции USB_IRQProcessHandler
			len = R8_USB_RX_LEN; // длинна пакета

			// MASK_UIS_ENDP = 0x0F - in device mode, the endpoint number of the current USB transfer transaction.
			// На устройстве USB в режиме device, а значит первые 4 бита в usb_irq_pid (R8_USB_INT_ST) номер ендпоинта
			// а 5 и 6 биты как уже знаем это тип пакета (IN, OUT, не определен и Idle)
			switch (R8_USB_INT_ST & 0x3f) { // 0x3f = MASK_UIS_TOKEN | MASK_UIS_ENDP = 0x30 | 0x0F
			// тип пакета и номер ендпоинта в одном флаконе


			case UIS_TOKEN_OUT | 1: {
			    // ендпоинт 1, host прислал данные
				if ( R8_USB_INT_FG & RB_U_TOG_OK) {
				    // такого условия нет во втором методе (вызываемом в основном цикле)
				    // потому что там проверка usb_irq_flag==1, а мы его тут сбросим если условие не выполниться
				    // а значит в основном цикле если usb_irq_flag==1 то и автоматически выполняется условие
				    // R8_USB_INT_FG & RB_U_TOG_OK
					/*
					RB_U_TOG_OK или RB_UIS_TOG_OK могут использоваться для определения того,
					соответствует ли бит тригера синхронизации полученного пакета данных
					биту тригера синхронизации конечной точки. Если данные синхронны, то они действительны.
					Если данные не являются синхронными, они должны быть проигнорированы.
					Каждый раз, когда обрабатывается прерывание отправки или получения по USB,
                    бит запуска синхронизации соответствующей конечной точки должен быть корректно изменен,
                    чтобы определить, являются ли отправленный в следующий раз пакет данных и полученный
                    в следующий раз пакет данных синхронными.
                    Кроме того, RB_UEP_AUTO_TOG может быть настроен на автоматическое переключение соответствующего
                    бита запуска синхронизации после успешной отправки или получения.

                    Оригинал:
					 RB_U_TOG_OK or RB_UIS_TOG_OK can be used to judge whether the synchronization trigger bit
					 of the data packet received matches the synchronization trigger bit of the endpoint.
					 If the data is synchronous, the data is valid. If the data is not synchronous,
					 the data should be discarded. Every time the USB sending or receiving interrupt is processed,
					 the synchronization trigger bit of corresponding endpoint should be modified correctly to detect whether
					 the data packet sent next time and the data packet received next time are synchronous.
					 In addition, RB_UEP_AUTO_TOG can be set to automatically flip the corresponding
					 synchronization trigger bit after sending or receiving successfully.
					 */
					// То есть тут будем только ели флаг Synchronous уствнвлен в R8_USB_INT_FG
					R8_UEP1_CTRL ^= RB_UEP_R_TOG; // RB_UEP_R_TOG = 0x80, инверсия бита управления
					R8_UEP1_CTRL = (R8_UEP1_CTRL & 0xf3) | 0x08; // OUT_NAK
					// сбросить 2 и 3 биты и установить бит 3 в 1 => 3 и 2 биты = 10 => Respond to NAK or busy;
					// Копируем пришедшие данные по 4 байта за цикл
					// во втором методе нет | 0x08
					// R8_UEP1_CTRL = R8_UEP1_CTRL & 0xf3; // OUT_ACK
					// но мы приняли данные, почему ответили что NAK? То что что и Busy? И что тогда? Повторит?
					// Отвечаем NAK потому что буфер не пуст?
					for (j = 0; j < (ENDPOINT_0_SIZE / 4); j++)
						((UINT32*) Ep1OUTDataBuf)[j] = ((UINT32*) Ep1Buffer)[j];
                    //Ep1OUTDataBuf
//                    log2(Ep1OUTDataBuf, len);
					if(g_USBCallbackType)
					{
						g_USBCallbackType(Ep1OUTDataBuf, len);
					}
                    R8_UEP1_CTRL = R8_UEP1_CTRL & 0xf3; // OUT_ACK
                    // А как это работает? Мы и в прерываниях обрабатываем и тут? Аз зачем?
				}
			}
				break;

			case UIS_TOKEN_IN | 1: {
			    // ендпоинт 1, host спришивает есть ли для него данные, говорим что нет
				R8_UEP1_CTRL ^= RB_UEP_T_TOG;
				SendUSBData();
//				R8_UEP1_CTRL = (R8_UEP1_CTRL & 0xfc) | IN_NAK; // IN_NAK 1 и 0 биты = 10 => Respond to NAK or busy;
			}
				break;

			case UIS_TOKEN_OUT | 0: {    // endpoint 0
			    // а почему тут не интвертим R8_UEP0_CTRL ^= RB_UEP_R_TOG ???
				if ( R8_USB_INT_FG & RB_U_TOG_OK)
					R8_UEP0_CTRL = (R8_UEP0_CTRL & 0xf3) | 0x08; // OUT_NAK

				len = R8_USB_RX_LEN;
				if (len) {
					switch (SetupReqCode) {
					/* Set up the serial port */
					case DEF_SET_LINE_CODING: {
						UINT32 set_bps;
						UINT8 data_bit;
						UINT8 stop_bit;
						UINT8 ver_bit;

						memcpy(&set_bps, Ep0Buffer, 4);
						stop_bit = Ep0Buffer[4];
						ver_bit = Ep0Buffer[5];
						data_bit = Ep0Buffer[6];

						Uart0Para.BaudRate = set_bps;
						Uart0Para.StopBits = stop_bit;
						Uart0Para.ParityType = ver_bit;
						Uart0Para.DataBits = data_bit;

						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_ACK;
						break;
					}
					default:
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_NAK;
						break;
					}
				} else {
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_NAK
							| UEP_T_RES_NAK;
				}

			}
				break;
			case UIS_TOKEN_IN | 0: {  // endpoint 0
			    // а почему тут не интвертим R8_UEP0_CTRL ^= RB_UEP_T_TOG ???
				R8_UEP0_CTRL = (R8_UEP0_CTRL & 0xfc) | IN_NAK; // IN_NAK

				switch (SetupReqCode) {
				/*SETUP*/
				case USB_GET_DESCRIPTOR: {
					len = (SetupLen >= ENDPOINT_0_SIZE) ?
							ENDPOINT_0_SIZE : SetupLen;
					memcpy(Ep0Buffer, pDescr, len);
					SetupLen -= len;
					pDescr += len;
					if (len) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL ^= RB_UEP_T_TOG;
						USBDevEPnINSetStatus(ENDP0, ENDP_TYPE_IN, IN_ACK);
					} else {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_ACK | UEP_T_RES_NAK;
					}
					break;
				}
				case USB_SET_ADDRESS: {
					R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT)
							| (devinf.UsbAddress);
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_NAK
							| UEP_T_RES_NAK;
					break;
				}
				case DEF_GET_LINE_CODING: {
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK
							| UEP_T_RES_NAK;
					break;
				}
				case DEF_SET_LINE_CODING: {
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_NAK
							| UEP_T_RES_NAK;
					break;
				}
				default: {
					R8_UEP0_T_LEN = 0;
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_NAK
							| UEP_T_RES_NAK;
					break;
				}
				}
			}
				break;
			}

			   // мы тут если пришел любой запрос на любой ендпоинт и в тех что мы контролируем нет ошибок
			   // например пришел SETUP?
			R8_USB_INT_FG = RB_UIF_TRANSFER; // reset, USB transmission completion interrupt flag. Write 1 to reset:
		}
		/* setup
		 When MASK_UIS_TOKEN is not idle, and RB_UIS_SETUP_ACT is also 1, it is required to process the former first,
		 and clear RB_UIF_TRANSFER once after the former is processed to make the former enter the idle status,
		 and then process the latter, and finally clear RB_UIF_TRANSFER again.
		 */
		if (R8_USB_INT_ST & RB_UIS_SETUP_ACT) {
				len = 8;
				if (len == sizeof(USB_SETUP_REQ)) {
				    // Похоже запрос будет приходить несколько раз с различными параметрами (SetupReqCode):
				    // - USB_REQ_TYP_STANDARD
				    //    - USB_GET_DESCRIPTOR - 7 раз, самый жирный запрос ответ на который не поместится в 1 пакет, поэтому есть логика слать по частям
				    //    - USB_SET_ADDRESS
				    //    - USB_GET_CONFIGURATION
				    //    - USB_SET_CONFIGURATION
				    //    - USB_CLEAR_FEATURE
				    //    - USB_GET_INTERFACE
				    //    - USB_GET_STATUS
				    // - USB_REQ_TYP_CLASS
				    //    - DEF_SET_LINE_CODING
				    //    - DEF_SET_CONTROL_LINE_STATE
				    //    - DEF_GET_LINE_CODING
				    //    - DEF_SERIAL_STATE
				    //
				    // - B добавок для каждого SetupReqCode
                    //    - R8_UEP0_T_LEN = len;
					//    - R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_NAK | UEP_T_RES_ACK;
				    // Все верно, токен всегда 8 байт, проверили на вский случай, иначе - ошибка
				    // Тут сохраняем данные которые будут обработаны в слещующих запросах
				    // SetupLen
				    // SetupReqCode
				    //
				    //
					SetupLen = UsbSetupBuf->wLengthL;
					if (UsbSetupBuf->wLengthH)
						SetupLen = 0xff;
					len = 0;
					SetupReqCode = UsbSetupBuf->bRequest;
					data_dir = USB_REQ_TYP_OUT;
					if (UsbSetupBuf->bRequestType & USB_REQ_TYP_IN)
						data_dir = USB_REQ_TYP_IN;
					if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK)
							== USB_REQ_TYP_STANDARD) {
						switch (SetupReqCode) {
						//Get descriptor 0x06
						case USB_GET_DESCRIPTOR: {
							switch ( UsbSetupBuf->wValueH) {
							case 1: {
								memcpy(ep0_send_buf, &TAB_USB_CDC_DEV_DES[0],
										sizeof(TAB_USB_CDC_DEV_DES));
								pDescr = ep0_send_buf;
								len = sizeof(TAB_USB_CDC_DEV_DES);
								break;
							}
							case 2: {
								memcpy(ep0_send_buf, &TAB_USB_CDC_CFG_DES[0],
										sizeof(TAB_USB_CDC_CFG_DES));
								pDescr = ep0_send_buf;
								len = sizeof(TAB_USB_CDC_CFG_DES);
								break;
							}
							case 3: {
								switch (UsbSetupBuf->wValueL) {
								case 0: {
									pDescr = (PUINT8) (&TAB_USB_LID_STR_DES[0]);
									len = sizeof(TAB_USB_LID_STR_DES);
									break;
								}
								case 1:  //iManufacturer
								case 2:   //iProduct
								case 3:   //iSerialNumber
								{
									UINT8 ep0_str_len;
									UINT8 *p_send;
									UINT8 *manu_str;
									UINT8 tmp;
									if (UsbSetupBuf->wValueL == 1)
										manu_str =
												(UINT8*) USB_DEV_PARA_CDC_MANUFACTURE_STR;
									else if (UsbSetupBuf->wValueL == 2)
										manu_str =
												(UINT8*) USB_DEV_PARA_CDC_PRODUCT_STR;
									else if (UsbSetupBuf->wValueL == 3)
										manu_str =
												(UINT8*) USB_DEV_PARA_CDC_SERIAL_STR;
									ep0_str_len = (UINT8) strlen(
											(char*) manu_str);
                                    // Формируем ответ:
                                    //    Длинна пакета 1 байт
                                    //    0x03 код пакета 1 байт
                                    //    Текст с 0x00 после каждой буквы (а зачем? UNICODE?) len x 2 байтр
									p_send = ep0_send_buf;
									*p_send++ = ep0_str_len * 2 + 2; // Вот и выходит длинна: len x 2 + 1 + 1
									*p_send++ = 0x03;
									for (tmp = 0; tmp < ep0_str_len; tmp++) {
										*p_send++ = manu_str[tmp];
										*p_send++ = 0x00;
									}
									pDescr = ep0_send_buf;
									len = ep0_send_buf[0];
									break;
								}
								default:
									len = 0xFF;
									break;
								}
								break;
							}
							case 6: {
								pDescr = (PUINT8) (&My_QueDescr[0]);
								len = sizeof(My_QueDescr);
								break;
							}
							default:
								len = 0xFF;
								break;
							}
							if (SetupLen > len)
								SetupLen = len;
							// SetupLen <= len
							len = (SetupLen >= ENDPOINT_0_SIZE) ?
									ENDPOINT_0_SIZE : SetupLen;
							// len = ENDPOINT_0_SIZE | SetupLen
							memcpy(Ep0Buffer, pDescr, len); // так всетаки в SETUP возвращаем чтото?
							SetupLen -= len; // Зачем если уже все отправили? Если не помещается за раз, шлем в несколько?
							pDescr += len; // Зачем если уже все отправили?
							break;
						}
						case USB_SET_ADDRESS: {
							devinf.gUsbFlag |= DEF_BIT_USB_ADDRESS;
							devinf.UsbAddress = UsbSetupBuf->wValueL; // А почему в буфер? А не сразу в R8_USB_DEV_AD?
							break;
						}
						case USB_GET_CONFIGURATION: {
							Ep0Buffer[0] = devinf.UsbConfig;
							if (SetupLen >= 1)
								len = 1;

							break;
						}
						case USB_SET_CONFIGURATION: {
							devinf.gUsbFlag |= DEF_BIT_USB_SET_CFG;
							devinf.UsbConfig = UsbSetupBuf->wValueL;
							break;
						}
						case USB_CLEAR_FEATURE: {
							len = 0;
							if (( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK)
									== USB_REQ_RECIP_DEVICE) {
								R8_UEP1_CTRL = (R8_UEP1_CTRL
										& (~( RB_UEP_T_TOG | MASK_UEP_T_RES)))
										| UEP_T_RES_NAK;
								R8_UEP4_CTRL = (R8_UEP4_CTRL
										& (~( RB_UEP_T_TOG | MASK_UEP_T_RES)))
										| UEP_T_RES_NAK;
							} else if (( UsbSetupBuf->bRequestType
									& USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
								switch ( UsbSetupBuf->wIndexL) {
								case 0x84:
									R8_UEP4_CTRL =
											(R8_UEP4_CTRL
													& (~( RB_UEP_T_TOG
															| MASK_UEP_T_RES)))
													| UEP_T_RES_NAK;
									break;
								case 0x04:
									R8_UEP4_CTRL =
											(R8_UEP4_CTRL
													& (~( RB_UEP_R_TOG
															| MASK_UEP_R_RES)))
													| UEP_R_RES_ACK;
									break;
								case 0x81:
									R8_UEP1_CTRL =
											(R8_UEP1_CTRL
													& (~( RB_UEP_T_TOG
															| MASK_UEP_T_RES)))
													| UEP_T_RES_NAK;
									break;
								case 0x01:
									R8_UEP1_CTRL =
											(R8_UEP1_CTRL
													& (~( RB_UEP_R_TOG
															| MASK_UEP_R_RES)))
													| UEP_R_RES_ACK;
									break;
								default:
									len = 0xFF;
									break;
								}
							} else
								len = 0xFF;
							break;
						}
						case USB_GET_INTERFACE: {
							Ep0Buffer[0] = 0x00;
							if (SetupLen >= 1)
								len = 1;
							break;
						}
						case USB_GET_STATUS: {
							Ep0Buffer[0] = 0x00;
							Ep0Buffer[1] = 0x00;
							if (SetupLen >= 2)
								len = 2;
							else
								len = SetupLen;
							break;
						}
						default:
							len = 0xFF;
							break;
						}
					} else if (( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK)
							== USB_REQ_TYP_CLASS) {
						if (data_dir == USB_REQ_TYP_OUT) {
							switch (SetupReqCode) {
							/* SET_LINE_CODING */
							case DEF_SET_LINE_CODING: {
								if (Ep0Buffer[4] == 0x00) {
									len = 0x00;
								} else if (Ep0Buffer[4] == 0x02) {
									len = 0x00;
								} else
									len = 0xFF;
								break;
							}
								/* SET_CONTROL_LINE_STATE */
							case DEF_SET_CONTROL_LINE_STATE: {
								len = 0;
								break;
							}
							default: {
								len = 0xFF;
								break;
							}
							}
						} else {
							switch (SetupReqCode) {
							/* GET_LINE_CODING */
							case DEF_GET_LINE_CODING: {
								pDescr = Ep0Buffer;
								len = sizeof(LINE_CODE);
								((PLINE_CODE) Ep0Buffer)->BaudRate = 
										Uart0Para.BaudRate;
								((PLINE_CODE) Ep0Buffer)->StopBits =
										Uart0Para.StopBits;
								((PLINE_CODE) Ep0Buffer)->ParityType =
										Uart0Para.ParityType;
								((PLINE_CODE) Ep0Buffer)->DataBits =
										Uart0Para.DataBits;
								break;
							}
							case DEF_SERIAL_STATE: {
								//SetupLen
								len = 2;
								Ep0Buffer[0] = 0;
								Ep0Buffer[1] = 0;
								break;
							}
							default: {
								len = 0xFF;
								break;
							}
							}
						}
					} else
						len = 0xFF;
				} else {
					len = 0xFF;
				}
				if (len == 0xFF) { // значит ошибка
					SetupReqCode = 0xFF;
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL
							| UEP_T_RES_STALL;  // STALL
				} else if (len <= ENDPOINT_0_SIZE) {
					if (SetupReqCode == USB_SET_ADDRESS) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_ACK;
					} else if (SetupReqCode == USB_SET_CONFIGURATION) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_ACK; //Ĭ�����ݰ���DATA1
					} else if (SetupReqCode == USB_GET_DESCRIPTOR) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_ACK | UEP_T_RES_ACK; //Ĭ�����ݰ���DATA1
					} else if (SetupReqCode == DEF_SET_CONTROL_LINE_STATE) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_ACK; //Ĭ�����ݰ���DATA1
					} else if (SetupReqCode == USB_CLEAR_FEATURE) {
						R8_UEP0_T_LEN = len;
						R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
								| UEP_R_RES_NAK | UEP_T_RES_ACK; //Ĭ�����ݰ���DATA1
					} else {
						if (data_dir == USB_REQ_TYP_IN) {
							R8_UEP0_T_LEN = len;
							R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
									| UEP_R_RES_NAK | UEP_T_RES_ACK; //Ĭ�����ݰ���DATA1
						} else {
							R8_UEP0_T_LEN = len;
							R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG
									| UEP_R_RES_ACK | UEP_T_RES_NAK; //Ĭ�����ݰ���DATA1
						}
					}
				} else {
				    // len > ENDPOINT_0_SIZE
					R8_UEP0_T_LEN = 0;
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK
							| UEP_T_RES_ACK;  // Ĭ�����ݰ���DATA1
				}
			/* RB_UIS_SETUP_ACT
			 In USB device mode, when this bit is 1, 8-byte SETUP request packet has been successfully received.
			 SETUP token does not affect RB_UIS_TOG_OK, MASK_UIS_TOKEN, MASK_UIS_ENDP and R8_USB_RX_LEN.

			 Не понятно что это и как происходит. С одной стороны как будто SETUP запрос успешно получен,
			 ну так и был бы обработан как любой другой пакет, но почему то именно для SETUP
			 добавляем дополнительное событие на обработку, видимо гарантировано что когда в основном цикле дойдет
			 до обработки буфер будет еще в актуальном состоянии и содержать те самые даные.
			 Пока SETUP не обработан врядли будут другие запросы IN/OUT которые перепишут, пока складывается картинка

			 А если не тут может быть еще в каком месте в программе произойдет добавление UIS_TOKEN_SETUP | 0 в массив?

			 За одно прерывание добавили две записи - текущую любую успешную и предыдущую - SETUP (как уже писал ранее не понял как такое возможно)

			 но как могут идти другие пакеты если еще SETUP (а он первый) не обработан?
			 видимо когда не в самом начале а уже раз случилось, процесс идет и в процессе еще пошли SETUP зачем то (сброить фичи?)
			 Но тогда добавили мы ее в список ПОСЛЕ текущей, а значит ли что она УЖЕ перезатерла буфер и при обработке в основном цикле SETUP
			 данные уже будут перезаписаны? или в ендпоинт 0 ниего не придет пока не сделаем "reset, USB transmission completion interrupt flag"?

			 все равно не понимаю зачем RB_UIS_SETUP_ACT когда можно было просто обработать UIS_TOKEN_SETUP?
			 как вышло что прерывание одно, а запросов два? какой то и предыдищуий SETUP?

             #define  RB_UIS_SETUP_ACT   0x80      // RO, indicate SETUP token & 8 bytes setup request received for USB device mode
			 #define  RB_UIS_TOKEN1      0x20      // RO, current token PID code bit 1 received for USB device mode
             #define  RB_UIS_TOKEN0      0x10      // RO, current token PID code bit 0 received for USB device mode
             #define  MASK_UIS_TOKEN     0x30      // RO, bit mask of current token PID code received for USB device mode
             #define  UIS_TOKEN_SETUP    0x30
            // bUIS_TOKEN1 & bUIS_TOKEN0: current token PID code received for USB device mode, keep last status during SETUP token, clear RB_UIF_TRANSFER ( RB_UIF_TRANSFER from 1 to 0 ) to set free
            //   00: OUT token PID received
            //   10: IN token PID received
            //   11: free
			 */

			R8_USB_INT_FG = RB_UIF_TRANSFER; // reset, USB transmission completion interrupt flag. Write 1 to reset:
			// ^^^^  ...and finally clear RB_UIF_TRANSFER again.
			// перевести предидщую в Idle "...and clear RB_UIF_TRANSFER once after the former is processed to make the former enter the idle status..."
		}
        if ( R8_USB_INT_FG & RB_UIF_BUS_RST) {
            R8_UEP0_CTRL = UEP_R_RES_NAK | UEP_T_RES_NAK;
            R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
            R8_UEP4_CTRL = UEP_T_RES_NAK;

            R8_USB_DEV_AD = 0x00; // сбрасываем адрес
            devinf.UsbAddress = 0;

            R8_USB_INT_FG = RB_UIF_BUS_RST;
        } else if ( R8_USB_INT_FG & RB_UIF_SUSPEND) {
            R8_USB_INT_FG = RB_UIF_SUSPEND;
        }
	}
}

/*******************************************************************************
 * Function Name  : USBDevEPnINSetStatus
 * Description    : �˵�״̬���ú���
 * Input          : ep_num���˵��
 type���˵㴫������
 sta���л��Ķ˵�״̬
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USBDevEPnINSetStatus(UINT8 ep_num, UINT8 type, UINT8 sta) {
	UINT8 *p_UEPn_CTRL;
	p_UEPn_CTRL = (UINT8*) (USB_BASE_ADDR + 0x22 + ep_num * 4);
	if (type == ENDP_TYPE_IN)
		*((PUINT8V) p_UEPn_CTRL) = (*((PUINT8V) p_UEPn_CTRL) & (~(0x03))) | sta;
	else
		*((PUINT8V) p_UEPn_CTRL) = (*((PUINT8V) p_UEPn_CTRL) & (~(0x03 << 2)))
				| (sta << 2);
}

/*******************************************************************************
 * Function Name  : InitCDCDevice
 * Description    : ��ʼ��CDC�豸
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitCDCDevice(USBCallbackType callback) {
	g_USBCallbackType = callback;
	R8_USB_CTRL = 0x00;

	R8_UEP4_1_MOD = RB_UEP4_TX_EN | RB_UEP1_TX_EN | RB_UEP1_RX_EN;

	R16_UEP0_DMA = (UINT16) (UINT32) &Ep0Buffer[0];
	R16_UEP1_DMA = (UINT16) (UINT32) &Ep1Buffer[0];

	R8_UEP0_CTRL = UEP_R_RES_NAK | UEP_T_RES_NAK;
	R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
	R8_UEP4_CTRL = UEP_T_RES_NAK;
	R8_USB_DEV_AD = 0x00;

	R8_UDEV_CTRL = RB_UD_PD_DIS;
	R8_USB_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN;
	R8_USB_INT_FG = 0xFF;

	//R8_USB_INT_EN = RB_UIE_SUSPEND | RB_UIE_TRANSFER | RB_UIE_BUS_RST;
	R8_USB_INT_EN = RB_UIE_TRANSFER;
	PFIC_EnableIRQ(USB_IRQn);

	R8_UDEV_CTRL |= RB_UD_PORT_EN;

	devinf.UsbConfig = 0; // А что такое конфигурации? Никак не обрабатываем только сохоаняем тут и возвращаем
	devinf.UsbAddress = 0;
}

/*******************************************************************************
 * Function Name  : InitUSBDevPara
 * Description    : USB��صı�����ʼ��
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void InitUSBDevPara(void) {
	UINT8 i;
	Uart0Para.BaudRate = 115200;
	Uart0Para.DataBits = HAL_UART_8_BITS_PER_CHAR;
	Uart0Para.ParityType = HAL_UART_NO_PARITY;
	Uart0Para.StopBits = HAL_UART_ONE_STOP_BIT;
}

/*******************************************************************************
 * Function Name  : SendUSBData
 * Description    : �������ݴ���
 * Input          : p_send_dat�����͵�����ָ��
 send_len�����͵�״̬
 * Output         : None
 * Return         : ���͵�״̬
 *******************************************************************************/
void SendUSBData() {
    int len = end - start;
    if (len>64) len=64;
    if (len>0) {
        memcpy(&Ep1Buffer, &log_buf[start], len);
        start += len;
        if(start>1000) start -= 1000;
        R8_UEP1_T_LEN = (UINT8) len;
        R8_UEP1_CTRL = R8_UEP1_CTRL & 0xfc; //IN_ACK
    } else {
        R8_UEP1_CTRL = (R8_UEP1_CTRL & 0xfc) | IN_NAK; // IN_NAK 1 и 0 биты = 10 => Respond to NAK or busy;
    }
}

/*******************************************************************************
 * Function Name  : SendUSBData
 * Description    : �������ݴ���
 * Input          : p_send_dat�����͵�����ָ��
 send_len�����͵�״̬
 * Output         : None
 * Return         : ���͵�״̬
 *******************************************************************************/
UINT8 SendUSBData0(UINT8 *p_send_dat, UINT16 send_len) {
	UINT8 sta = 0;
	memcpy(&Ep1Buffer[ENDPOINT_0_SIZE], p_send_dat, send_len);
	R8_UEP1_T_LEN = (UINT8) send_len;
	PFIC_DisableIRQ(USB_IRQn);
	R8_UEP1_CTRL = R8_UEP1_CTRL & 0xfc; //IN_ACK
	PFIC_EnableIRQ(USB_IRQn);
	return sta;
}

void log2(UINT8 *p_send_dat, UINT8 len) {
	memcpy(&log_buf[start], p_send_dat, len);
	end = start + len;
	if(end>1000) end -= 1000;
}

void my_log(const char *p_send_dat, ...) {
	va_list args;
	va_start(args, p_send_dat);

	char buffer[1000];
//	UINT8 len = strlen((char*) p_send_dat);
//	log2(p_send_dat, len);
	int len2 = vsnprintf(buffer, 1000, p_send_dat, args);
	va_end(args);
	SendUSBData0(buffer, len2);//+1
}

