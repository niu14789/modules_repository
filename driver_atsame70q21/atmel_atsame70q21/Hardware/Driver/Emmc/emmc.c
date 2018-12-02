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

/**
 *  \page hsmci_multimedia_card Basic MultiMediaCard Example
 *
 *  \section Purpose
 *
 *  The Basic MultiMediaCard Example will help you to get familiar with HSMCI
 *  interface on SAM Microcontrollers. It can also help you to get familiar
 *  with the SD and MMC operation flow which can be used for fast implementation
 *  of your own SD/MMC drivers and other applications related.
 *
 *  \section Requirements
 *
 *  This package can be used with SAMV71 Xplained Ultra board or SAME70 Xplained board.
 *
 *  \section Description
 *
 *  Open HyperTerminal before running this program, the HyperTerminal will
 *  give out the test hints, you can run different tests on a inserted card.
 *
 *  \section Usage
 *
 * The MultiMedia Card Example offers a set of functions to perform
 *  MultiMedia Card tests:
 *  -# Dump MultiMedia Card information
 *  -# Test all blocks on MultiMedia Card
 *  -# Test R/W Speed (performance) of the MultiMedia Card
 *  You can find following information depends on your needs:
 *  - Usage of auto detection of sdcard insert and sdcard write-protection
 *  - (HS)MCI interface initialize sequence and interrupt installation
 *  - SD/MMC card driver implementation based on (HS)MCI interface
 *  - SD card physical layer initialize sequence implementation
 *  - MMC card physical layer initialize sequence implementation
 *  - Sample usage of SD/MMC card write and read
 *
 *  -# Build the program and download it inside the board.
 *     Please refer to the Getting Started with SAM V71/E70 Microcontrollers.pdf
 *  -# On the computer, open and configure a terminal application
 *     (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *    - 115200 bauds
 *    - 8 bits of data
 *    - No parity
 *    - 1 stop bit
 *    - No flow control
 *  -# Start the application
 *  -# In HyperTerminal, it will show something like on start up
 *      \code
 *      -- MultiMedia Card Example xxx --
 *      -- SAMxxxxx-xx
 *      -- Compiled: xxx xx xxxx xx:xx:xx --
 *      -I- Cannot check if SD card is write-protected
 *
 *      ==========================================
 *      -I- Card Type 1, CSD_STRUCTURE 0
 *      -I- SD 4-BITS BUS
 *      -I- CMD6(1) arg 0x80FFFF01
 *      -I- SD HS Not Supported
 *      -I- SD/MMC TRANS SPEED 25000 KBit/s
 *      -I- SD/MMC card initialization successful
 *      -I- Card size: 483 MB, 990976 * 512B
 *      ...
 *      \endcode
 *  -# Test function menu is like this
 *      \code
 *      # i,I   : Re-initialize card
 *      # t     : Disk R/W/Verify test
 *      # T     : Disk performance test
 *      # p     : Change number of blocks in one access for test
 *      \endcode
 *
 *  \par See Also
 *  - \ref hsmci_sdcard : Another Simple Example for SD/MMC access.
 *  - \ref sdmmc_lib : SD/MMC card driver with mci-interface.
 *  - \ref hsmci_module : sdcard physical layer driver with hsmci-interface.
 *
 *  \section References
 *  - hsmci_multimedia_card/main.c
 *  - hsmci.h
 *  - pio.h
 */

/**
 *  \file
 *
 * This file contains all the specific code for the hsmci_multimedia_card example.
 *
 *  \section Purpose
 *
 *  \section Contents
 *  The hsmci_multimedia_card application can be roughly broken down as follows:
 *     - Optional functions for detection (card insert, card protect)
 *        - CardDetectConfigure(), CardIsConnected()
 *        - CardIsProtected()
 *     - Interrupt handlers
 *        - MCI_IrqHandler()
 *     - The main function, which implements the program behaviour
 *        - I/O configuration
 *        - SD/MMC card auto-detect write-protected-check (if supported)
 *        - Initialize MCI interface and installing an isr relating to MCI
 *        - Initialize sdcard, get necessary sdcard's parameters
 *        - write/read sdcard
 */

/*----------------------------------------------------------------------------
 *         Headers
 *----------------------------------------------------------------------------*/

#include "board.h"
#include "emmc.h"
#include "libsdmmc.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DEBUG_PRINTF(...) 
/*----------------------------------------------------------------------------
 *         Local definitions
 *----------------------------------------------------------------------------*/

