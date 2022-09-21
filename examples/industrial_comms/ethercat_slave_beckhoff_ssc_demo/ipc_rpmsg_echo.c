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
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/ipc_notify.h>
#include <drivers/ipc_rpmsg.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* This example shows message exchange between multiple cores.
 *
 * One of the core is designated as the 'main' core
 * and other cores are designated as `remote` cores.
 *
 * The main core initiates IPC with remote core's by sending it a message.
 * The remote cores echo the same message to the main core.
 *
 * The main core repeats this for gMsgEchoCount iterations.
 *
 * In each iteration of message exchange, the message value is incremented.
 *
 * When iteration count reaches gMsgEchoCount, the example is completed.
 *
 */

#if defined(SOC_AM243X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_M4FSS0_0,
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined (SOC_AM263X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM64X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
//    CSL_CORE_ID_M4FSS0_0,
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_R5FSS1_0,
    CSL_CORE_ID_R5FSS1_1,
//    CSL_CORE_ID_A53SS0_0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

#if defined(SOC_AM273X) || defined(SOC_AWR294X)
/* main core that starts the message exchange */
uint32_t gMainCoreId = CSL_CORE_ID_R5FSS0_0;
/* remote cores that echo messages from main core, make sure to NOT list main core in this list */
uint32_t gRemoteCoreId[] = {
    CSL_CORE_ID_R5FSS0_1,
    CSL_CORE_ID_C66SS0,
    CSL_CORE_ID_MAX /* this value indicates the end of the array */
};
#endif

/*
 * Remote core service end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
uint16_t gRemoteServiceEndPt = 13u;

/* maximum size that message can have in this example */
#define MAX_MSG_SIZE        (128u)

/* Main core ack reply end point
 *
 * pick any unique value on that core between 0..RPMESSAGE_MAX_LOCAL_ENDPT-1
 * the value need not be unique across cores
 */
#define MAIN_CORE_ACK_REPLY_END_PT  (12U)

/* RPMessage_Object MUST be global or static */
RPMessage_Object gAckReplyMsgObject;

void ipc_rpmsg_echo_main_core_init(void *args)
{
    RPMessage_CreateParams createParams;
    uint32_t i, numRemoteCores;
    int32_t status;

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = MAIN_CORE_ACK_REPLY_END_PT;
    status = RPMessage_construct(&gAckReplyMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);

    numRemoteCores = 0;
    for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++)
    {
        numRemoteCores++;
    }

    /* wait for all cores to be ready */
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    ClockP_usleep(500*1000); /* wait for log messages from remote cores to be flushed, otherwise this delay is not needed */

    DebugP_log("[IPC RPMSG ECHO] Message exchange started by main core !!!\r\n");

    //RPMessage_destruct(&gAckReplyMsgObject);

}

