/*!
* \file IOLM_Port_smiExample.c
*
* \brief
* Example application to show how the IO-Link Master Stack can be used via the SMI.
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-10-06
*
* \copyright
* Copyright (c) 2021, KUNBUS GmbH<br /><br />
* All rights reserved.<br />
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:<br />
* <ol>
* <li>Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.</li>
* <li>Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.</li>
* <li>Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.</li>
* </ol>
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <stdint.h>
#include "IOLM_Port_smiExample.h"
#include "IOLinkPort/IOLM_Port_SMI.h"

IOLM_SMI_SCallbacks IOLM_EXMPL_SSmiCallbacks_g =
{
    /* Generic channel for SMI over UART */
    .cbGenericCnf               =       IOLM_SMI_cbGenericCnf,
    /* Master Identification */
    .cbMasterIdentificationCnf  =       IOLM_EXMPL_cbMasterIdentificationCnf,
    /* Event handling(acyclic) */ 
    .cbPortEventInd             =       IOLM_EXMPL_PortEventInd,
    .cbDeviceEventInd           =       IOLM_EXMPL_DeviceEventInd,
    /* Communication establishment */ 
    .cbPortConfigurationCnf     =       IOLM_EXMPL_cbPortConfigurationCnf,
    .cbPortStatusCnf            =       IOLM_EXMPL_cbPortStatusCnf,
    /* Acyclic communication */
    .cbDeviceWriteCnf           =       IOLM_SMI_cbDeviceWriteCnf,
    .cbDeviceReadCnf            =       IOLM_EXMPL_cbDeviceReadCnf,
    /* Cyclic communication */ 
    .cbPDInCnf                  =       IOLM_EXMPL_cbPDInCnf,
    .cbPDOutCnf                 =       IOLM_EXMPL_cbPDOutCnf,
};

IOLM_EXMPL_SPortDataValues_t port[IOLM_EXMPL_PORTS_USED + 1];

/*!
 *  \brief
 *  Initialization of example application
 *
 *  \return     void
 *
 */
void IOLM_EXMPL_init(void)
{
    uint8_t portNumber;
    uint8_t dataValueCounter;
    /* SMI Example init */
    /* Initialize state machine status array */
    /* In SMI context portNumber numbers start with 1 -> portNumber 0 will not be used */
    
    port[0].exampleState = IOLM_eExampleState_NotUsed;
 
    /* Set all active ports to init state */
    for (portNumber = 1; portNumber <= IOLM_EXMPL_PORTS_USED; portNumber++)
    {
        port[portNumber].exampleState = IOLM_eExampleState_Init; 
    } 

    /* Set all active ports to init port */
    for (portNumber = 1; portNumber <= IOLM_EXMPL_PORTS_USED; portNumber++) 
     {
        port[portNumber].currentStackPortStatus = IOLM_SMI_ePortStatus_NO_DEVICE;
        for (dataValueCounter = 0; dataValueCounter <= PD_INPUT_LENGTH; dataValueCounter++)
        {
            port[portNumber].aPDInCnfData[dataValueCounter] = 0;
        }
    }
    /* Initialze external SMI channel */
    IOLM_SMI_portInit();

    /* IO-Link Master stack init Example init */
    IOLM_SMI_vInit(&IOLM_EXMPL_SSmiCallbacks_g);
}

/*!
 *  \brief
 *  Initialization of example main loop
 *
 *  \return     void
 *
 */
void IOLM_EXMPL_mainLoop(void)
{
    IOLM_EXMPL_printf("---------------START EXAMPLE APPLICATION------------------\n");
    IOLM_SMI_vMasterIdentificationReq(IOLM_SMI_CLIENT_APP);
    OSAL_SCHED_sleep(5); /*Wait for answer*/
    IOLM_EXMPL_printf("\n");
    
    while (1)
    {
#if (IOLM_EXMPL_ENABLE_AUTO_SETUP == 1)
        IOLM_EXMPL_stateMachine();
#endif
        OSAL_SCHED_sleep(1000);
    }
}

/*!
 *  \brief
 *  Initialization of example state machine
 *
 *  \return     void
 *
 */
