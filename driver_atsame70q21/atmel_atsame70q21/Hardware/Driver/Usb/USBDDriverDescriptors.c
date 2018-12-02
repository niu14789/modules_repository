/* ---------------------------------------------------------------------------- */
/*                  Atmel Microcontroller Software Support                      */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2015, Atmel Corporation                                        */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* Atmel's name may not be used to endorse or promote products derived from     */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR   */
/* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE   */
/* DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,      */
/* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,  */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING         */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                           */
/* ---------------------------------------------------------------------------- */

/** \file
 * Declaration of the descriptors used by the HID device Transfer driver.
 */

/** \addtogroup usbd_hid_tran
 *@{
 */

/*------------------------------------------------------------------------------
 *         Headers
 *----------------------------------------------------------------------------*/

#include "board.h"
#include "USBD_Config.h"
#include <string.h>
#include "HIDDTransferDriver.h"
#include "HIDReports.h"
#include "MSDDriver.h"
#include "MSDescriptors.h"
/*------------------------------------------------------------------------------
 *         Definitions
 *----------------------------------------------------------------------------*/

/** \addtogroup usbd_hid_trans_device_descriptor_ids HID Transfer Device
 * Descriptor IDs
 *      @{
 *
 * \section IDs
 * - HIDDTransferDriverDescriptors_PRODUCTID
 * - HIDDTransferDriverDescriptors_VENDORID
 * - HIDDTransferDriverDescriptors_RELEASE
 */
/** Product ID for the Mass Storage device driver. */
#define MSDDriverDescriptors_PRODUCTID      USBD_PID_MSD
/** Device product ID. */
#define HIDDTransferDriverDescriptors_PRODUCTID       USBD_PID_HIDTRANSFER
/** Device vendor ID. */
#define HIDDTransferDriverDescriptors_VENDORID        USBD_VID_ATMEL
/** Device release number. */
#define HIDDTransferDriverDescriptors_RELEASE         USBD_RELEASE_1_00
/**      @}*/

/*------------------------------------------------------------------------------
 *         Macros
 *----------------------------------------------------------------------------*/

/** Returns the minimum between two values. */
#define MIN(a, b)       ((a < b) ? a : b)
uint8_t usb_mode = HID_MODE;///MSC_MODE   HID_MODE
/*------------------------------------------------------------------------------
 *         Internal variables
 *----------------------------------------------------------------------------*/

/** Device descriptor. */
static  USBDeviceDescriptor deviceDescriptor = {

	sizeof(USBDeviceDescriptor),
	USBGenericDescriptor_DEVICE,
	USBDeviceDescriptor_USB2_00,
	HIDDeviceDescriptor_CLASS,
	HIDDeviceDescriptor_SUBCLASS,
	HIDDeviceDescriptor_PROTOCOL,
	CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0),
	HIDDTransferDriverDescriptors_VENDORID,
 
	//MSDDriverDescriptors_PRODUCTID,
	HIDDTransferDriverDescriptors_PRODUCTID,
	HIDDTransferDriverDescriptors_RELEASE,
	1, /* Index of manufacturer description */
	2, /* Index of product description */
	3, /* Index of serial number description */
	1  /* One possible configuration */
};
/** Device descriptor. */
 const USBDeviceDescriptor deviceDescriptor_MSC = {

	sizeof(USBDeviceDescriptor),
	USBGenericDescriptor_DEVICE,
	USBDeviceDescriptor_USB2_00,
	HIDDeviceDescriptor_CLASS,
	HIDDeviceDescriptor_SUBCLASS,
	HIDDeviceDescriptor_PROTOCOL,
	CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0),
	HIDDTransferDriverDescriptors_VENDORID,
 
	MSDDriverDescriptors_PRODUCTID,
	//HIDDTransferDriverDescriptors_PRODUCTID,
	HIDDTransferDriverDescriptors_RELEASE,
	1, /* Index of manufacturer description */
	2, /* Index of product description */
	3, /* Index of serial number description */
	1  /* One possible configuration */
};
/** 
 * function  USB_mode_switch 
 * @brief    �л�USBģʽ��HID/MSC
 * @param   ģʽ��0/1.  
 * @return  ��
 */  
