/*
 * AndroidAccessory.h
 *
 *  Created on: Jan 13, 2019
 *      Author: once2go
 */

#ifndef ANDROIDACCESSORY_H_
#define ANDROIDACCESSORY_H_

#include "usbh_core.h"

#define ANDROID_ACCESSSORY_VENDOR_ID    0x18D1

//AOAv1
#define USB_ACCESSORY_PRODUCT_ID        0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID    0x2D01

//AOAv2
#define USB_AUDIO_PRODUCT_ID       			  0x2D02
#define USB_AUDIO_ADB_PRODUCT_ID   			  0x2D03
#define USB_ACCESSORY_AUDIO_PRODUCT_ID        0x2D04
#define USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID    0x2D05


#define ACCESSORY_GET_PROTOCOL          51
#define ACCESSORY_SEND_STRING			52
#define ACCESSORY_START					53

//AOAv2 SET_AUDIO_MODE
#define ACCESSORY_SET_AUDIO_MODE        58

typedef enum {
	HOST_AA_IDLE = 100,
	HOST_AA_CHECK_COMPATIBILITY,
	HOST_AA_GET_PROTOCOL ,
	HOST_AA_SET_CRED_STATE ,
	HOST_AA_SET_AUDIO ,
	HOST_AA_START_MODE
} HOST_AndroidAccessoryStateTypeDef;

typedef enum {
	ACCESSORY_MANUFACTURER = 0,
	ACCESSORY_MODEL,
	ACCESSORY_DESCRIPTION,
	ACCESSORY_VERSION,
	ACCESSORY_URI,
	ACCESSORY_SERIAL
} HOST_AACredentialsStateTypeDef;

typedef struct {
	char *creds[6];
	uint8_t credState;
	uint8_t audioSupport;
	uint8_t isConnected;
} AA;


void AndroidAccessory(char *manufacturer, char *model, char *description,
		char *version, char *uri, char *serial, uint8_t isAudioSupported);

USBH_StatusTypeDef USBH_AA_CheckStatus(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_AA_Process(USBH_HandleTypeDef *phost);






//private
USBH_StatusTypeDef USBH_GetAAProtocol(USBH_HandleTypeDef *phost, uint8_t* buff,
		uint8_t length);
USBH_StatusTypeDef USBH_SendAAString(USBH_HandleTypeDef *phost, uint8_t index,
		const char *str);
USBH_StatusTypeDef USBH_StartAccessory(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_AAEnableAudio(USBH_HandleTypeDef *phost);
//private


#endif /* ANDROIDACCESSORY_H_ */