/** Maximum number of blocks read once (for performance test) */
#define NB_MULTI_BLOCKS     64//8192

/** Split R/W to 2, first R/W 4 blocks then remaining */
#define NB_SPLIT_MULTI      4

/** Test settings: start block address (0) */
#define TEST_BLOCK_START    (0)

/** Test settings: end block address (total SD/MMC) */
#define TEST_BLOCK_END      SD_GetNumberBlocks(&sdDrv[bMciID])

/** Test settings: skip size when "skip" key pressed */
#define TEST_BLOCK_SKIP     (100 * 1024 * 2)    // 100M

/** Test settings: Number of bytes to test performance */
#define TEST_PERFORMENCT_SIZE   (4*1024*1024)

/** Test settings: The value used to generate test data */
#define TEST_FILL_VALUE_U32     (0x5A6C1439)

/** Number of errors displayed */
#define NB_ERRORS       3

/** Number of bad blocks displayed */
#define NB_BAD_BLOCK    200

/*----------------------------------------------------------------------------
 *         Local variables
 *----------------------------------------------------------------------------*/

/** DMA driver instance */
extern sXdmad dmaDrv;//from uart.c

/** MCI driver instance. */
COMPILER_ALIGNED(32)  sMcid mciDrv[BOARD_NUM_MCI];

COMPILER_ALIGNED(32)  sSdCard sdDrv[1];

/** Current selected MCI interface */
static uint8_t bMciID = 0;

/** SD card pins instance. */
static const Pin pinsSd[] = {BOARD_MCI_PINS_SLOTA, BOARD_MCI_PIN_CK};

/** SD card detection pin instance. */
static const Pin pinsCd[] = {BOARD_MCI_PIN_CD};

void emmc_file_init(void)
{
	memset(&mciDrv,0,sizeof(mciDrv));
	memset(&sdDrv,0,sizeof(sdDrv));
	bMciID = 0;
}
int emmc_callback(int type,void * data,int len)
{
	unsigned int i;
	switch(type)
	{
		case HSMCI_IRQn:

				for (i = 0; i < BOARD_NUM_MCI; i ++)
				{
					MCID_Handler(&mciDrv[i]);
				}
			break;
		default:break;
	}
	return 0;
}
/*----------------------------------------------------------------------------
 *         Local macros
 *----------------------------------------------------------------------------*/

/* Defined to test Multi-Block functions */

/** \def READ_MULTI
 *  \brief Define to test multi-read (SD_Read())
 *         or
 *         single-read is used (SD_ReadBlocks()) */
#define READ_MULTI
/** \def WRITE_MULTI
 *  \brief Define to test multi-write (SD_Write())
 *         or
 *         single-write is used (SD_WriteBlocks()) */
#define WRITE_MULTI

/** \macro SDT_ReadFun
 * Function used for SD card test reading.
 * \param pSd  Pointer to a SD card driver instance.
 * \param address  Address of the block to read.
 * \param nbBlocks Number of blocks to be read.
 * \param pData    Data buffer whose size is at least the block size.
 */
#ifdef  READ_MULTI
#define MMCT_ReadFun(pSd, blk, nbBlk, pData) \
	SD_Read(pSd, blk, pData, nbBlk, NULL, NULL)
#else
#define MMCT_ReadFun(pSd, blk, nbBlk, pData) \
	SD_ReadBlocks(pSd, blk, pData, nbBlk)
#endif

/** \macro SDT_WriteFun
 * Function used for SD card test writing.
 * \param pSd  Pointer to a SD card driver instance.
 * \param address  Address of the block to read.
 * \param nbBlocks Number of blocks to be read.
 * \param pData    Data buffer whose size is at least the block size.
 */
#ifdef  WRITE_MULTI
#define MMCT_WriteFun(pSd, blk, nbBlk, pData) \
	SD_Write(pSd, blk, pData, nbBlk, NULL, NULL)
#else
#define MMCT_WriteFun(pSd, blk, nbBlk, pData) \
	SD_WriteBlocks(pSd, blk, pData, nbBlk)
#endif

/*----------------------------------------------------------------------------
 *         Local functions
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *         Optional: SD card detection (connection, protection)
 *----------------------------------------------------------------------------*/

/**
 * Configure for SD detect pin
 */
static void CardDetectConfigure(void)
{
	PIO_Configure(pinsCd, PIO_LISTSIZE(pinsCd));
	/* No protection detect pin */
}