void IOLM_EXMPL_stateMachine(void) 
{
    uint8_t portNumber;

    for (portNumber = 1; portNumber <= IOLM_EXMPL_PORTS_USED; portNumber++)
    {
        /* STATE MACHINE ARCHITECTURE:  
            --------------------Establishment of the COMMUNICATION--------------------
                   Application                                              Stack
                   states:                                                  act as:
             1.)   IOLM_eExampleState_Init                   
             2.)   IOLM_eExampleState_Config                    ->          Receiver
             3.)   IOLM_eExampleState_ConfigWait                <-          Transmitter 
             4.)   IOLM_eExampleState_PortStatusRequest         ->          Receiver
             5.)   IOLM_eExampleState_PortStatusWait            <-          Transmitter
             ----------------------------COMMUNICATION-----------------------------
             6.)   IOLM_eExampleState_ReadVendorName            ->          Receiver
             7.)   IOLM_eExampleState_ReadVendorNameWait        <-          Transmitter
             8.)   IOLM_eExampleState_ReadProductName           ->          Receiver
             9.)   IOLM_eExampleState_ReadProductNameWait       <-          Transmitter
             10.)  IOLM_eExampleState_ReadSerialnumber          ->          Receiver
             11.)  IOLM_eExampleState_ReadSerialnumberWait      <-          Transmitter
             12.)  IOLM_eExampleState_WriteProcessDataValue     ->          Receiver
             13.)  IOLM_eExampleState_WriteProcessDataValueWait <-          Transmitter
             ----------------------------Read process data COMMUNICATION------------
             14.)  IOLM_eExampleState_PortStatusRequestPD       ->          Receiver
             15.)  IOLM_eExampleState_PortStatusWaitPD          <-          Transmitter
             16.)  IOLM_eExampleState_ReadProcessDataValue      ->          Receiver
             17.)  IOLM_eExampleState_ReadProcessDataValueWait  <-          Transmitter
             18.)  IOLM_eExampleState_CheckForOperate           ->          Receiver
             --.)  default:IOLM_eExampleState_PortStatusWait    <-          Transmitter

            Note: The state machine is not strictly executed in this specific sequence,
            see for example IOLM_EXMPL_PortErrorHandler.
        */

        switch (port[portNumber].exampleState)
        {
            case IOLM_eExampleState_Init:                                                                                   /* start initialization */
            {
                port[portNumber].exampleState = IOLM_eExampleState_Config;
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Init\" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_Config:                                                                                 /* start establishment of communication */
            {
                IOLM_SMI_SPortConfigList portConfig;
                memset(&portConfig, 0, sizeof(portConfig));
                portConfig.u8PortMode = IOLM_SMI_ePortMode_IOL_AUTOSTART;
                
                portConfig.u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_PortConfigList);
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Config\" mode\n", portNumber);
                IOLM_SMI_vPortConfigurationReq(IOLM_SMI_CLIENT_APP, portNumber, sizeof(portConfig), (INT8U*)&portConfig);
                port[portNumber].exampleState = IOLM_eExampleState_ConfigWait;
            }
                break;

            case IOLM_eExampleState_ConfigWait:                                                                             /* wait for read response */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"ConfigWait\" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_PortStatusRequest:                                                                      /* start check port status */    
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Port request\" mode\n", portNumber);
                IOLM_SMI_vPortStatusReq(IOLM_SMI_CLIENT_APP, portNumber);
                port[portNumber].exampleState = IOLM_eExampleState_PortStatusWait;
            }
                break;

            case IOLM_eExampleState_PortStatusWait:                                                                         /*  wait for port status response */ 
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"PortStatusWait\" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_ReadVendor:                                                                             /* start reading vendor name */
            {
                /* Allocate ArgBlock Memory */            
                INT8U aMem[IOLM_SMI_ARGBLOCK_ONREQ_LEN(0)];
                IOLM_SMI_SOnRequestData* pReq = (IOLM_SMI_SOnRequestData*)aMem;

                /* Fill Request */
                pReq->u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_OnRequestDataRead);
                pReq->u16Index = IOLM_SMI_ENDIAN_16(IOL_eISDUIndex_VendorName);                                             /* Index 16 = VendorName     */
                pReq->u8Subindex = 0;

                /* Send Request */
                IOLM_EXMPL_printf("IO-Link port %u: Read Vendor Name\n", portNumber);
                IOLM_SMI_vDeviceReadReq(IOLM_SMI_CLIENT_APP, portNumber, IOLM_SMI_ARGBLOCK_ONREQ_LEN(0), (INT8U*)aMem);     /* Data */
                port[portNumber].exampleState = IOLM_eExampleState_ReadVendorWait;
            }
                break;

            case IOLM_eExampleState_ReadVendorWait:                                                                         /* wait for read response */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"ReadVendorWait\" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_ReadProductName:                                                                        /* start reading ProductName */
            {
                /* Allocate ArgBlock Memory */
                INT8U aMem[IOLM_SMI_ARGBLOCK_ONREQ_LEN(0)];
                IOLM_SMI_SOnRequestData* pReq = (IOLM_SMI_SOnRequestData*)aMem;

                /* Fill Request */
                pReq->u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_OnRequestDataRead);

                pReq->u16Index = IOLM_SMI_ENDIAN_16(IOL_eISDUIndex_ProductName);                                            /* Index 18 = ProductName     */
                pReq->u8Subindex = 0;

                /* Send Request */
                IOLM_EXMPL_printf("IO-Link port %u: Read Product Name\n", portNumber);
                IOLM_SMI_vDeviceReadReq(IOLM_SMI_CLIENT_APP, portNumber, IOLM_SMI_ARGBLOCK_ONREQ_LEN(0), (INT8U*)aMem);     /* Data */
                port[portNumber].exampleState = IOLM_eExampleState_ReadProductNameWait;
            }
                break;

            case IOLM_eExampleState_ReadProductNameWait:                                                                    /* wait for read response */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"ReadProductNameWait\" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_ReadSerialnumber:                                                                       /* start reading Serialnumber */
            {
                /* Allocate ArgBlock Memory */
                INT8U aMem[IOLM_SMI_ARGBLOCK_ONREQ_LEN(0)];
                IOLM_SMI_SOnRequestData* pReq = (IOLM_SMI_SOnRequestData*)aMem;

                /* Fill Request */
                pReq->u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_OnRequestDataRead);
                pReq->u16Index = IOLM_SMI_ENDIAN_16(IOL_eISDUIndex_SerialNumber);                                           /* Index 21 = Serialnumber */
                pReq->u8Subindex = 0;

                /* Send Request */
                IOLM_EXMPL_printf("IO-Link port %u: Read Serialnumber\n", portNumber);
                IOLM_SMI_vDeviceReadReq(IOLM_SMI_CLIENT_APP, portNumber, IOLM_SMI_ARGBLOCK_ONREQ_LEN(0), (INT8U*)aMem);     /* Data */
                port[portNumber].exampleState = IOLM_eExampleState_ReadSerialnumberWait;
             
            }
                break;

            case IOLM_eExampleState_ReadSerialnumberWait:                                                                   /* wait for read response */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"ReadSerialnumberWait\" mode\n", portNumber);
            }
                break;
