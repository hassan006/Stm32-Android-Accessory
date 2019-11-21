/*
 * aa_switch_class.c
 *
 *  Created on: Jan 13, 2019
 *      Author: once2go
 */

#include "AndroidAccessoryUSBClass.h"

static USBH_StatusTypeDef USBH_AA_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AA_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AA_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AA_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_AA_SOFProcess(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef AA_Class = { "ANDROID ACCESSORY",
USB_AA_CLASS_CODE, USBH_AA_InterfaceInit, USBH_AA_InterfaceDeInit,
		USBH_AA_ClassRequest, USBH_AA_Process, USBH_AA_SOFProcess,
		NULL };


static USBH_StatusTypeDef USBH_AA_InterfaceInit(USBH_HandleTypeDef *phost) {
	printf("AA_interface init");
	uint8_t interface = 0xFF;
	AA_HandleTypeDef *AA_Handle;
	USBH_StatusTypeDef status = USBH_FAIL;

	interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode,
	USB_AA_CLASS_CODE, USB_AA_SUB_CLASS_CODE);

	if (interface == 0xFF) /* Not Valid Interface */
	{
		printf ("Cannot Find the interface for %s class.", phost->pActiveClass->Name);
		status = USBH_FAIL;
	} else {
		USBH_SelectInterface(phost, interface);
		phost->pActiveClass->pData = (AA_HandleTypeDef *) USBH_malloc(
				sizeof(AA_HandleTypeDef));
		AA_Handle =  (AA_HandleTypeDef *) phost->pActiveClass->pData;

		if (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress
				& 0x80) {
			AA_Handle->InEp =
					(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
			AA_Handle->InEpSize =
					phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
		} else {
			AA_Handle->OutEp =
					(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
			AA_Handle->OutEpSize =
					phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
		}

		if (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress
				& 0x80) {
			AA_Handle->InEp =
					(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
			AA_Handle->InEpSize =
					phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
		} else {
			AA_Handle->OutEp =
					(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
			AA_Handle->OutEpSize =
					phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
		}

		AA_Handle->OutPipe = USBH_AllocPipe(phost, AA_Handle->OutEp);
		AA_Handle->InPipe = USBH_AllocPipe(phost, AA_Handle->InEp);

		/* Open the new channels */
		USBH_OpenPipe(phost, AA_Handle->OutPipe, AA_Handle->OutEp,
				phost->device.address, phost->device.speed,
				USB_EP_TYPE_BULK, AA_Handle->OutEpSize);

		USBH_OpenPipe(phost, AA_Handle->InPipe, AA_Handle->InEp,
				phost->device.address, phost->device.speed,
				USB_EP_TYPE_BULK, AA_Handle->InEpSize);

		USBH_LL_SetToggle(phost, AA_Handle->InPipe, 0);
		USBH_LL_SetToggle(phost, AA_Handle->OutPipe, 0);
		status = USBH_OK;
	}
	return status;
}

static USBH_StatusTypeDef USBH_AA_InterfaceDeInit(USBH_HandleTypeDef *phost) {
	AA_HandleTypeDef *AA_Handle =
			(AA_HandleTypeDef *) phost->pActiveClass->pData;

	if (AA_Handle->OutPipe) {
		USBH_ClosePipe(phost, AA_Handle->OutPipe);
		USBH_FreePipe(phost, AA_Handle->OutPipe);
		AA_Handle->OutPipe = 0; /* Reset the Channel as Free */
	}

	if (AA_Handle->InPipe) {
		USBH_ClosePipe(phost, AA_Handle->InPipe);
		USBH_FreePipe(phost, AA_Handle->InPipe);
		AA_Handle->InPipe = 0; /* Reset the Channel as Free */
	}

	if (phost->pActiveClass->pData) {
		USBH_free(phost->pActiveClass->pData);
		phost->pActiveClass->pData = 0;
	}
	return USBH_OK;
}

uint8_t write = 0;

static USBH_StatusTypeDef USBH_AA_ClassRequest(USBH_HandleTypeDef *phost) {
	AA_HandleTypeDef *AA_Handle =
			(AA_HandleTypeDef *) phost->pActiveClass->pData;
	printf("eIn: %02x eOut: %02x",  AA_Handle->InEp,  AA_Handle->OutEp);
	printf("szIn: %d szOut: %d",  AA_Handle->InEpSize,  AA_Handle->OutEpSize);
	AA_Handle->state = AA_READ;
	return USBH_OK;
}

void accesoryWriteUSBDataFromRxTxBuffer(void) {
	write = 1;
}

static USBH_StatusTypeDef USBH_AA_Process(USBH_HandleTypeDef *phost) {
	AA_HandleTypeDef *AA_Handle =
			(AA_HandleTypeDef *) phost->pActiveClass->pData;
	USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
	if (write) {
		AA_Handle->state = AA_WRITE;
		write = 0;
	}
	switch (AA_Handle->state) {
	case AA_READ:
		if (USBH_BulkReceiveData(phost, rx_txBuffer, USB_RX_TX_BUFFER_SIZE, AA_Handle->InPipe) != USBH_OK) {
			printf("Read error");
		}
		AA_Handle->state = AA_READ_WAIT;
		break;
	case AA_READ_WAIT:
		URB_Status = USBH_LL_GetURBState(phost, AA_Handle->InPipe);
		if (URB_Status == USBH_URB_DONE) {
			usbDataReceivedCallback(rx_txBuffer, USB_RX_TX_BUFFER_SIZE);
			AA_Handle->state = AA_READ;
		}
		break;
	case AA_WRITE:
		if (USBH_LL_GetURBState(phost, AA_Handle->InPipe) == USBH_URB_DONE) {
			if (USBH_BulkSendData(phost, rx_txBuffer, USB_RX_TX_BUFFER_SIZE, AA_Handle->OutPipe, 1U) != USBH_OK) {
				printf("Write error");
			} else {
				AA_Handle->state = AA_WRITE_WAIT;
			}
		}
		break;
	case AA_WRITE_WAIT:
		URB_Status = USBH_LL_GetURBState(phost, AA_Handle->OutPipe);
		if (URB_Status == USBH_URB_DONE) {
			AA_Handle->state = AA_READ;
		} else if(URB_Status == USBH_URB_NOTREADY) {
			AA_Handle->state = AA_WRITE;
		}
		break;
	default:
		break;
	}
	return USBH_OK;
}


static USBH_StatusTypeDef USBH_AA_SOFProcess(USBH_HandleTypeDef *phost) {

	return USBH_OK;
}

