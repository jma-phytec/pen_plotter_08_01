/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <kernel/dpl/DebugP.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <drivers/ospi/ospi_sfdp.h>
#include <string.h>

/* Cypress flashes have hybrid sector configuration, this puts the first 256KB of the flash in
   hybrid sector mode. This will make block erases to first 256 KB fail. Not to lose generality,
   choosing the offset to be at 512 KB */
#define APP_OSPI_FLASH_OFFSET  (512*1024U)

#define APP_OSPI_DATA_SIZE (256)
uint8_t gOspiTxBuf[APP_OSPI_DATA_SIZE];
/* read buffer MUST be cache line aligned when using DMA, we aligned to 128B though 32B is enough */
uint8_t gOspiRxBuf[APP_OSPI_DATA_SIZE] __attribute__((aligned(128U)));

OSPI_SfdpHeader gSfdpHeader;
OSPI_SfdpParamHeader gParamHeaders[OSPI_SFDP_NPH_MAX];
OSPI_SfdpParamHeader *gBfptHeader;

OSPI_SfdpBasicFlashParamTable gBfpt;
OSPI_SfdpSectorMapParamTable gSmpt;
OSPI_SfdpSCCRParamTable gSccr;
OSPI_SfdpXspiProfile1ParamTable gXpt1;
OSPI_Sfdp4ByteAddressingParamTable g4bait;

OSPI_GenericXspiDevDefines gXspiDevDefines;

void ospi_flash_diag_test_fill_buffers();
int32_t ospi_flash_diag_test_compare_buffers();
int32_t ospi_flash_diag_print_sfdp(OSPI_Handle handle);
void ospi_flash_diag_print_defines(OSPI_GenericXspiDevDefines *xspiDefines);

