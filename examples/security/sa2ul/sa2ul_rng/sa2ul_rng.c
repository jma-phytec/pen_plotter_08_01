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

/* This example demonstrates the SA2UL Random number generation  */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* SA2UL RNG Output words length */
#define APP_SA2ULRNG_OUTPUT_WORDS_LENGTH    (4U)

void sa2ul_rng(void *args)
{
    Drivers_open();
    Board_driversOpen();
    SA2UL_Handle     handle   = NULL;
    uint32_t num_words = APP_SA2ULRNG_OUTPUT_WORDS_LENGTH, out[APP_SA2ULRNG_OUTPUT_WORDS_LENGTH], status = NULL;

    DebugP_log("[SA2UL] Sa2ul Rng HW example started ...\r\n");

    /* Open Sa2ul instance */
    SA2UL_Params params;
    params.reserved = NULL;

    handle = SA2UL_open(0, &params);
    DebugP_assert(handle != NULL);

    status = SA2UL_rngSetup(handle);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SA2UL_rngRead(handle, out);
    DebugP_assert(SystemP_SUCCESS == status);
    for(int i = 0; i<num_words; i++)
    {
        DebugP_log("[SA2UL] Sa2ul Rng output word %d -- 0x%X\r\n",i+1,out[i]);
    }

    status = SA2UL_rngRead(handle, out);
    DebugP_assert(SystemP_SUCCESS == status);
    for(int i = 0; i<num_words; i++)
    {
        DebugP_log("[SA2UL] Sa2ul Rng output word %d -- 0x%X\r\n",i+1,out[i]);
    }

    status = SA2UL_rngRead(handle, out);
    DebugP_assert(SystemP_SUCCESS == status);
    for(int i = 0; i<num_words; i++)
    {
        DebugP_log("[SA2UL] Sa2ul Rng output word %d -- 0x%X\r\n",i+1,out[i]);
    }

    status = SA2UL_rngRead(handle, out);
    DebugP_assert(SystemP_SUCCESS == status);
    for(int i = 0; i<num_words; i++)
    {
        DebugP_log("[SA2UL] Sa2ul Rng output word %d -- 0x%X\r\n",i+1,out[i]);
    }

    status = SA2UL_rngRead(handle, out);
    DebugP_assert(SystemP_SUCCESS == status);
    for(int i = 0; i<num_words; i++)
    {
        DebugP_log("[SA2UL] Sa2ul Rng output word %d -- 0x%X\r\n",i+1,out[i]);
    }
    
    SA2UL_close(handle);

    DebugP_log("[SA2UL] Sa2ul Rng HW example completed!!\r\n");
    DebugP_log("All tests have passed!!\r\n");

    Board_driversClose();
    Drivers_close();

    return;
}