/*          SPECIFIC DEVICE PROCESS DATA WRITE CALL:  
            For different devices there a different process data write commands. For example: one will turn on a LED, an other will change 
            a specific temperature measurement mode and so on, so this following comments are just for inspiration, 
            you need to check your device manual first before write process data.
            case IOLM_eExampleState_WriteProcessDataValue:
            {
                //Allocate ArgBlock Memory 
                INT8U aMem[IOLM_SMI_ARGBLOCK_ONREQ_LEN(2)];                                                               
                IOLM_SMI_SPDOut* pReq = (IOLM_SMI_SPDOut*)aMem;

                //Fill Request
                pReq->u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_PDOut);
                pReq->u8OE = 0x01;
                pReq->u8OutputDataLength = 0x01;                                                                            
                pReq->au8CurrenData[0]= 0x02;                  

                //Send PD Request
                IOLM_SMI_vPDOutReq(IOLM_SMI_CLIENT_APP, portNumber, IOLM_SMI_ARGBLOCK_ONREQ_LEN(2), (INT8U*)aMem);
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Write Process Data\" mode\n", portNumber);
                IOLM_EXMPL_printf("IO-Link port %u: write 0x02\n", portNumber);
                port[portNumber].exampleState = IOLM_eExampleState_WriteProcessDataValueWait;
            }
                break;

            case IOLM_eExampleState_WriteProcessDataValueWait:
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Wait Write Process Data\" mode\n", portNumber);
            }
                break;
*/

            case IOLM_eExampleState_PortStatusRequestPD:                                                                      /* start check port status */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Process Data Exchange \" mode\n", portNumber);
                IOLM_SMI_vPortStatusReq(IOLM_SMI_CLIENT_APP, portNumber);
                port[portNumber].exampleState = IOLM_eExampleState_PortStatusWaitPD;
            }
                break;

            case IOLM_eExampleState_PortStatusWaitPD:                                                                         /*  wait for port status response */
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Process Data Exchange wait \" mode\n", portNumber);
            }
                break;

            case IOLM_eExampleState_ReadProcessDataValue:                                                                   /* start reading process data */
            {
                IOLM_SMI_vPDInReq(IOLM_SMI_CLIENT_APP, portNumber);
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Port Read Process Data\" mode\n", portNumber);
                port[portNumber].exampleState = IOLM_eExampleState_ReadProcessDataValueWait;
            }
                break;

            case IOLM_eExampleState_ReadProcessDataValueWait:    
            {
                IOLM_EXMPL_printf("IO-Link port %u: Port is now in \"Wait Read Process Data\" mode\n", portNumber);
            }
                break;

            default:
            {
                port[portNumber].exampleState = IOLM_eExampleState_PortStatusRequest;
            }       
        }
    }
}

