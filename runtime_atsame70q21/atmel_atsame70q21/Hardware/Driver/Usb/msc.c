/** 
* @file  msc.c 
* @brief   usb mscģʽ����. 
* @details  usb mscģʽ��ʼ���������շ��ӿ�. 
* @author   zbb 
* @date      2016-7-28
* @version  FM_IFC_3.0_Plane
* @par Copyright (c):  
*       ���ڷ�������˿Ƽ����޹�˾��������
*/ 
#include "msc.h"
#include "diskio.h"
#include "string.h"

extern const USBDDriverDescriptors msdDriverDescriptors; ///<MSD Driver Descriptors List

static const Pin pinsSd[] = {BOARD_MCI_PINS_SLOTA, BOARD_MCI_PIN_CK};///<SD card pins instance.

static const Pin pinsCd[] = {BOARD_MCI_PIN_CD}; ///<SD card detection pin instance

//sMedia medias[MAX_LUNS];///<Available media.

/** DMA driver instance */
extern sXdmad dmaDrv;//from uart.c
extern sMcid mciDrv[BOARD_NUM_MCI];
extern sMedia medias[2];

MSDLun luns[MAX_LUNS];///<Device LUNs.

//COMPILER_ALIGNED(32)  sSdCard sdDrv[BOARD_NUM_MCI];///<SDCard driver instance.
extern  sSdCard sdDrv[BOARD_NUM_MCI];
//COMPILER_ALIGNED(32)  sMcid mciDrv[BOARD_NUM_MCI];///<MCI driver instance

COMPILER_ALIGNED(32)  uint8_t mSdBuffer[MSD_BUFFER_SIZE];///<LUN read/write buffer

uint32_t msdWriteTotal = 0;///<Total data write to disk

uint8_t  msdRefresh = 0;///<Delay TO event

/*----------------------------------------------------------------------------
 *       heaps
 *----------------------------------------------------------------------------*/
void msc_heap_init(void)
{
	memset(luns,0,sizeof(luns));
	memset(mSdBuffer,0,sizeof(mSdBuffer));
	msdWriteTotal = 0;
	msdRefresh = 0;
}
/*----------------------------------------------------------------------------
 *        Callbacks
 *----------------------------------------------------------------------------*/
/**
 * funcation  MSDCallbacks_Data
 * @brief Invoked when the MSD finish a READ/WRITE.
 * \param flowDirection 1 - device to host (READ10)
 *                      0 - host to device (WRITE10)
 * @param dataLength Length of data transferred in bytes.
 * @param fifoNullCount Times that FIFO is NULL to wait
 * @param fifoFullCount Times that FIFO is filled to wait
 * @return ��
 */
static void MSDCallbacks_Data(uint8_t flowDirection,
							  uint32_t  dataLength,
							  uint32_t  fifoNullCount,
							  uint32_t  fifoFullCount)
{
	fifoNullCount = fifoNullCount; /* dummy */
	fifoFullCount = fifoFullCount;  /*dummy */

	if (!flowDirection)
		msdWriteTotal += dataLength;
}
/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
/**
 * funcation CardDetectConfigure
 * @brief Configure for SD detect pin
 * @param ��
 * @return ��
 */
static void CardDetectConfigure(void)
{
	PIO_Configure(pinsCd, PIO_LISTSIZE(pinsCd));
	/* No protection detect pin */
}

/**
 * funcation CardIsConnected
 * @briefCheck if the card is connected.
 * @param iMci Controller number.
 * @return 1 if card is inserted.
 */
//static uint8_t CardIsConnected(uint8_t iMci)
//{
//	return PIO_Get(&pinsCd[iMci]) ? 0 : 1;
//}

/**
 * funcation CardInit
 * @briefRun init on the inserted card
 * @param iMci Controller number.
 * @return ��
 */
static void CardInit(sSdCard *pSd)
{
	uint8_t error;
	uint8_t retry = 2;

	while (retry --) 
	{
		error = SD_Init(pSd);

		if (error == SDMMC_OK) break;
	}

	if (error) {
		TRACE_ERROR("SD/MMC card initialization failed: %d\n\r", error);
		return;
	}

	TRACE_INFO(" SD/MMC card initialization successful\n\r");

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDMMC) 
	{
		TRACE_INFO(" MEM Card OK, size: %d MB", (int)SD_GetTotalSizeKB(pSd) / 1000);
		TRACE_INFO(", %d * %dB\n\r", (int)SD_GetNumberBlocks(pSd),
				   (int)SD_GetBlockSize(pSd));
	}

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDIO) 
	{
		TRACE_ERROR("-E- IO Card Detected \n\r");
	}
}

/**
 * funcation _ConfigurePIOs
 * @brief Initialize PIOs
 * @param ��
 * @return ��
 */
static void _ConfigurePIOs(void)
{
	PIO_Configure(pinsSd, PIO_LISTSIZE(pinsSd));///<Configure SDcard pins

	CardDetectConfigure();///<Configure SD card detection
}

/**
 * funcation _ConfigureDrivers
 * @brief Initialize driver instances.
 * @param ��
 * @return ��
 */
