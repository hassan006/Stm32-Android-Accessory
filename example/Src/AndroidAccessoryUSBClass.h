/*
 * aa_switch_class.h
 *
 *  Created on: Jan 13, 2019
 *      Author: once2go
 */

#ifndef AA_CLASS_H_
#define AA_CLASS_H_

#include "usbh_core.h"

extern USBH_ClassTypeDef AA_Class;
#define USBH_AA_CLASS    &AA_Class

#define USB_AA_CLASS_CODE    		0xFF
#define USB_AA_SUB_CLASS_CODE    	0xFF
#define USB_AA_PROTOCOL    			0

typedef enum {
  AA_IDLE = 0,
  AA_READ,
  AA_READ_WAIT,
  AA_WRITE,
  AA_WRITE_WAIT
} AA_StateTypeDef;

typedef struct {
	AA_StateTypeDef state;
	uint8_t InPipe;
	uint8_t OutPipe;
	uint8_t OutEp;
	uint8_t InEp;
	uint16_t OutEpSize;
	uint16_t InEpSize;
} AA_HandleTypeDef;
#endif /* AA_CLASS_H_ */

#define USB_RX_TX_BUFFER_SIZE 		64


uint8_t rx_txBuffer[USB_RX_TX_BUFFER_SIZE];

/*
 * Data read call back
 */
void (*usbDataReceivedCallback)(uint8_t*, int);


/*
 * Write data to @rx_txBuffer and
 * call it when @rx_txBuffer is ready for transmission
 */
void accesoryWriteUSBDataFromRxTxBuffer(void);
