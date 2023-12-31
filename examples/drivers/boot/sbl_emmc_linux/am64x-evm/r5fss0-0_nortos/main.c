/*
 *  Copyright (C) 2018-2021 Texas Instruments Incorporated
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

#include <stdlib.h>
#include <string.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <drivers/sciclient.h>
#include <drivers/bootloader.h>

/*  In this sample bootloader, we load appimages for RTOS/Baremetal and Linux at different offset
    i.e the appimage for Linux (for A53) and RTOS/Baremetal (for R5, M4) is flashed at different offset in eMMC

    Here at one eMMC offset, there is a multi-core .appimage that holds RPRC for M4 and R5
    and another .appimage that holds the linux binaries(ATF, OPTEE, A53-SPL) at another offset.

    When flashing make sure to flash images to below offset using the flash tool.

    RTOS/Baremetal appimage (for R5, M4 cores) flash at offset 0x800000 of eMMC
    Linux appimage (for A53) flash at offset 0xA00000 of eMMC
*/

/* call this API to stop the booting process and spin, do that you can connect
 * debugger, load symbols and then make the 'loop' variable as 0 to continue execution
 * with debugger connected.
 */
void loop_forever()
{
    volatile uint32_t loop = 1;
    while(loop)
        ;
}

int32_t App_loadImages(Bootloader_Handle bootHandle, Bootloader_BootImageInfo *bootImageInfo)
{
	int32_t status = SystemP_FAILURE;

    if(bootHandle != NULL)
    {
        status = Bootloader_parseMultiCoreAppImage(bootHandle, bootImageInfo);

        /* Load CPUs */
        if(status == SystemP_SUCCESS)
        {
            bootImageInfo->cpuInfo[CSL_CORE_ID_M4FSS0_0].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_M4FSS0_0);
            status = Bootloader_loadCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_M4FSS0_0]));
        }
        if(status == SystemP_SUCCESS)
        {
            bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_0].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_R5FSS1_0);
            status = Bootloader_loadCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_0]));
        }
        if(status == SystemP_SUCCESS)
        {
            bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_1].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_R5FSS1_1);
            status = Bootloader_loadCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_1]));
        }
        if(status == SystemP_SUCCESS)
        {
            /* Set clocks for self cluster */
            bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS0_0].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_R5FSS0_0);
            bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS0_1].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_R5FSS0_1);

            /* Reset self cluster, both Core0 and Core 1. Init RAMs and load the app  */
            status = Bootloader_loadSelfCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS0_0]));
            if(status == SystemP_SUCCESS)
            {
                status = Bootloader_loadSelfCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS0_1]));
            }
        }
    }

    return status;
}

int32_t App_loadLinuxImages(Bootloader_Handle bootHandle, Bootloader_BootImageInfo *bootImageInfo)
{
	int32_t status = SystemP_FAILURE;

    if(bootHandle != NULL)
    {
		status = Bootloader_parseAndLoadLinuxAppImage(bootHandle, bootImageInfo);

		if(status == SystemP_SUCCESS)
		{
			bootImageInfo->cpuInfo[CSL_CORE_ID_A53SS0_0].clkHz = Bootloader_socCpuGetClkDefault(CSL_CORE_ID_A53SS0_0);
			status = Bootloader_loadCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_A53SS0_0]));
		}
	}

	return status;
}

int32_t App_runCpus(Bootloader_Handle bootHandle, Bootloader_BootImageInfo *bootImageInfo)
{
	int32_t status = SystemP_FAILURE;

	status = Bootloader_runCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_M4FSS0_0]));

	if(status == SystemP_SUCCESS)
	{
		status = Bootloader_runCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_0]));
	}
	if(status == SystemP_SUCCESS)
	{
		status = Bootloader_runCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_R5FSS1_1]));
	}
	return status;
}

int32_t App_runLinuxCpu(Bootloader_Handle bootHandle, Bootloader_BootImageInfo *bootImageInfo)
{
	int32_t status = SystemP_FAILURE;

    /* Initialize GTC by enabling using Syscfg */

	status = Bootloader_runCpu(bootHandle, &(bootImageInfo->cpuInfo[CSL_CORE_ID_A53SS0_0]));

	return status;
}

int main()
{
    int32_t status;

    Bootloader_profileReset();

    Bootloader_socLoadSysFwLinux();
    Bootloader_profileAddProfilePoint("SYSFW Load");

    System_init();
    Bootloader_profileAddProfilePoint("System_init");

    Drivers_open();
    Bootloader_profileAddProfilePoint("Drivers_open");

    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);
    Bootloader_profileAddProfilePoint("Board_driversOpen");

    status = Sciclient_getVersionCheck(0);

    if(SystemP_SUCCESS == status)
    {
        Bootloader_BootImageInfo bootImageInfo;
		Bootloader_Params bootParams;
        Bootloader_Handle bootHandle;

		Bootloader_BootImageInfo bootImageInfoLinux;
		Bootloader_Params bootParamsLinux;
        Bootloader_Handle bootHandleLinux;

        Bootloader_Params_init(&bootParams);
		Bootloader_Params_init(&bootParamsLinux);

		Bootloader_BootImageInfo_init(&bootImageInfo);
		Bootloader_BootImageInfo_init(&bootImageInfoLinux);

        bootHandle = Bootloader_open(CONFIG_BOOTLOADER_EMMC0, &bootParams);
		bootHandleLinux = Bootloader_open(CONFIG_BOOTLOADER_EMMMC_LINUX, &bootParamsLinux);
        if(bootHandle != NULL)
        {
			status = App_loadImages(bootHandle, &bootImageInfo);
            Bootloader_profileAddProfilePoint("App_loadImages");
        }

		if(SystemP_SUCCESS == status)
		{
			if(bootHandleLinux != NULL)
			{
				status = App_loadLinuxImages(bootHandleLinux, &bootImageInfoLinux);
                Bootloader_profileAddProfilePoint("App_loadLinuxImages");
			}
		}

		if(SystemP_SUCCESS == status)
		{
			/* Print SBL log as Linux prints log to the same UART port */
			Bootloader_profilePrintProfileLog();
			DebugP_log("Image loading done, switching to application ...\r\n");
			DebugP_log("Starting linux and RTOS/Baremetal applications\r\n\n");
			UART_flushTxFifo(gUartHandle[CONFIG_UART0]);
		}

		if(SystemP_SUCCESS == status)
		{
			status = App_runLinuxCpu(bootHandleLinux, &bootImageInfoLinux);
		}

        Bootloader_close(bootHandleLinux);

        if(SystemP_SUCCESS == status)
		{
			status = App_runCpus(bootHandle, &bootImageInfo);
		}

		if(SystemP_SUCCESS == status)
		{
			status = Bootloader_runSelfCpuWithLinux();
		}

        Bootloader_close(bootHandle);
    }

    if(status != SystemP_SUCCESS )
    {
        DebugP_log("Some tests have failed!!\r\n");
    }

    Drivers_close();
    System_deinit();

    return 0;
}