/*!
 * \brief
 * Get Master identification confirmation callback.  
 *
 * This function is called if a Master identification read request has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the master identification (#IOLM_SMI_SMasterident).
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbMasterIdentificationCnf(uint8_t clientID_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SMasterident* psuMasterIdent = (IOLM_SMI_SMasterident*)pArgBlock_p;
    psuMasterIdent->u16ArgBlockID = IOLM_SMI_ENDIAN_16(IOLM_SMI_eArgBlockID_MasterIdent);

    if (error_p != IOL_eErrorType_NONE)
    {
        IOLM_EXMPL_printf("NO Master Identification Possible.\n");
    }
    else
    {
        IOLM_EXMPL_printf("Master Identification: \nMasterID:  %u \nMax ports: %u \n ",
            IOLM_SMI_ENDIAN_32(psuMasterIdent->u32MasterID),
            psuMasterIdent->u8MaxNumberOfPorts);
    }
}

/*!
 * \brief
 * Port event indication
 *
 * This function is called if a port event indication occures. 
 *
 * \param[in]  port_p                  Port ID.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the port event data (#IOLM_SMI_SPortEvent).
 *
 * \return void
 *
 */
void IOLM_EXMPL_PortEventInd(uint8_t port_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SPortEvent* pEvent = (IOLM_SMI_SPortEvent*)pArgBlock_p;
    /* Example Output*/
    IOLM_EXMPL_printf("IO-Link port %u: Port event - qualifier: 0x%02x - code: 0x%04x\n",
        port_p, pEvent->u8EventQualifier,
        pEvent->u16EventCode); 
}

/*!
 * \brief
 * Device event indication
 *
 * This function is called if a device event indication occures.
 *
 * \param[in]  port_p                  Port ID.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the device event data (#IOLM_SMI_SDeviceEvent).
 *
 * \return void
 *
 */
void IOLM_EXMPL_DeviceEventInd(uint8_t port_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SDeviceEvent* pEvent = (IOLM_SMI_SDeviceEvent*)pArgBlock_p;
    /* Example Output */
    IOLM_EXMPL_printf("IO-Link port %u: Device event - qualifier: 0x%02x - code: 0x%04x\n",
        port_p,
        pEvent->u8EventQualifier,
        pEvent->u16EventCode);
}