void USB_mode_switch(uint8_t mode)
{
	if(mode != 0)
	{
    memset(&deviceDescriptor,0,sizeof(deviceDescriptor));
		memcpy(&deviceDescriptor,&deviceDescriptor_MSC,sizeof(deviceDescriptor));
	}
}
/** Configuration descriptor. */
 const HIDDTransferDriverConfigurationDescriptors configurationDescriptors
= {

	/* Configuration descriptor */

	{
		sizeof(USBConfigurationDescriptor),
		USBGenericDescriptor_CONFIGURATION,
		sizeof(HIDDTransferDriverConfigurationDescriptors),
		1, /* One interface in this configuration */
		1, /* This is configuration #1 */
		0, /* No associated string descriptor */
		USBD_BMATTRIBUTES,
		USBConfigurationDescriptor_POWER(100)
	},
	/* Interface descriptor */

	{
		sizeof(USBInterfaceDescriptor),
		USBGenericDescriptor_INTERFACE,
		0, /* This is interface #0 */
		0, /* This is alternate setting #0 */
		2, /* Two endpoints used */
		HIDInterfaceDescriptor_CLASS,
		HIDInterfaceDescriptor_SUBCLASS_NONE,
		HIDInterfaceDescriptor_PROTOCOL_NONE,
		0  /* No associated string descriptor */
	},
	/* HID descriptor */

	{
		sizeof(HIDDescriptor1),
		HIDGenericDescriptor_HID,
		HIDDescriptor_HID1_11,
		0, /* Device is not localized, no country code */
		1, /* One HID-specific descriptor (apart from this one) */
		HIDGenericDescriptor_REPORT,
		{HIDDTransferDriver_REPORTDESCRIPTORSIZE}
	},
	/* Interrupt IN endpoint descriptor */

	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_IN,
			HIDDTransferDriverDescriptors_INTERRUPTIN),
		USBEndpointDescriptor_INTERRUPT,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(
			HIDDTransferDriverDescriptors_INTERRUPTIN),
		MIN(USBEndpointDescriptor_MAXINTERRUPTSIZE_HS,
		HIDDTransferDriver_REPORTSIZE)),
		HIDDTransferDriverDescriptors_INTERRUPTIN_POLLING
	},
	/* Interrupt OUT endpoint descriptor */

	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_OUT,
			HIDDTransferDriverDescriptors_INTERRUPTOUT),
		USBEndpointDescriptor_INTERRUPT,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(
			HIDDTransferDriverDescriptors_INTERRUPTOUT),
		MIN(USBEndpointDescriptor_MAXINTERRUPTSIZE_HS,
		HIDDTransferDriver_REPORTSIZE)),
		HIDDTransferDriverDescriptors_INTERRUPTOUT_POLLING
	}
};
/********************************************MSC******************/
/** Device qualifier descriptor (Necessary to pass USB test). */
 const USBDeviceQualifierDescriptor qualifierDescriptor = {

	sizeof(USBDeviceQualifierDescriptor),
	USBGenericDescriptor_DEVICEQUALIFIER,
	USBDeviceDescriptor_USB2_00,
	MSDeviceDescriptor_CLASS,
	MSDeviceDescriptor_SUBCLASS,
	MSDeviceDescriptor_PROTOCOL,
	CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0),
	1, // Device has one possible configuration.
	0x00
};