static void _ConfigureDrivers(void)
{
	uint32_t i;
	XDMAD_Initialize(&dmaDrv, 0);///<Initialize the DMA driver


//	NVIC_ClearPendingIRQ(XDMAC_IRQn);///<Enable XDMA interrupt and give it priority over any other peripheral interrupt
//	NVIC_SetPriority(XDMAC_IRQn, XDMAC_PRI);
//	NVIC_EnableIRQ(XDMAC_IRQn);

	MCID_Init(&mciDrv[0], HSMCI, ID_HSMCI, BOARD_MCK, &dmaDrv, 0);///<Initialize the HSMCI driver

//	NVIC_ClearPendingIRQ(HSMCI_IRQn);///<Enable MCI interrupt and give it priority lower than DMA
//	NVIC_SetPriority(HSMCI_IRQn, HSMCI_PRI);
//	NVIC_EnableIRQ(HSMCI_IRQn);

	for (i = 0; i < BOARD_NUM_MCI; i ++)
	{
		SDD_InitializeSdmmcMode(&sdDrv[i], &mciDrv[i], 0);///<Initialize SD driver
	}
}

extern uint8_t usb_mode;
/**
 * funcation SDDiskInit
 * @brief Initialize SD driver 
 * @param pSd sd�豸ָ��
 * @return ��
 */
static void SDDiskInit(sSdCard *pSd)
{
	uint8_t sdConnected=0;
	pSd = &sdDrv[0];
	sdConnected = 0;
	if (sdConnected == 0) 
	{
		sdConnected = 1;
	//	printf("-I- connect to solt n\r");
		CardInit(pSd);

		SD_DumpCID(pSd->CID);
		SD_DumpCSD(pSd->CSD);
		SD_DumpExtCSD(pSd->EXT);
		MEDSdusb_Initialize(&medias[DRV_SDMMC], pSd);
	}
	LUN_Init(&(luns[DRV_SDMMC]),
			 &(medias[DRV_SDMMC]),
			 mSdBuffer, MSD_BUFFER_SIZE,
			 0, 0, 0, 0,
			 MSDCallbacks_Data);
 if(usb_mode == DEV_MSC_MODE)
 {
	LUN_Init(&(luns[1]),
			 &(medias[1]),
			 mSdBuffer, MSD_BUFFER_SIZE,
			 0, 0, 0, 0,
			 MSDCallbacks_Data);
 }
}



/**
 * funcation _MemoriesInitialize
 * @brief Initialize MSD Media & LUNs
 * @param pSd sd�豸ָ��
 * @return  ��
 */
static void _MemoriesInitialize(sSdCard *pSd)
{
	uint32_t i;

	for (i = 0; i < MAX_LUNS; i ++)
	{
		LUN_Init(&luns[i], 0, 0, 0, 0, 0, 0, 0, 0);///<Reset all LUNs
    }
	SDDiskInit(pSd);///<Initialize SD Card
	gNbMedias = 2;
}
/**
 * funcation _MemoriesInitialize
 * @brief Configure USBHS settings for USB device��UTMI parallel mode, High/Full/Low Speed
 * @param ��
 * @return  ��
 */
static void _ConfigureUotghs(void)
{
	PMC->PMC_SCDR = PMC_SCDR_USBCLK;///<UUSBCK not used in this configuration (High Speed)
	PMC->PMC_USB = PMC_USB_USBS;///<USB clock register: USB Clock Input is UTMI PLL

	PMC_EnablePeripheral(ID_USBHS);///<Enable peripheral clock for USBHS
	USBHS->USBHS_CTRL = USBHS_CTRL_UIMOD_DEVICE;

	PMC->CKGR_UCKR = CKGR_UCKR_UPLLEN | CKGR_UCKR_UPLLCOUNT(0xF);///<Enable PLL 480 MHz


	while (!(PMC->PMC_SR & PMC_SR_LOCKU));///<Wait that PLL is considered locked by the PMC
  
	NVIC_SetPriority(USBHS_IRQn, USBHS_PRI);
	NVIC_EnableIRQ(USBHS_IRQn);
}
/**
 * funcation msc_init
 * @brief Initialize usb msc mode
 * @param ��
 * @return  ��
 */
void msc_init(void)
{
	sSdCard *pSd = 0;
	_ConfigureUotghs();///<Initialize all USB power (off)

	_ConfigurePIOs();///<Initialize PIO pins

	_ConfigureDrivers();///<Initialize drivers 

	_MemoriesInitialize(pSd);
usb_mode = DEV_MSC_MODE;
	if(usb_mode == DEV_MSC_MODE)
 {
	MSDDriver_Initialize(&msdDriverDescriptors, luns, MAX_LUNS);///<BOT driver initialization
 }
 else
 {
	MSDDriver_Initialize(&msdDriverDescriptors, luns, 1);///<BOT driver initialization
 }
	NVIC_SetPriority(USBHS_IRQn ,5);
	USBD_Connect();///<connect if needed 
	while(1)
	{
		if (USBD_GetState() < USBD_STATE_CONFIGURED) ///<Mass storage state machine
		{
		
		}
		else 
			
		{
			MSDDriver_StateMachine();

			if (msdRefresh) 
			{
				msdRefresh = 0;

				if (msdWriteTotal < 50 * 1000) 
				{
					///< Flush Disk Media 
				}

				msdWriteTotal = 0;
			}
		}
	}
}



