/*
 * AndroidAccessory.c
 *
 *  Created on: Jan 13, 2019
 *      Author: once2go
 */

#include "AndroidAccessory.h"

AA androidAccessory;

uint8_t protocolVersion[2];
uint16_t vendorID = -1;
uint16_t productID = -1;

void AndroidAccessory(char *manufacturer, char *model, char *description,
		char *version, char *uri, char *serial, uint8_t isAudio) {
	androidAccessory.isConnected = 0;
	androidAccessory.creds[ACCESSORY_MANUFACTURER] = manufacturer;
	androidAccessory.creds[ACCESSORY_MODEL] = model;
	androidAccessory.creds[ACCESSORY_DESCRIPTION] = description;
	androidAccessory.creds[ACCESSORY_VERSION] = version;
	androidAccessory.creds[ACCESSORY_URI] = uri;
	androidAccessory.creds[ACCESSORY_SERIAL] = serial;
	androidAccessory.audioSupport = isAudio;
}

#define USB_ACCESSORY_PRODUCT_ID        0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID    0x2D01

//AOAv2
#define USB_AUDIO_PRODUCT_ID       			  0x2D02
#define USB_AUDIO_ADB_PRODUCT_ID   			  0x2D03
#define USB_ACCESSORY_AUDIO_PRODUCT_ID        0x2D04
#define USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID    0x2D05

uint8_t Is_AA_Connected(void) {
	return androidAccessory.isConnected;
}

USBH_StatusTypeDef USBH_AA_Process(USBH_HandleTypeDef *phost) {
	switch (phost->gState) {
	case HOST_AA_CHECK_COMPATIBILITY:
		  vendorID = phost->device.DevDesc.idVendor;
		  productID = phost->device.DevDesc.idProduct;
		  printf("vid: %04X and pid: %04X", vendorID, productID);
			if (vendorID != 0) {
				if (ANDROID_ACCESSSORY_VENDOR_ID == vendorID
						&& (productID == USB_ACCESSORY_PRODUCT_ID
								|| productID == USB_ACCESSORY_ADB_PRODUCT_ID
								|| productID == USB_AUDIO_PRODUCT_ID
								|| productID == USB_AUDIO_ADB_PRODUCT_ID
								|| productID == USB_ACCESSORY_AUDIO_PRODUCT_ID
								|| productID == USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID)) {
					androidAccessory.isConnected = 1;
					phost->gState = HOST_CHECK_CLASS;
				} else {
					phost->gState = HOST_AA_GET_PROTOCOL;
				}
			}
		break;
	case HOST_AA_GET_PROTOCOL:
		if (USBH_GetAAProtocol(phost, protocolVersion, 2) == USBH_OK) {
			if (protocolVersion[0] > 0) {
				printf("\r\nAoAv%d protocol version supported", protocolVersion[0]);
				androidAccessory.credState = 0;
				phost->gState = HOST_AA_SET_CRED_STATE;
			}
		}
		break;
	case HOST_AA_SET_CRED_STATE:
		if (androidAccessory.credState <= ACCESSORY_SERIAL) {
			if (USBH_SendAAString(phost, androidAccessory.credState,
					androidAccessory.creds[androidAccessory.credState])
					== USBH_OK) {
				androidAccessory.credState++;
			}
		} else {
			androidAccessory.credState = 0;
			if (androidAccessory.audioSupport == 1 && protocolVersion[0] > 1) {
				phost->gState = HOST_AA_SET_AUDIO;
			} else {
				phost->gState = HOST_AA_START_MODE;
			}
		}
		break;
	case HOST_AA_SET_AUDIO:
		if (USBH_AAEnableAudio(phost) == USBH_OK) {
			phost->gState = HOST_AA_START_MODE;
		}
		break;
	case HOST_AA_START_MODE:
		if (USBH_StartAccessory(phost) == USBH_OK) {
			printf("AA_ mode on");
			HAL_Delay(1000);
			MX_USB_HOST_Init();
		}
		break;
	default:
		break;
	}
	return USBH_OK;
}

USBH_StatusTypeDef USBH_GetAAProtocol(USBH_HandleTypeDef *phost, uint8_t* buff,
		uint8_t length) {
	if (phost->RequestState == CMD_SEND) {
		phost->Control.setup.b.bmRequestType =
		USB_D2H | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
		phost->Control.setup.b.bRequest = ACCESSORY_GET_PROTOCOL;
		phost->Control.setup.b.wValue.w = 0;
		phost->Control.setup.b.wIndex.w = 0;
		phost->Control.setup.b.wLength.w = length;
	}
	return USBH_CtlReq(phost, buff, length);
}

USBH_StatusTypeDef USBH_SendAAString(USBH_HandleTypeDef *phost, uint8_t index,
		const char *str) {
	if (phost->RequestState == CMD_SEND) {
		phost->Control.setup.b.bmRequestType =
		USB_H2D | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
		phost->Control.setup.b.bRequest = ACCESSORY_SEND_STRING;
		phost->Control.setup.b.wValue.w = 0;
		phost->Control.setup.b.wIndex.w = index;
		phost->Control.setup.b.wLength.w = strlen((char*) str) + 1;
	}
	return USBH_CtlReq(phost, (char*) str, strlen((char*) str) + 1);
}

USBH_StatusTypeDef USBH_StartAccessory(USBH_HandleTypeDef *phost) {
	if (phost->RequestState == CMD_SEND) {
		phost->Control.setup.b.bmRequestType =
		USB_H2D | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
		phost->Control.setup.b.bRequest = ACCESSORY_START;
		phost->Control.setup.b.wValue.w = 0;
		phost->Control.setup.b.wIndex.w = 0;
		phost->Control.setup.b.wLength.w = 0;
	}
	return USBH_CtlReq(phost, NULL, 0);
}

USBH_StatusTypeDef USBH_AAEnableAudio(USBH_HandleTypeDef *phost) {
	if (phost->RequestState == CMD_SEND) {
		phost->Control.setup.b.bmRequestType =
		USB_H2D | USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_DEVICE;
		phost->Control.setup.b.bRequest = ACCESSORY_SET_AUDIO_MODE;
		phost->Control.setup.b.wValue.w = 1;
		phost->Control.setup.b.wIndex.w = 0;
		phost->Control.setup.b.wLength.w = 0;
	}
	return USBH_CtlReq(phost, NULL, 0);
}