/*!
 * \brief
 * Port configuration confirmation callback.
 *
 * This function is called if a device configuration request has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbPortConfigurationCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p)
{
    if (error_p != IOL_eErrorType_NONE)
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else 
    {
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequest;
    }
}

/*!
 * \brief
 * Port status confirmation callback.
 *
 * This function is called if a device port status request has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the port status list (#IOLM_SMI_SPortStatusList).
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbPortStatusCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p,uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SPortStatusList* psuPortStatus = (IOLM_SMI_SPortStatusList*)pArgBlock_p;
    
    port[port_p].currentStackPortStatus = psuPortStatus->u8PortStatusInfo;

    if (error_p != IOL_eErrorType_NONE) 
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else if  (port[port_p].currentStackPortStatus == IOLM_SMI_ePortStatus_OPERATE)
    {
        if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequest
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWait)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadVendor;
        }
        else if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequestPD
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWaitPD)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadProcessDataValue;
        }   
    }
    else
    {
        IOLM_EXMPL_printf("Not in IOLM_SMI_ePortStatus_OPERATE\n");
        port[port_p].exampleState = IOLM_eExampleState_Init;
    }
}

/*!
 * \brief
 * Device write confirmation callback.
 *
 * This function is called if a device write request has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 *
 * \return void
 *
 */
void IOLM_SMI_cbDeviceWriteCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p)
{
    IOLM_EXMPL_printf("---------------Write Confirmation------------------\n");
    if (error_p != IOL_eErrorType_NONE)
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else
    {
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequestPD;
    }
}

/*!
 * \brief
 * Device read confirmation callback.
 *
 * This function is called if a device read request has finished. 
 * This is the answer to an on-request data. 
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the device on-request data (#IOLM_SMI_SOnRequestData).
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbDeviceReadCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SOnRequestData* pReq = (IOLM_SMI_SOnRequestData*)pArgBlock_p;
    uint8_t payloadLength = argBlockLength_p - IOLM_SMI_ARGBLOCK_ONREQ_LEN(0);
    char aDeviceData[DEVICE_READ_DATA_LENGTH] = {""};
    strncpy(aDeviceData, (char*)pReq->au8Data, payloadLength);
    aDeviceData[payloadLength] = 0;

    if (payloadLength > sizeof(aDeviceData))
    {
        payloadLength = sizeof(aDeviceData);
    }
    if (error_p != IOL_eErrorType_NONE)
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else if (port[port_p].exampleState == IOLM_eExampleState_ReadVendorWait)
    {
        IOLM_EXMPL_printf("IO-Link port %u: Device vendor is \"%s\"\n", port_p, aDeviceData);
        port[port_p].exampleState = IOLM_eExampleState_ReadProductName;
    }
    else if (port[port_p].exampleState == IOLM_eExampleState_ReadProductNameWait)
    {
        IOLM_EXMPL_printf("IO-Link port %u: Device vendor is \"%s\"\n", port_p, aDeviceData);
        port[port_p].exampleState = IOLM_eExampleState_ReadSerialnumber;
    }
    else if (port[port_p].exampleState == IOLM_eExampleState_ReadSerialnumberWait)
    {
        IOLM_EXMPL_printf("IO-Link port %u: Serialnumber: \"%s\"\n", port_p, aDeviceData);
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequestPD;
    }
}

/*!
 * \brief
 *  Set output data confirmation callback.
 *
 * This function is called if a process data write request from the master to the device has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbPDOutCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p)
{
    if (error_p != IOL_eErrorType_NONE)
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else
    {
        IOLM_EXMPL_printf("---------------Write Process Data Success-----------------\n");
        port[port_p].exampleState = IOLM_eExampleState_ReadProcessDataValue;
    }
}

/*!
 * \brief
 * Get input data request and confirmation.
 *
 * This function is called if a process data read request from the master to the device has finished.
 *
 * \param[in]  clientID_p              Client ID.
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 * \param[in]  argBlockLength_p        Length of ArgBlock.
 * \param[in]  pArgBlock_p             Data pointer which points to the PDIn data (#IOLM_SMI_SPDIn).
 *
 * \return void
 *
 */