/** Full-speed configuration descriptor. */
 const MSDConfigurationDescriptors configurationDescriptorsFS = {

	/* Standard configuration descriptor. */
	{
		sizeof(USBConfigurationDescriptor),
		USBGenericDescriptor_CONFIGURATION,
		sizeof(MSDConfigurationDescriptors),
		1, /* Configuration has one interface. */
		1, /* This is configuration #1. */
		0, /* No string descriptor for configuration. */
		USBD_BMATTRIBUTES,
		USBConfigurationDescriptor_POWER(100)
	},
	/* Mass Storage interface descriptor. */
	{
		sizeof(USBInterfaceDescriptor),
		USBGenericDescriptor_INTERFACE,
		0, /* This is interface #0. */
		0, /* This is alternate setting #0. */
		2, /* Interface uses two endpoints. */
		MSInterfaceDescriptor_CLASS,
		MSInterfaceDescriptor_SCSI,
		MSInterfaceDescriptor_BULKONLY,
		0 /* No string descriptor for interface. */
	},
	/* Bulk-OUT endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_OUT,
			MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_MAXBULKSIZE_FS),
		0 /* Must be 0 for full-speed Bulk endpoints. */
	},
	/* Bulk-IN endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_IN,
			MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_MAXBULKSIZE_FS),
		0 /* Must be 0 for full-speed Bulk endpoints. */

	}
};

/** Full-speed other speed configuration descriptor. */
 const MSDConfigurationDescriptors otherSpeedDescriptorsFS = {

	/* Standard configuration descriptor. */
	{
		sizeof(USBConfigurationDescriptor),
		USBGenericDescriptor_OTHERSPEEDCONFIGURATION,
		sizeof(MSDConfigurationDescriptors),
		1, /* Configuration has one interface. */
		1, /* This is configuration #1. */
		0, /* No string descriptor for configuration. */
		BOARD_USB_BMATTRIBUTES,
		USBConfigurationDescriptor_POWER(100)
	},
	/* Mass Storage interface descriptor. */
	{
		sizeof(USBInterfaceDescriptor),
		USBGenericDescriptor_INTERFACE,
		0, /* This is interface #0. */
		0, /* This is alternate setting #0. */
		2, /* Interface uses two endpoints. */
		MSInterfaceDescriptor_CLASS,
		MSInterfaceDescriptor_SCSI,
		MSInterfaceDescriptor_BULKONLY,
		0 /* No string descriptor for interface. */
	},
	/* Bulk-OUT endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_OUT,
			MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_MAXBULKSIZE_HS),
		0 /* No string descriptor for endpoint. */
	},
	/* Bulk-IN endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_IN,
			MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_MAXBULKSIZE_HS),
		0 /* No string descriptor for endpoint. */
	}
};

/** High-speed configuration descriptor. */
 const MSDConfigurationDescriptors configurationDescriptorsHS = {

	/* Standard configuration descriptor. */
	{
		sizeof(USBConfigurationDescriptor),
		USBGenericDescriptor_CONFIGURATION,
		sizeof(MSDConfigurationDescriptors),
		1, /* Configuration has one interface. */
		1, /* This is configuration #1. */
		0, /* No string descriptor for configuration. */
		BOARD_USB_BMATTRIBUTES,
		USBConfigurationDescriptor_POWER(100)
	},
	/* Mass Storage interface descriptor. */
	{
		sizeof(USBInterfaceDescriptor),
		USBGenericDescriptor_INTERFACE,
		0, /* This is interface #0. */
		0, /* This is alternate setting #0. */
		2, /* Interface uses two endpoints. */
		MSInterfaceDescriptor_CLASS,
		MSInterfaceDescriptor_SCSI,
		MSInterfaceDescriptor_BULKONLY,
		0 /* No string descriptor for interface. */
	},
	/* Bulk-OUT endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_OUT,
			MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_MAXBULKSIZE_HS),
		0 /* No string descriptor for endpoint. */
	},
	/* Bulk-IN endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_IN,
			MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_MAXBULKSIZE_HS),
		0 /* No string descriptor for endpoint. */
	}
};

