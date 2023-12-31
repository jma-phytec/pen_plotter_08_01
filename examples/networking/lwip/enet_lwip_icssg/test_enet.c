/*
 * Copyright (c) 2001,2002 Florian Schulze.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * test.c - This file is part of lwIP test
 *
 */

/* C runtime includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* lwIP core includes */
#include "lwip/opt.h"
#include "test_enet_lwip.h"

/* SDK includes */
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>

#include <enet_board_cfg.h>
#if defined(ENET_ENABLE_ICSSG)
#include <networking/enet/core/include/per/icssg.h>
#endif


/* SDK includes */
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>

extern Icssg_FwPoolMem gEnetSoc_Icssg1_1_FwPoolMem[];

Icssg_FwPoolMem* EnetCb_getFwPoolMem(Enet_Type enetType, uint32_t instId)
{
    EnetAppUtils_assert((enetType == ENET_ICSSG_DUALMAC) && (instId == 2U));
    return ((Icssg_FwPoolMem*)&gEnetSoc_Icssg1_1_FwPoolMem);
}

void print_cpu_load()
{
    static uint32_t start_time = 0;
    uint32_t print_interval_in_secs = 5;
    uint32_t cur_time = ClockP_getTimeUsec()/1000;

    if(start_time==0)
    {
        start_time = cur_time;
    }
    else
    if( (cur_time-start_time) >= (print_interval_in_secs*1000) )
    {
        uint32_t cpu_load = TaskP_loadGetTotalCpuLoad();

        DebugP_log(" %6d.%3ds : CPU load = %3d.%2d %%\r\n",
            cur_time/1000, cur_time%1000,
            cpu_load/100, cpu_load%100 );

        start_time = cur_time;

        TaskP_loadResetAll();
    }
}

void EnetApp_initLinkArgs(EnetPer_PortLinkCfg *linkArgs,
                          Enet_MacPort macPort)
{
    EnetPhy_Cfg *phyCfg = &linkArgs->phyCfg;
    EnetMacPort_LinkCfg *linkCfg = &linkArgs->linkCfg;
    EnetMacPort_Interface *mii = &linkArgs->mii;
    EnetBoard_EthPort ethPort;
    const EnetBoard_PhyCfg *boardPhyCfg;
    int32_t status;

    /* Setup board for requested Ethernet port */
    ethPort.enetType = ENET_ICSSG_DUALMAC;
    ethPort.instId   = 2;
    ethPort.macPort  = macPort;
    ethPort.boardId  = ENETBOARD_AM64X_AM243X_EVM;
    ethPort.mii.layerType      = ENET_MAC_LAYER_GMII;
    ethPort.mii.sublayerType   = ENET_MAC_SUBLAYER_REDUCED;
    ethPort.mii.variantType    = ENET_MAC_VARIANT_FORCED;

    status = EnetBoard_setupPorts(&ethPort, 1U);
    EnetAppUtils_assert(status == ENET_SOK);

    if (Enet_isCpswFamily(ethPort.enetType))
    {
        CpswMacPort_Cfg *macCfg = (CpswMacPort_Cfg *)linkArgs->macCfg;
        CpswMacPort_initCfg(macCfg);
        if (EnetMacPort_isSgmii(mii) || EnetMacPort_isQsgmii(mii))
        {
            macCfg->sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY;
        }
        else
        {
            macCfg->sgmiiMode = ENET_MAC_SGMIIMODE_INVALID;
        }
    }
#if defined(ENET_ENABLE_ICSSG)
    else
    {
        IcssgMacPort_Cfg *macCfg = (IcssgMacPort_Cfg *)linkArgs->macCfg;
        IcssgMacPort_initCfg(macCfg);
        macCfg->specialFramePrio = 1U;
    }
#endif

    boardPhyCfg = EnetBoard_getPhyCfg(&ethPort);
    if (boardPhyCfg != NULL)
    {
        EnetPhy_initCfg(phyCfg);
        phyCfg->phyAddr     = CONFIG_ENET_ICSS0_PHY1_ADDR;
        phyCfg->isStrapped  = boardPhyCfg->isStrapped;
        phyCfg->loopbackEn  = false;
        phyCfg->skipExtendedCfg = boardPhyCfg->skipExtendedCfg;
        phyCfg->extendedCfgSize = boardPhyCfg->extendedCfgSize;
        memcpy(phyCfg->extendedCfg, boardPhyCfg->extendedCfg, phyCfg->extendedCfgSize);
    }
    else
    {
        DebugP_log("No PHY configuration found for MAC port %u\r\n",
                           ENET_MACPORT_ID(ethPort.macPort));
        EnetAppUtils_assert(false);
    }

    mii->layerType     = ethPort.mii.layerType;
    mii->sublayerType  = ethPort.mii.sublayerType;
    mii->variantType   = ENET_MAC_VARIANT_FORCED;
    linkCfg->speed     = ENET_SPEED_AUTO;
    linkCfg->duplexity = ENET_DUPLEX_AUTO;

    if (Enet_isCpswFamily(ENET_ICSSG_DUALMAC))
    {
        CpswMacPort_Cfg *macCfg = (CpswMacPort_Cfg *)linkArgs->macCfg;

        if (EnetMacPort_isSgmii(mii) || EnetMacPort_isQsgmii(mii))
        {
            macCfg->sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY;
        }
        else
        {
            macCfg->sgmiiMode = ENET_MAC_SGMIIMODE_INVALID;
        }
    }
}

void EnetApp_getEnetInstInfo(Enet_Type *enetType,
                             uint32_t *instId,
                             Enet_MacPort macPortList[],
                             uint8_t *numMacPorts)
{
    *enetType = ENET_ICSSG_DUALMAC;
    *instId   = 2;
    *numMacPorts = 1;
    macPortList[0] = ENET_MAC_PORT_1;
}

int enet_lwip_example(void *args)
{
    Enet_Type enetType;
    uint32_t instId;
    Enet_MacPort macPortList[1];
    uint8_t numMacPorts;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("==========================\r\n");
    DebugP_log("      ENET LWIP App       \r\n");
    DebugP_log("==========================\r\n");


    EnetApp_getEnetInstInfo(&enetType,
                            &instId,
                             macPortList,
                             &numMacPorts);
    EnetAppUtils_enableClocks(enetType, instId);

    /* no stdio-buffering, please! */
    //setvbuf(stdout, NULL,_IONBF, 0);
    main_loop(NULL);
    return 0;
}