/**
 * Dump card registers
 * \param iMci Controller number.
 */
static void DumpCardInfo(uint8_t iMci)
{
	sSdCard *pSd = &sdDrv[iMci];

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDIO)
		SDIO_DumpCardInformation(pSd);

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDMMC) {
		SD_DumpCID(pSd->CID);
		SD_DumpCSD(pSd->CSD);

		if (SD_GetCardType(pSd) & CARD_TYPE_bmSD)
			SD_DumpSdStatus(pSd->SSR);
	}
}

/**
 * Run tests on the inserted card
 * \param iMci Controller number.
 */
static void SD_CardInit(uint8_t iMci)
{
	sSdCard *pSd = &sdDrv[iMci];
	uint8_t error;
	uint8_t retry = 2;


	while (retry --) {
		error = SD_Init(pSd);

		if (error == SDMMC_OK) 
			break;
	}

	if (error) {
		DEBUG_PRINTF("-E- SD/MMC card initialization failed: %d\n\r", error);
		return;
	}

	DEBUG_PRINTF("-I- SD/MMC card initialization successful\n\r");

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDMMC) {
		DEBUG_PRINTF("-I- MEM Card OK, size: %d MB", (int)SD_GetTotalSizeKB(pSd) / 1000);
		DEBUG_PRINTF(", %d * %dB\n\r", (int)SD_GetNumberBlocks(pSd),
			   (int)SD_GetBlockSize(pSd));
	}

	if (SD_GetCardType(pSd) & CARD_TYPE_bmSDIO)
		DEBUG_PRINTF("-I- IO Card Detected OK\n\r");

	DumpCardInfo(iMci);
}


/**
 * Initialize PIOs
 */
static void SD_ConfigurePIOs(void)
{
	/* Configure SDcard pins */
	PIO_Configure(pinsSd, PIO_LISTSIZE(pinsSd));
	/* Configure SD card detection */
	CardDetectConfigure();
}

/**
 * Initialize driver instances.
 */
static void SD_ConfigureDrivers(void)
{
	uint32_t i;
	/* Initialize the DMA driver */
	XDMAD_Initialize(&dmaDrv, 0);

//	NVIC_ClearPendingIRQ(XDMAC_IRQn);
//	NVIC_SetPriority(XDMAC_IRQn, 1);
//	NVIC_EnableIRQ(XDMAC_IRQn);

	/* Initialize the HSMCI driver */
	MCID_Init(&mciDrv[0], HSMCI, ID_HSMCI, BOARD_MCK, &dmaDrv, 0);

//	NVIC_ClearPendingIRQ(HSMCI_IRQn);
//	NVIC_SetPriority(HSMCI_IRQn, 3);
//	NVIC_EnableIRQ(HSMCI_IRQn);

	/* Initialize SD driver */
	for (i = 0; i < BOARD_NUM_MCI; i++)
		SDD_InitializeSdmmcMode(&sdDrv[i], &mciDrv[i], 0);
}

/*----------------------------------------------------------------------------
 *         Global functions
 *----------------------------------------------------------------------------*/

/**
 *  \brief hsmci_multimedia_card Application entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
char txt1[512];//写入sd卡必须是全局变量
void emmc_init(void)
{

    
	bMciID = 0;

	/* Initialize PIO pins */
	SD_ConfigurePIOs();

	/* Initialize drivers */
	SD_ConfigureDrivers();
    
    SD_CardInit(bMciID);

	/* Card insert detection loop */
	

            
//	memset(txt1,0,512);
//	for(uint8_t addr=0; addr<10; addr++)
//	{
//			for(uint16_t i=0; i<512; i++)
//			{
//					txt1[i]++;
//			}
//			SCB_CleanInvalidateDCache();
//			SD_Write(&sdDrv[bMciID], addr, (void*)txt1, 1, NULL, NULL);
//	}
//						 
//	for(uint8_t addr1=0; addr1<10; addr1++)
//	{
//			SCB_CleanInvalidateDCache();
//			SD_Read(&sdDrv[bMciID], 0, (void*)txt1, 1, NULL, NULL);
//			
//			for(uint16_t i=0; i<512; i++)
//			{
//					if(i%16==0)
//					DEBUG_PRINTF("\n");
//					DEBUG_PRINTF("%d",txt1[i]);
//					
//			}
//	}
	//while(1){};
}