void ospi_flash_diag_main(void *args)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t offset;
    uint32_t manfId, deviceId;

    /* Open OSPI Driver, among others */
    Drivers_open();

    DebugP_log("[OSPI Flash Diagnostic Test] Starting ...\r\n");

    OSPI_Handle ospiHandle = OSPI_getHandle(CONFIG_OSPI0);

    /* Zero init the dev defines struct */
    memset(&gXspiDevDefines, 0, sizeof(gXspiDevDefines));
    
    /* Initialize the flash device in 1s1s1s mode */
    OSPI_norFlashInit1s1s1s(ospiHandle);

    /* Read ID */
    status = OSPI_norFlashReadId(ospiHandle, &manfId, &deviceId);

    if(SystemP_SUCCESS == status)
    {
        DebugP_log("[OSPI Flash Diagnostic Test] Flash Manufacturer ID : 0x%X\r\n", manfId);
        DebugP_log("[OSPI Flash Diagnostic Test] Flash Device ID       : 0x%X\r\n", deviceId);

        gXspiDevDefines.XSPI_NOR_MANF_ID = manfId;
        gXspiDevDefines.XSPI_NOR_DEVICE_ID = deviceId;
    }

    /* Fill buffers with known data, 
     * find block number from offset, 
     * erase block, write the data, read back from a specific offset
     * and finally compare the results.
     */
    offset = APP_OSPI_FLASH_OFFSET;

    if( SystemP_SUCCESS == status)
    {
        ospi_flash_diag_test_fill_buffers();

        uint32_t offset  = APP_OSPI_FLASH_OFFSET;

        DebugP_log("[OSPI Flash Diagnostic Test] Executing Flash Erase on first block...\r\n");
        OSPI_norFlashErase(ospiHandle, offset);
        DebugP_log("[OSPI Flash Diagnostic Test] Done !!!\r\n");
        DebugP_log("[OSPI Flash Diagnostic Test] Performing Write-Read Test...\r\n");
        OSPI_norFlashWrite(ospiHandle, offset, gOspiTxBuf, APP_OSPI_DATA_SIZE);
        OSPI_norFlashRead(ospiHandle, offset, gOspiRxBuf, APP_OSPI_DATA_SIZE);

        status |= ospi_flash_diag_test_compare_buffers();

        if(SystemP_SUCCESS == status)
        {
            DebugP_log("[OSPI Flash Diagnostic Test] Write-Read Test Passed!\r\n");
        }
    }

    if(SystemP_SUCCESS == status)
    {
        /* Read the SFDP table and print flash details */
        status = ospi_flash_diag_print_sfdp(ospiHandle);
    }

    if(SystemP_SUCCESS == status)
    {
        DebugP_log("All tests have passed!!\r\n");
    }
    else
    {
        DebugP_log("Some tests have failed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();
}

void ospi_flash_diag_test_fill_buffers()
{
    uint32_t i;

    for(i = 0U; i < APP_OSPI_DATA_SIZE; i++)
    {
        gOspiTxBuf[i] = i;
        gOspiRxBuf[i] = 0U;
    }
}

int32_t ospi_flash_diag_test_compare_buffers()
{
    int32_t status = SystemP_SUCCESS;
    uint32_t i;

    for(i = 0U; i < APP_OSPI_DATA_SIZE; i++)
    {
        if(gOspiTxBuf[i] != gOspiRxBuf[i])
        {
            status = SystemP_FAILURE;
            DebugP_logError("OSPI read data mismatch !!!\r\n");
            break;    
        }
    }
    return status;
}

int32_t ospi_flash_diag_print_sfdp(OSPI_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t ptp = 0xFFFFFFFFU;

    /* Read the SFDP header */
    status = OSPI_norFlashReadSfdp(handle, OSPI_SFDP_HEADER_START_OFFSET, (void *)&gSfdpHeader, sizeof(OSPI_SfdpHeader));

    gBfptHeader = &gSfdpHeader.firstParamHeader;

    /* Check if the signature is read correctly */
    if(gSfdpHeader.sfdpHeader.signature != OSPI_SFDP_SIGNATURE)
    {
        DebugP_log("[OSPI Flash Diagnostic Test] Error in reading SFDP Table or SFDP not supported by Flash !!!\r\n");
    }
    else
    {
        /* Print SFDP basic information */
        DebugP_log("[OSPI Flash Diagnostic Test] SFDP Information : \r\n");
        DebugP_log("================================================\r\n");
        DebugP_log("                      SFDP                      \r\n");
        DebugP_log("================================================\r\n");

        DebugP_log("SFDP Major Revision                       : 0x%X\r\n", gSfdpHeader.sfdpHeader.majorRev);
        DebugP_log("SFDP Minor Revision                       : 0x%X\r\n", gSfdpHeader.sfdpHeader.minorRev);
        DebugP_log("Number of Parameter Headers in this Table : %u\r\n\r\n", gSfdpHeader.sfdpHeader.numParamHeaders + 1);

        /* First parameter header is already read, read the rest of parameter headers if they exist */
        uint32_t nph = gSfdpHeader.sfdpHeader.numParamHeaders;
        uint32_t i;

        if(nph > 0)
        {
            status = OSPI_norFlashReadSfdp(handle, OSPI_SFDP_SECOND_PARAM_HEADER_OFFSET, (void *)&gParamHeaders, nph * sizeof(OSPI_SfdpParamHeader));
        }

        if(status == SystemP_SUCCESS)
        {
            /* Print the types of parameter tables present */
            DebugP_log("Types of Additional Parameter Tables in this flash\r\n");
            DebugP_log("---------------------------------------------------\r\n");

            for(i = 0; i < nph; i++)
            {
                OSPI_SfdpParamHeader *paramHeader = &gParamHeaders[i];

                uint32_t paramID = (uint32_t)((uint32_t)(paramHeader->paramIdMsb << 8U) | (uint32_t)(paramHeader->paramIdLsb));

                if(paramID != OSPI_SFDP_BASIC_PARAM_TABLE_ID)
                {
                    char *paramName = OSPI_Sfdp_getParameterTableName(paramID);

                    if(paramName == NULL)
                    {
                        DebugP_log("Unsupported Parameter Table type!!! - 0x%X\r\n", paramID);    
                    }
                    else
                    {
                        DebugP_log("%s\r\n", paramName);
                    }
                }
                else
                {
                    /* Update the gBfpt pointer to the latest version */
                    if((paramHeader->paramTableMajorRev == OSPI_SFDP_JESD216_MAJOR) && 
                        ((paramHeader->paramTableMinorRev > gBfptHeader->paramTableMinorRev) || 
                            ((paramHeader->paramTableMinorRev > gBfptHeader->paramTableMinorRev) &&
                                (paramHeader->paramTableLength > gBfptHeader->paramTableLength))))
                    {
                        gBfptHeader = paramHeader;
                    }
                }
            }
        }

        /* Read the Basic Flash Parameter Table (BFPT) */
        ptp = OSPI_Sfdp_getPtp(gBfptHeader);

        status = OSPI_norFlashReadSfdp(handle, ptp, (void *)&gBfpt, gBfptHeader->paramTableLength * sizeof(uint32_t));

        /* Parse BFPT */
        if(status == SystemP_SUCCESS)
        {
            status = OSPI_Sfdp_parseBfpt(&gBfpt, &gXspiDevDefines, gBfptHeader->paramTableLength);
        }

        /* Parse other parameter tables */
        for(i = 0; i < nph; i++)
        {
            OSPI_SfdpParamHeader *paramHeader = &gParamHeaders[i];

            uint32_t paramID = (uint32_t)((uint32_t)(paramHeader->paramIdMsb << 8U) | (uint32_t)(paramHeader->paramIdLsb));
            
            ptp = OSPI_Sfdp_getPtp(paramHeader);

            switch(paramID)
            {
                case OSPI_SFDP_XSPI_PROFILE_TABLE_ID:
                    status = OSPI_norFlashReadSfdp(handle, ptp, (void *)&gXpt1, paramHeader->paramTableLength * sizeof(uint32_t));
                    status = OSPI_Sfdp_parseXpt1(&gXpt1, &gXspiDevDefines, paramHeader->paramTableLength);
                    break;

                case OSPI_SFDP_4BYTE_ADDR_INSTR_TABLE_ID:
                    status = OSPI_norFlashReadSfdp(handle, ptp, (void *)&g4bait, paramHeader->paramTableLength * sizeof(uint32_t));
                    status = OSPI_Sfdp_parse4bait(&g4bait, &gXspiDevDefines, paramHeader->paramTableLength);
                    break;

                case OSPI_SFDP_SECTOR_MAP_TABLE_ID:
                    status = OSPI_norFlashReadSfdp(handle, ptp, (void *)&gSmpt, paramHeader->paramTableLength * sizeof(uint32_t));
                    status = OSPI_Sfdp_parseSmpt(&gSmpt, &gXspiDevDefines, paramHeader->paramTableLength);
                    break;

                case OSPI_SFDP_SCCR_TABLE_ID:
                    status = OSPI_norFlashReadSfdp(handle, ptp, (void *)&gSccr, paramHeader->paramTableLength * sizeof(uint32_t));
                    status = OSPI_Sfdp_parseSccr(&gSccr, &gXspiDevDefines, paramHeader->paramTableLength);
                    break;

                default:
                    /* Parsing not yet supported */
                    DebugP_log("\r\n");
                    DebugP_log("Parsing of %s table not yet supported. \r\n", OSPI_Sfdp_getParameterTableName(paramID));
                    break;
            }
        }

        /* Print the final config */
        if(status == SystemP_SUCCESS)
        {
            ospi_flash_diag_print_defines(&gXspiDevDefines);
        }
    }

    return status;
}

void ospi_flash_diag_print_defines(OSPI_GenericXspiDevDefines *xspiDefines)
{
    if(xspiDefines != NULL)
    {
        DebugP_log("\r\n");

        DebugP_log("Flash_NorXspiDevDefines gFlashNorXspiDeviceDefines_<part-number> = {\r\n");
        DebugP_log("\r\n");
        DebugP_log("    .XSPI_NOR_CMD_RSTEN = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_RSTEN);
        DebugP_log("    .XSPI_NOR_CMD_RSTMEM = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_RSTMEM);
        DebugP_log("    .XSPI_NOR_CMD_WREN = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_WREN);
        DebugP_log("    .XSPI_NOR_CMD_WRREG = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_WRREG);
        DebugP_log("    .XSPI_NOR_CMD_BULK_ERASE = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_BULK_ERASE);
        DebugP_log("    .XSPI_NOR_CMD_SECTOR_ERASE_3B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_3B);
        DebugP_log("    .XSPI_NOR_CMD_SECTOR_ERASE_4B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_4B);
        DebugP_log("    .XSPI_NOR_CMD_BLOCK_ERASE_3B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_3B);
        DebugP_log("    .XSPI_NOR_CMD_BLOCK_ERASE_4B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_4B);
        DebugP_log("    .XSPI_NOR_CMD_PAGE_PROG_3B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_PAGE_PROG_3B);
        DebugP_log("    .XSPI_NOR_CMD_PAGE_PROG_4B = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_PAGE_PROG_4B);
        DebugP_log("    .XSPI_NOR_CMD_RDSR = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_RDSR);
        DebugP_log("    .XSPI_NOR_CMD_RDREG = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_RDREG);
        DebugP_log("    .XSPI_NOR_CMD_RDID = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_RDID);
        DebugP_log("    .XSPI_NOR_CMD_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_READ);
        DebugP_log("    .XSPI_NOR_CMD_888_SDR_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_888_SDR_READ);
        DebugP_log("    .XSPI_NOR_CMD_888_DDR_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_888_DDR_READ);
        DebugP_log("    .XSPI_NOR_CMD_444_SDR_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_444_SDR_READ);
        DebugP_log("    .XSPI_NOR_CMD_444_DDR_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_444_DDR_READ);
        DebugP_log("    .XSPI_NOR_CMD_114_READ = 0x%02X,\r\n", xspiDefines->XSPI_NOR_CMD_114_READ);
        DebugP_log("    .XSPI_NOR_SR_WIP = %u,\r\n", xspiDefines->XSPI_NOR_SR_WIP);
        DebugP_log("    .XSPI_NOR_SR_WEL = %u,\r\n", xspiDefines->XSPI_NOR_SR_WEL);
        DebugP_log("    .XSPI_NOR_RDID_NUM_BYTES = %u,\r\n", xspiDefines->XSPI_NOR_RDID_NUM_BYTES);
        DebugP_log("    .XSPI_NOR_MANF_ID = 0x%02X,\r\n", xspiDefines->XSPI_NOR_MANF_ID);
        DebugP_log("    .XSPI_NOR_DEVICE_ID = 0x%03X,\r\n", xspiDefines->XSPI_NOR_DEVICE_ID);
        DebugP_log("    .XSPI_NOR_114_READ_MODE_CLKS = %u,\r\n", xspiDefines->XSPI_NOR_114_READ_MODE_CLKS);
        DebugP_log("    .XSPI_NOR_114_READ_DUMMY_CYCLES = %u,\r\n", xspiDefines->XSPI_NOR_114_READ_DUMMY_CYCLES);
        DebugP_log("    .XSPI_NOR_444_READ_MODE_CLKS = %u,\r\n", xspiDefines->XSPI_NOR_444_READ_MODE_CLKS);
        DebugP_log("    .XSPI_NOR_444_READ_DUMMY_CYCLES = %u,\r\n", xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES);
        DebugP_log("    .XSPI_NOR_444_READ_DUMMY_CYCLES_LC = 0x%02X,\r\n", xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES_LC);
        DebugP_log("    .XSPI_NOR_QUAD_CMD_READ_DUMMY_CYCLES = 0x%02X,\r\n", xspiDefines->XSPI_NOR_QUAD_CMD_READ_DUMMY_CYCLES);
        DebugP_log("    .XSPI_NOR_OCTAL_READ_DUMMY_CYCLE = %u,\r\n", xspiDefines->XSPI_NOR_OCTAL_READ_DUMMY_CYCLE);
        DebugP_log("    .XSPI_NOR_OCTAL_READ_DUMMY_CYCLE_LC = 0x%02X,\r\n", xspiDefines->XSPI_NOR_OCTAL_READ_DUMMY_CYCLE_LC);
        DebugP_log("    .XSPI_NOR_OCTAL_DDR_RDSR_DUMMY_CYCLE = %u,\r\n", xspiDefines->XSPI_NOR_OCTAL_DDR_RDSR_DUMMY_CYCLE);
        DebugP_log("    .XSPI_NOR_OCTAL_DDR_RDREG_ADDR_BYTES = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_DDR_RDREG_ADDR_BYTES);
        DebugP_log("    .XSPI_NOR_OCTAL_DDR_WRREG_ADDR_BYTES = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_DDR_WRREG_ADDR_BYTES);
        DebugP_log("    .XSPI_NOR_OCTAL_DDR_RDVREG_DUMMY_CYCLE = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_DDR_RDVREG_DUMMY_CYCLE);
        DebugP_log("    .XSPI_NOR_OCTAL_DDR_RDNVREG_DUMMY_CYCLE = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_DDR_RDNVREG_DUMMY_CYCLE);
        DebugP_log("    .XSPI_NOR_OCTAL_RDSFDP_DUMMY_CYCLE = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_RDSFDP_DUMMY_CYCLE);
        DebugP_log("    .XSPI_NOR_OCTAL_RDSFDP_ADDR_TYPE = %u, \r\n", xspiDefines->XSPI_NOR_OCTAL_RDSFDP_ADDR_TYPE);
        DebugP_log("    .XSPI_NOR_WRR_WRITE_TIMEOUT = %u,\r\n", xspiDefines->XSPI_NOR_WRR_WRITE_TIMEOUT);
        DebugP_log("    .XSPI_NOR_BULK_ERASE_TIMEOUT = %u,\r\n", xspiDefines->XSPI_NOR_BULK_ERASE_TIMEOUT);
        DebugP_log("    .XSPI_NOR_PAGE_PROG_TIMEOUT = %u,\r\n", xspiDefines->XSPI_NOR_PAGE_PROG_TIMEOUT);
        DebugP_log("    .XSPI_NOR_VREG_OFFSET = 0x%X,\r\n", xspiDefines->XSPI_NOR_VREG_OFFSET);
        DebugP_log("    .XSPI_NOR_NVREG_OFFSET = 0x%X,\r\n", xspiDefines->XSPI_NOR_NVREG_OFFSET);
        DebugP_log("    .XSPI_NOR_QUAD_MODE_CFG_ADDR = 0x%X,\r\n", xspiDefines->XSPI_NOR_QUAD_MODE_CFG_ADDR);
        DebugP_log("    .XSPI_NOR_QUAD_MODE_CFG_BIT_LOCATION = 0x%X,\r\n", xspiDefines->XSPI_NOR_QUAD_MODE_CFG_BIT_LOCATION);
        DebugP_log("    .XSPI_NOR_DDR_OCTAL_MODE_CFG_ADDR = 0x%X,\r\n", xspiDefines->XSPI_NOR_DDR_OCTAL_MODE_CFG_ADDR);
        DebugP_log("    .XSPI_NOR_DDR_OCTAL_MODE_CFG_BIT_LOCATION = 0x%X,\r\n", xspiDefines->XSPI_NOR_DDR_OCTAL_MODE_CFG_BIT_LOCATION);
        DebugP_log("    .XSPI_NOR_DUMMY_CYCLE_CFG_ADDR = 0x%X,\r\n", xspiDefines->XSPI_NOR_DUMMY_CYCLE_CFG_ADDR);
        DebugP_log("    .XSPI_NOR_FLASH_SIZE = %u,\r\n", xspiDefines->XSPI_NOR_FLASH_SIZE);
        DebugP_log("    .XSPI_NOR_PAGE_SIZE = %u,\r\n", xspiDefines->XSPI_NOR_PAGE_SIZE);
        DebugP_log("    .XSPI_NOR_BLOCK_SIZE = %u,\r\n", xspiDefines->XSPI_NOR_BLOCK_SIZE);
        DebugP_log("    .XSPI_NOR_SECTOR_SIZE = %u,\r\n", xspiDefines->XSPI_NOR_SECTOR_SIZE);
        DebugP_log("    .addrByteSupport = %u,\r\n", xspiDefines->addrByteSupport);
        DebugP_log("    .dtrSupport = %u,\r\n", xspiDefines->addrByteSupport);
        DebugP_log("    .qeType = %u,\r\n", xspiDefines->qeType);
        DebugP_log("    .seq444Enable = { %u, %u, %u, %u, %u },\r\n", xspiDefines->seq444Enable[0], xspiDefines->seq444Enable[1], xspiDefines->seq444Enable[2], xspiDefines->seq444Enable[3], xspiDefines->seq444Enable[4] );
        DebugP_log("    .seq444Disable = { %u, %u, %u, %u },\r\n", xspiDefines->seq444Disable[0], xspiDefines->seq444Disable[1], xspiDefines->seq444Disable[2], xspiDefines->seq444Disable[3]);
        DebugP_log("    .oeType = %u,\r\n", xspiDefines->oeType);
        DebugP_log("    .cmdExtType = %u,\r\n", xspiDefines->cmdExtType);
        DebugP_log("    .byteOrder = %u,\r\n", xspiDefines->byteOrder);
        DebugP_log("};\r\n\r\n");
    }
}