void IOLM_EXMPL_cbPDInCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p)
{
    IOLM_SMI_SPDIn* psuPDIn = (IOLM_SMI_SPDIn*)pArgBlock_p;
    uint8_t au8CurrentData[PD_INPUT_LENGTH] = { 0, };
    uint8_t memoryCompareResult = 0;
    uint8_t dataElement;
    if (error_p != IOL_eErrorType_NONE)
    {
        port[port_p].exampleState = IOLM_EXMPL_PortErrorHandler(port_p, error_p);
    }
    else
    {
        IOLM_EXMPL_printf("---------------Read Process Data Success-------------------\n");
        memcpy(au8CurrentData, psuPDIn->au8Data, psuPDIn->u8InputDataLength);
        memoryCompareResult = memcmp(au8CurrentData, port[port_p].aPDInCnfData, sizeof(au8CurrentData));
        if (memoryCompareResult != 0)
        {
            memcpy(port[port_p].aPDInCnfData, au8CurrentData, sizeof(au8CurrentData));
            for (dataElement = 0; dataElement <= psuPDIn->u8InputDataLength; dataElement++)
            {
                if (au8CurrentData[dataElement] != 0)
                {
                    IOLM_EXMPL_printf("IO-Link port %u: Process data: 0x%02x \n", port_p, au8CurrentData[dataElement]);
                }
            }
        }
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequestPD;
    }
}

/*!
 * \brief
 * Call port error handler.
 *
 * This function is called from the callback, if a error occures.
 *
 * \param[in]  port_p                  Port ID.
 * \param[in]  error_p                 Error message as #IOL_EErrorType.
 *
 * \return  portState_g[port_p] as uint8_t.
 *
 */
uint8_t IOLM_EXMPL_PortErrorHandler(uint8_t port_p, uint16_t error_p)
{
    IOLM_EXMPL_printf("IO-Link port %u: Read failed with error 0x%04x", port_p, error_p);
    switch (error_p)
    {
    case IOL_eErrorType_ARGBLOCK_NOT_SUPPORTED:
        IOLM_EXMPL_printf(" - \"ARGBLOCK_NOT_SUPPORTED\"");
        if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequest
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWait)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadVendor;
        }
        else if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequestPD
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWaitPD)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadProcessDataValue;
        }
        else
        {
            port[port_p].exampleState = IOLM_eExampleState_Init;
        }
        break;
    case IOL_eErrorType_ARGBLOCK_ID_NOT_SUPPORTED:
        IOLM_EXMPL_printf(" - \"ARGBLOCK_ID_NOT_SUPPORTED\"");
        if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequest
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWait)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadVendor;
        }
        else if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequestPD
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWaitPD)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadProcessDataValue;
        }
        else
        {
            port[port_p].exampleState = IOLM_eExampleState_Init;
        }
        break;
    case IOL_eErrorType_SERVICE_NOT_SUPPORTED:
        IOLM_EXMPL_printf(" - \"SERVICE_NOT_SUPPORTED\"");
        if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequest
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWait)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadVendor;
        }
        else if (port[port_p].exampleState == IOLM_eExampleState_PortStatusRequestPD
            || port[port_p].exampleState == IOLM_eExampleState_PortStatusWaitPD)
        {
            port[port_p].exampleState = IOLM_eExampleState_ReadProcessDataValue;
        }
        break;
    case IOL_eErrorType_DEVICE_NOT_ACCESSIBLE:
        IOLM_EXMPL_printf(" - \"DEVICE_NOT_ACCESSIBLE\"");
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequest;
        break;
    case IOL_eErrorType_PORT_NUM_INVALID:
        IOLM_EXMPL_printf(" - \"PORT_NUM_INVALID\"");
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequest;
        break;
    case IOL_eErrorType_SERVICE_TEMP_UNAVAILABLE:
        IOLM_EXMPL_printf(" - \"SERVICE_TEMP_UNAVAILABLE\"");
        port[port_p].exampleState = IOLM_eExampleState_PortStatusRequest;
        break;
    case IOL_eErrorType_PORT_CONFIG_INCONSISTENT:
        IOLM_EXMPL_printf(" - \"PORT_CONFIG_INCONSISTENT\"");
        port[port_p].exampleState = IOLM_eExampleState_Init;
        break;
    default:
        port[port_p].exampleState = IOLM_eExampleState_Init;
        break;
    }
    IOLM_EXMPL_printf("\n");
    return port[port_p].exampleState;
}