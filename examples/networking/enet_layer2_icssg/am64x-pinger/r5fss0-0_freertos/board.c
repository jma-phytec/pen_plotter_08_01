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

#include <stdlib.h>
#include <drivers/hw_include/cslr_soc.h>
#include <stdbool.h>

#include <enet_board_cfg.h>
#include <networking/enet/core/include/phy/dp83867.h>

uint32_t txDelay = 1500U;
uint32_t rxDelay = 2000U;

/*
 * Board info
 */
void Board_cpswMuxSel(void)
{
    return;
}

/*
 * Tx and Rx Delay set
 */
void Board_TxRxDelaySet(const EnetBoard_PhyCfg *boardPhyCfg)
{
    Dp83867_Cfg *extendedCfg = (Dp83867_Cfg *)boardPhyCfg->extendedCfg;
    extendedCfg->txDelayInPs = txDelay;
    extendedCfg->rxDelayInPs = rxDelay;
    return;
}

/*
 * Get ethernet board id
 */
uint32_t Board_getEthBoardId(void)
{
    return ENETBOARD_AM64X_AM243X_EVM;
}

/*
 * Get ethernet type
 */
uint32_t Board_getEthType(void)
{
    return ENET_CPSW_3G;
}