/** High-speed other speed configuration descriptor. */
 const MSDConfigurationDescriptors otherSpeedDescriptorsHS = {

	/* Standard configuration descriptor. */
	{
		sizeof(USBConfigurationDescriptor),
		USBGenericDescriptor_OTHERSPEEDCONFIGURATION,
		sizeof(MSDConfigurationDescriptors),
		1, /* Configuration has one interface. */
		1, /* This is configuration #1. */
		0, /* No string descriptor for configuration. */
		BOARD_USB_BMATTRIBUTES,
		USBConfigurationDescriptor_POWER(100)
	},
	/* Mass Storage interface descriptor. */
	{
		sizeof(USBInterfaceDescriptor),
		USBGenericDescriptor_INTERFACE,
		0, /* This is interface #0. */
		0, /* This is alternate setting #0. */
		2, /* Interface uses two endpoints. */
		MSInterfaceDescriptor_CLASS,
		MSInterfaceDescriptor_SCSI,
		MSInterfaceDescriptor_BULKONLY,
		0 /* No string descriptor for interface. */
	},
	/* Bulk-OUT endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_OUT,
			MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKOUT),
		USBEndpointDescriptor_MAXBULKSIZE_FS),
		0 /* No string descriptor for endpoint. */
	},
	/* Bulk-IN endpoint descriptor */
	{
		sizeof(USBEndpointDescriptor),
		USBGenericDescriptor_ENDPOINT,
		USBEndpointDescriptor_ADDRESS(
			USBEndpointDescriptor_IN,
			MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_BULK,
		MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(MSDDriverDescriptors_BULKIN),
		USBEndpointDescriptor_MAXBULKSIZE_FS),
		0 /* No string descriptor for endpoint. */
	}
};
/*********************************************END MSC*************/
/**
 *  Variables: String descriptors
 *      languageIdDescriptor - Language ID string descriptor.
 *      manufacturerDescriptor - Manufacturer name.
 *      productDescriptor - Product name.
 *      serialNumberDescriptor - Product serial number.
 *      stringDescriptors - Array of pointers to string descriptors.
 */
 const unsigned char languageIdDescriptor[] = {

	USBStringDescriptor_LENGTH(1),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_ENGLISH_US
};

 const unsigned char manufacturerDescriptor[] = {

	USBStringDescriptor_LENGTH(5),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('A'),
	USBStringDescriptor_UNICODE('T'),
	USBStringDescriptor_UNICODE('M'),
	USBStringDescriptor_UNICODE('E'),
	USBStringDescriptor_UNICODE('L')
};

 const unsigned char productDescriptor[] = {

	USBStringDescriptor_LENGTH(22),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('A'),
	USBStringDescriptor_UNICODE('T'),
	USBStringDescriptor_UNICODE('M'),
	USBStringDescriptor_UNICODE('E'),
	USBStringDescriptor_UNICODE('L'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('A'),
	USBStringDescriptor_UNICODE('T'),
	USBStringDescriptor_UNICODE('9'),
	USBStringDescriptor_UNICODE('1'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('H'),
	USBStringDescriptor_UNICODE('I'),
	USBStringDescriptor_UNICODE('D'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('T'),
	USBStringDescriptor_UNICODE('R'),
	USBStringDescriptor_UNICODE('A'),
	USBStringDescriptor_UNICODE('N'),
	USBStringDescriptor_UNICODE('S'),
	USBStringDescriptor_UNICODE('F'),
	USBStringDescriptor_UNICODE('E'),
	USBStringDescriptor_UNICODE('R'),
};

 const unsigned char serialNumberDescriptor[] = {

	USBStringDescriptor_LENGTH(12),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('0'),
	USBStringDescriptor_UNICODE('1'),
	USBStringDescriptor_UNICODE('2'),
	USBStringDescriptor_UNICODE('3'),
	USBStringDescriptor_UNICODE('4'),
	USBStringDescriptor_UNICODE('5'),
	USBStringDescriptor_UNICODE('6'),
	USBStringDescriptor_UNICODE('7'),
	USBStringDescriptor_UNICODE('8'),
	USBStringDescriptor_UNICODE('9'),
	USBStringDescriptor_UNICODE('A'),
	USBStringDescriptor_UNICODE('F')
};

 const unsigned char *stringDescriptors[] = {

	languageIdDescriptor,
	manufacturerDescriptor,
	productDescriptor,
	serialNumberDescriptor
};

/*------------------------------------------------------------------------------
 *         Exported variables
 *----------------------------------------------------------------------------*/

/** List of descriptors used by the HID Transfer driver. */
const USBDDriverDescriptors hiddTransferDriverDescriptors = {

	&deviceDescriptor,
	(USBConfigurationDescriptor *) &configurationDescriptors,
	0, /* No full-speed device qualifier descriptor */
	0, /* No full-speed other speed configuration */
	0, /* No high-speed device descriptor */
	0, /* No high-speed configuration descriptor */
	0, /* No high-speed device qualifier descriptor */
	0, /* No high-speed other speed configuration descriptor */
	stringDescriptors,
	4 /* Four string descriptors in list */
};

/** List of the standard descriptors used by the Mass Storage driver. */
const USBDDriverDescriptors msdDriverDescriptors = {

	&deviceDescriptor,
	(USBConfigurationDescriptor *) &configurationDescriptorsFS,
	&qualifierDescriptor,
	(USBConfigurationDescriptor *) &otherSpeedDescriptorsFS,
	0, /* No high-speed device descriptor (uses FS one) */
	(USBConfigurationDescriptor *) &configurationDescriptorsHS,
	&qualifierDescriptor,
	(USBConfigurationDescriptor *) &otherSpeedDescriptorsHS,
	stringDescriptors,
	4 /* Four string descriptors in array */
};
/*-----------------------------------------------------------------------------
 *         Callback re-implementation
 *-----------------------------------------------------------------------------*/

/**
 * Invoked when a new SETUP request is received from the host. Forwards the
 * request to the Mass Storage device driver handler function.
 * \param request  Pointer to a USBGenericRequest instance.
 */
void USBDCallbacks_RequestReceived(const USBGenericRequest *request)
{
	if(usb_mode==0)
	{
	HIDDTransferDriver_RequestHandler(request);
	}
	else
	{
		MSDDriver_RequestHandler(request);
	}
}

/**
 * Invoked when the configuration of the device changes. Resets the mass
 * storage driver.
 * \param cfgnum New configuration number.
 */
void USBDDriverCallbacks_ConfigurationChanged(unsigned char cfgnum)
{
		if(usb_mode==0)
	{
		HIDDTransferDriver_ConfigurationChangedHandler(cfgnum);
	}
	else
	{
		MSDDriver_ConfigurationChangeHandler(cfgnum);
	}
}

void usbddriverdescriptors_heap(void)
{
	usb_mode = HID_MODE;
	deviceDescriptor.bLength = sizeof(USBDeviceDescriptor);
	deviceDescriptor.bDescriptorType = USBGenericDescriptor_DEVICE;
	deviceDescriptor.bcdUSB = USBDeviceDescriptor_USB2_00;
	deviceDescriptor.bDeviceClass = HIDDeviceDescriptor_CLASS;
	deviceDescriptor.bDeviceSubClass = HIDDeviceDescriptor_SUBCLASS;
	deviceDescriptor.bDeviceProtocol = HIDDeviceDescriptor_PROTOCOL;
	deviceDescriptor.bMaxPacketSize0 = 64;//CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0);
	deviceDescriptor.idVendor = HIDDTransferDriverDescriptors_VENDORID;

	//MSDDriverDescriptors_PRODUCTID,
	deviceDescriptor.idProduct = HIDDTransferDriverDescriptors_PRODUCTID;
	deviceDescriptor.bcdDevice = HIDDTransferDriverDescriptors_RELEASE;
	deviceDescriptor.iManufacturer = 1; /* Index of manufacturer description */
	deviceDescriptor.iProduct = 2; /* Index of product description */
	deviceDescriptor.iSerialNumber = 3; /* Index of serial number description */
	deviceDescriptor.bNumConfigurations = 1;  /* One possible configuration */
}


/**@}*/
