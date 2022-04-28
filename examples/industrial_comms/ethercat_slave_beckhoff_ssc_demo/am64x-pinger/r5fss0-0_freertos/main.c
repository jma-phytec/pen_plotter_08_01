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
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <drivers/pinmux.h>

#define MAIN_TASK_PRI  (configMAX_PRIORITIES-1)

#define MAIN_TASK_SIZE (16384U/sizeof(configSTACK_DEPTH_TYPE))
StackType_t gMainTaskStack[MAIN_TASK_SIZE] __attribute__((aligned(32)));

StaticTask_t gMainTaskObj;
TaskHandle_t gMainTask;

void ethercat_slave_beckhoff_ssc_demo_main(void *args);

void frertos_main(void *args)
{
    ethercat_slave_beckhoff_ssc_demo_main(NULL);

    vTaskDelete(NULL);
}

/*
 * Mux and initialize the ethernet reference clock used in PHYTEC's phyCORE-AM64x design
 *
 * sysconfig doesn't support the configuration of this pin for some reason so have to
 * manually set it up like this for now, bypassing sysconfig.
 */

//Pinger carrier board
static Pinmux_PerCfg_t My_gPinMuxMainDomainCfg[] = {
    {
        PIN_EXT_REFCLK1, ( PIN_MODE(5) | PIN_PULL_DISABLE )
    },
    {PINMUX_END, PINMUX_END}
};

void EthRefCLK_init(void)
{
    Pinmux_config(My_gPinMuxMainDomainCfg, PINMUX_DOMAIN_ID_MAIN);
}

//is this really needed? Didnt have to do this to get the same phys working on the pinger lite
// WAIT, so does this even do anything
/*
static void EnetMp_ReleasePhyResets(void) {
    uint32_t phyResetBaseAddr, pinNum;

    // enable CPSW3G OUTCLK (used by ICSSG PHYs)
    //EnetAppUtils_enableClkOut(ENET_CPSW_3G, ENETAPPUTILS_CLKOUT_FREQ_25MHZ);

    // wait for clock to come alive
    ClockP_usleep(1);
#if 0
    // release PHY1 reset
    phyResetBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_PHY1_RESET_BASE_ADDR);
    pinNum = GPIO_PHY1_RESET_PIN;
    GPIO_setDirMode(phyResetBaseAddr, pinNum, GPIO_DIRECTION_OUTPUT);
    GPIO_pinWriteHigh(phyResetBaseAddr, pinNum);

    // release PHY2 reset
    phyResetBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_PHY2_RESET_BASE_ADDR);
    pinNum = GPIO_PHY2_RESET_PIN;
    GPIO_setDirMode(phyResetBaseAddr, pinNum, GPIO_DIRECTION_OUTPUT);
    GPIO_pinWriteHigh(phyResetBaseAddr, pinNum);
#endif
}*/

// PHYTEC - Map some special address we need to enable the ethernet clock below for the phyCORE-AM64x and Pinger carrier board
#define CTRLMMR_LOCK2_KICK0 (uint32_t*)0x43009008
#define CTRLMMR_LOCK2_KICK1 (uint32_t*)0x4300900c
#define CTRLMMR_CLKOUT_CTRL (uint32_t*)0x43008010

int main()
{

    // PHYTEC - Pinger Carrier board
    /* Set clkout0 to 25MHz and enable */
    /* 25MHz clock is needed for PRU-ICSS-G0 ETH PHYs */
    *CTRLMMR_LOCK2_KICK0 = 0x68ef3490;  /* kick0 */
    *CTRLMMR_LOCK2_KICK1 = 0xd172bc5a;  /* kick1 */
    *CTRLMMR_CLKOUT_CTRL = 0x11;    /* CLK_EN = 1, CLK_SEL = 1 */

    /* init SOC specific modules */
    System_init();
    EthRefCLK_init();
    Board_init();

    /* This task is created at highest priority, it should create more tasks and then delete itself */
    gMainTask = xTaskCreateStatic( frertos_main,   /* Pointer to the function that implements the task. */
                                  "freertos_main", /* Text name for the task.  This is to facilitate debugging only. */
                                  MAIN_TASK_SIZE,  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                  NULL,            /* We are not using the task parameter. */
                                  MAIN_TASK_PRI,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                  gMainTaskStack,  /* pointer to stack base */
                                  &gMainTaskObj ); /* pointer to statically allocated task object memory */
    configASSERT(gMainTask != NULL);

    /* Start the scheduler to start the tasks executing. */
    vTaskStartScheduler();

    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return 0;
}