uint8_t ipc_execute_line(char *line)
{
    char msgBuf[MAX_MSG_SIZE];
    int32_t status;
    uint32_t msg, i;
    uint16_t remoteCoreId, remoteCoreEndPt, msgSize;

    memcpy(msgBuf, line, MAX_MSG_SIZE-1);
    msgBuf[MAX_MSG_SIZE-1] = 0;
    msgSize = strlen(msgBuf) + 1; /* count the terminating char as well */

    DebugP_log("[IPC RPMSG ECHO] Main Core sent %s\r\n", msgBuf);

    /* send the same messages to all cores */
    for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
    {
        status = RPMessage_send(
                msgBuf, msgSize,
                gRemoteCoreId[i], gRemoteServiceEndPt,
                RPMessage_getLocalEndPt(&gAckReplyMsgObject),
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
    }
    memset(msgBuf, 0, MAX_MSG_SIZE-1);
    /* wait for response from all cores */
    for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
    {
            /* set 'msgSize' to size of recv buffer,
            * after return `msgSize` contains actual size of valid data in recv buffer
            */
            msgSize = sizeof(msgBuf);
            status = RPMessage_recv(&gAckReplyMsgObject,
                msgBuf, &msgSize,
                &remoteCoreId, &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
            DebugP_log("[IPC RPMSG ECHO] Main Core received %s\r\n", msgBuf);

    }

}


void ipc_rpmsg_echo_main_core_start()
{
    RPMessage_CreateParams createParams;
    uint32_t msg, i, numRemoteCores;
    char msgBuf[MAX_MSG_SIZE];
    int32_t status;
    uint16_t remoteCoreId, remoteCoreEndPt, msgSize;

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = MAIN_CORE_ACK_REPLY_END_PT;
    status = RPMessage_construct(&gAckReplyMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);

    numRemoteCores = 0;
    for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++)
    {
        numRemoteCores++;
    }

    /* wait for all cores to be ready */
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    ClockP_usleep(500*1000); /* wait for log messages from remote cores to be flushed, otherwise this delay is not needed */

    DebugP_log("[IPC RPMSG ECHO] Message exchange started by main core !!!\r\n");

    while(1)
    {
        snprintf(msgBuf, MAX_MSG_SIZE-1, "%d", msg);
        msgBuf[MAX_MSG_SIZE-1] = 0;
        msgSize = strlen(msgBuf) + 1; /* count the terminating char as well */

        /* send the same messages to all cores */
        for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
        {
            status = RPMessage_send(
                msgBuf, msgSize,
                gRemoteCoreId[i], gRemoteServiceEndPt,
                RPMessage_getLocalEndPt(&gAckReplyMsgObject),
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
        }
        /* wait for response from all cores */
        for(i=0; gRemoteCoreId[i]!=CSL_CORE_ID_MAX; i++ )
        {
            /* set 'msgSize' to size of recv buffer,
            * after return `msgSize` contains actual size of valid data in recv buffer
            */
            msgSize = sizeof(msgBuf);
            status = RPMessage_recv(&gAckReplyMsgObject,
                msgBuf, &msgSize,
                &remoteCoreId, &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);
            DebugP_assert(status==SystemP_SUCCESS);
        }
    }

    RPMessage_destruct(&gAckReplyMsgObject);

}

/* RPMessage_Object MUST be global or static */
static RPMessage_Object gRecvMsgObject;

void ipc_rpmsg_echo_remote_core_start()
{
    int32_t status;
    RPMessage_CreateParams createParams;
    char recvMsg[MAX_MSG_SIZE];
    uint16_t recvMsgSize, remoteCoreId, remoteCoreEndPt;

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = gRemoteServiceEndPt;
    status = RPMessage_construct(&gRecvMsgObject, &createParams);
    DebugP_assert(status==SystemP_SUCCESS);

    /* wait for all cores to be ready */
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    DebugP_log("[IPC RPMSG ECHO] Remote Core waiting for messages from main core ... !!!\r\n");

    /* wait for messages forever in a loop */
    while(1)
    {
        /* set 'recvMsgSize' to size of recv buffer,
        * after return `recvMsgSize` contains actual size of valid data in recv buffer
        */
        recvMsgSize = sizeof(recvMsg);
        status = RPMessage_recv(&gRecvMsgObject,
            recvMsg, &recvMsgSize,
            &remoteCoreId, &remoteCoreEndPt,
            SystemP_WAIT_FOREVER);
        DebugP_assert(status==SystemP_SUCCESS);

#if 0
        if(recvMsg[0]==0x35)
            GPIO_pinWriteHigh(gpioBaseAddr, pinNum);
        else
            GPIO_pinWriteLow(gpioBaseAddr, pinNum);
#endif

        /* echo the same message string as reply */

        /* send ack to sender CPU at the sender end point */
        status = RPMessage_send(
            recvMsg, recvMsgSize,
            remoteCoreId, remoteCoreEndPt,
            RPMessage_getLocalEndPt(&gRecvMsgObject),
            SystemP_WAIT_FOREVER);
        DebugP_assert(status==SystemP_SUCCESS);
    }
    /* This loop will never exit */
}

void ipc_rpmsg_echo_main(void *args)
{
    //Drivers_open();
    //Board_driversOpen();

    if(IpcNotify_getSelfCoreId()==gMainCoreId)
    {
        ipc_rpmsg_echo_main_core_start();
    }
    else
    {
	Drivers_open();
	Board_driversOpen();

        ipc_rpmsg_echo_remote_core_start();
    }

    Board_driversClose();
    /* We dont close drivers to let the UART driver remain open and flush any pending messages to console */
    /* Drivers_close(); */
}
