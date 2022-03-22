/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
\addtogroup SampleAppl Sample Application
@{
*/

/**
\file Sampleappl.c
\author EthercatSSC@beckhoff.com
\brief Implementation

\version 5.12

<br>Changes to version V5.11:<br>
V5.12 ECAT1: update SM Parameter measurement (based on the system time), enhancement for input only devices and no mailbox support, use only 16Bit pointer in process data length caluclation<br>
V5.12 ECAT2: big endian changes<br>
V5.12 EOE1: move icmp sample to the sampleappl,add EoE application interface functions<br>
V5.12 EOE2: prevent static ethernet buffer to be freed<br>
V5.12 EOE3: fix memory leaks in sample ICMP application<br>
V5.12 FOE1: update new interface,move the FoE sample to sampleappl,add FoE application callback functions<br>
<br>Changes to version V5.10:<br>
V5.11 ECAT11: create application interface function pointer, add eeprom emulation interface functions<br>
V5.11 ECAT4: enhance SM/Sync monitoring for input/output only slaves<br>
<br>Changes to version V5.01:<br>
V5.10 ECAT10: Add missing include 'objdef.h'<br>
              Add process data size calculation to sampleappl<br>
V5.10 ECAT6: Add "USE_DEFAULT_MAIN" to enable or disable the main function<br>
V5.10 FC1100: Stop stack if hardware init failed<br>
<br>Changes to version V5.0:<br>
V5.01 APPL2: Update Sample Application Output mapping<br>
V5.0: file created
*/


/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecat_def.h"

#if SAMPLE_APPLICATION
#include "applInterface.h"


#define _SAMPLE_APPLICATION_
#include "sampleappl.h"
#undef _SAMPLE_APPLICATION_

#if BOOTSTRAPMODE_SUPPORTED
#include"bootmode.h"
#endif


/*--------------------------------------------------------------------------------------
------
------    local types and defines
------
--------------------------------------------------------------------------------------*/
#if FOE_SUPPORTED
/*ECATCHANGE_START(V5.12) FOE1*/
#define    MAX_FILE_NAME_SIZE    16

/** \brief  MAX_FILE_SIZE: Maximum file size */
#if FC1100_HW
#define MAX_FILE_SIZE                             0x6400000 /*100MB*/
#else
#define MAX_FILE_SIZE                             0x180
#endif
/*ECATCHANGE_END(V5.12) FOE1*/
#endif

/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/

#if EOE_SUPPORTED
/*ECATCHANGE_START(V5.12) EOE1*/
/*Create broadcast Ethernet address*/
const    UINT8    BroadcastEthernetAddress[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
/*ECATCHANGE_END(V5.12) EOE1*/
#endif


#if FOE_SUPPORTED
/*ECATCHANGE_START(V5.12) FOE1*/
UINT32           nFileWriteOffset;
CHAR             aFileName[MAX_FILE_NAME_SIZE];
#if MBX_16BIT_ACCESS
UINT16 MBXMEM     aFileData[(MAX_FILE_SIZE >> 1)];
#else
UINT8 MBXMEM      aFileData[MAX_FILE_SIZE];
#endif //#else #ifMBX_16BIT_ACCESS


UINT32 u32FileSize;
#if BOOTSTRAPMODE_SUPPORTED
const UINT16 HUGE aFirmwareDownloadHeader[4] = { 0x4345, 0x5441, 0x5746, 0x5F5F }; // "ECATFW__"
#endif
/*ECATCHANGE_END(V5.12) FOE1*/
#endif //#if FOE_SUPPORTED

/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/
#if EOE_SUPPORTED

/////////////////////////////////////////////////////////////////////////////////////////
/**

\brief    This function calculates a checksum (only for an even number of bytes).
\brief Note that if you are going to checksum a checksummed packet that includes the checksum,
\brief you have to compliment the output.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 EOEAPPL_CalcCheckSum(UINT16 MBXMEM *pWord, UINT16 nLen)
{
    UINT32 crc;
    UINT32 CrcLo;
    UINT32 CrcHi;
    UINT16 RetCrc;

    crc = 0;
    while (nLen > 1)
    {
        crc += SWAPWORD(*pWord);
        pWord++;
        nLen -= 2;
    }
    if (nLen == 1)                          // if nLen odd
    {
        crc += *((UINT8*)pWord);
    }

    CrcLo = LOWORD(crc);
    CrcHi = HIWORD(crc);
    crc = CrcLo + CrcHi;

    CrcHi = HIWORD(crc);
    crc += CrcHi;
    if (crc == 0xFFFF)                     // remove the -0 ambiguity
    {
        crc = 0;
    }

    RetCrc = (UINT16)crc;
    RetCrc = ~RetCrc;
    return(RetCrc);
}


void EoeReceive(UINT16 *pFrame, UINT16 frameSize)
{
    /*ECATCHANGE_START(V5.12) ECAT2*/
    switch (SWAPWORD(((ETHERNET_FRAME *)pFrame)->FrameType))
        /*ECATCHANGE_END(V5.12) ECAT2*/
    {
    case ETHERNET_FRAME_TYPE_ARP1_SW:
    {
#if STATIC_ETHERNET_BUFFER
        ETHERNET_FRAME MBXMEM * pSendFrame = (ETHERNET_FRAME MBXMEM *)aEthernetSendBuffer;
#else
        ETHERNET_FRAME MBXMEM * pSendFrame = (ETHERNET_FRAME MBXMEM *) ALLOCMEM(frameSize);
#endif
        ARP_IP_HEADER MBXMEM    * pArpIp = (ARP_IP_HEADER MBXMEM    *) &pSendFrame[1];

        /*Copy Receive Frame to create ARP Reply*/
        MBXMEMCPY(pSendFrame, pFrame, frameSize);
#if MBX_16BIT_ACCESS
        if ((MBXMEMCMP(BroadcastEthernetAddress, pSendFrame->Destination.w, 4) == 0)
            && (pArpIp->lengthHwAddrPortAddr == 0x0406) /*lowbyte: length of MAC address; highbyte length of port address*/
#else
        if ((MBXMEMCMP(BroadcastEthernetAddress, pSendFrame->Destination.b, 4) == 0)
            && (pArpIp->lengthHwAddr == ETHERNET_ADDRESS_LEN)
            && (pArpIp->lengthProtAddr == SIZEOF(UINT32))
#endif
            && (pArpIp->hwAddrSpace == SWAPWORD(ARP_HW_ADDR_SPACE_ETHERNET_SW))
            && (pArpIp->protAddrSpace == SWAPWORD(ETHERNET_FRAME_TYPE_IP_SW))
            && (pArpIp->opcode == SWAPWORD(ARP_OPCODE_REQUEST_SW))
            )
        {
#if MBX_16BIT_ACCESS
            MBXMEMCPY(pSendFrame->Destination.w, pSendFrame->Source.w, 6);
            MBXMEMCPY(pSendFrame->Source.w, &aMacAdd[0], 6);

            MBXMEMCPY(pArpIp->macDest.w, pArpIp->macSource.w, 6);
            MBXMEMCPY(pArpIp->macSource.w, &aMacAdd[0], 6);
#else
            MBXMEMCPY(pSendFrame->Destination.b, pSendFrame->Source.b, 6);
            MBXMEMCPY(pSendFrame->Source.b, &aMacAdd[0], 6);

            MBXMEMCPY(pArpIp->macDest.b, pArpIp->macSource.b, 6);
            MBXMEMCPY(pArpIp->macSource.b, &aMacAdd[0], 6);
#endif
            MBXMEMCPY(pArpIp->ipDest, pArpIp->ipSource, 4);
            MBXMEMCPY(pArpIp->ipSource, aIpAdd, 4);

            pArpIp->opcode = SWAPWORD(ARP_OPCODE_REPLY_SW);

            EOE_SendFrameRequest((UINT16 MBXMEM *) pSendFrame, ARP_IP_HEADER_LEN + ETHERNET_FRAME_LEN);
        }
        /*ECATCHANGE_START(V5.12) EOE3*/
        else
        {
#if STATIC_ETHERNET_BUFFER
            pSendFrame = NULL;
#else
            if (pSendFrame != NULL)
            {
                FREEMEM(pSendFrame);
                pSendFrame = NULL;
            }
#endif
        }
        /*ECATCHANGE_END(V5.12) EOE3*/
    }
    break;
    case ETHERNET_FRAME_TYPE_IP_SW:
    {
#if STATIC_ETHERNET_BUFFER
        ETHERNET_IP_ICMP_MAX_FRAME MBXMEM * pIPHeader = (ETHERNET_IP_ICMP_MAX_FRAME MBXMEM *) aEthernetSendBuffer;
#else
        ETHERNET_IP_ICMP_MAX_FRAME MBXMEM * pIPHeader = (ETHERNET_IP_ICMP_MAX_FRAME MBXMEM *) ALLOCMEM(frameSize);
#endif

        /*Copy Receive Frame to create ICMP Reply*/
        MBXMEMCPY(pIPHeader, pFrame, frameSize);


#if MBX_16BIT_ACCESS
        if ((((SWAPWORD(pIPHeader->Ip.ttlProtocol) & IP_HEADER_PROTOCOL_MASK) >> 8) == IP_PROTOCOL_ICMP)
            && ((SWAPWORD(pIPHeader->IpData.Icmp.typeCode) & ICMP_TYPE_MASK) == ICMP_TYPE_ECHO)
#else
        if ((pIPHeader->Ip.protocol == IP_PROTOCOL_ICMP)
            && (pIPHeader->IpData.Icmp.type == ICMP_TYPE_ECHO)
#endif
            && (MBXMEMCMP(pIPHeader->Ip.dest, aIpAdd, 4) == 0)
            )
        {
            // ping requested
            UINT16 length;
#if !BIG_ENDIAN_FORMAT
            UINT16 lo = 0;
            UINT16 hi = 0;
#endif
            UINT32 tmp;

#if BIG_ENDIAN_FORMAT
            length = pIPHeader->Ip.length;
#else
            // length is in BigEndian format -> swap bytes
            lo = ((pIPHeader->Ip.length) & 0xff) << 8;
            hi = pIPHeader->Ip.length >> 8;
            length = hi + lo;
#endif
            // swap src and dest ip address
            MEMCPY(&tmp, pIPHeader->Ip.src, 4);
            MEMCPY(pIPHeader->Ip.src, pIPHeader->Ip.dest, 4);
            MEMCPY(pIPHeader->Ip.dest, &tmp, 4);

#if MBX_16BIT_ACCESS
            // set ping reply command
            pIPHeader->IpData.Icmp.typeCode &= ~ICMP_TYPE_MASK;
            pIPHeader->IpData.Icmp.typeCode |= ICMP_TYPE_ECHO_REPLY;

            // swap src and dest mac address
            MBXMEMCPY(pIPHeader->Ether.Destination.w, pIPHeader->Ether.Source.w, 6);
            MBXMEMCPY(pIPHeader->Ether.Source.w, aMacAdd, 6);

#else
            // set ping reply command
            pIPHeader->IpData.Icmp.type = ICMP_TYPE_ECHO_REPLY;

            // swap src and dest mac address
            MBXMEMCPY(pIPHeader->Ether.Destination.b, pIPHeader->Ether.Source.b, 6);
            MBXMEMCPY(pIPHeader->Ether.Source.b, aMacAdd, 6);
#endif

            // calculate ip checksum
            pIPHeader->Ip.cksum = 0;
            pIPHeader->Ip.cksum = SWAPWORD(EOEAPPL_CalcCheckSum((UINT16 MBXMEM *) &pIPHeader->Ip, IP_HEADER_MINIMUM_LEN));
            // calculate icmp checksum
            pIPHeader->IpData.Icmp.checksum = 0;
            /* type cast because of warning was added */
            pIPHeader->IpData.Icmp.checksum = SWAPWORD(EOEAPPL_CalcCheckSum((UINT16 MBXMEM *) &pIPHeader->IpData.Icmp, (UINT16)(length - 20)));
            /* type cast because of warning was added */
            EOE_SendFrameRequest((UINT16 MBXMEM *) pIPHeader, (UINT16)(ETHERNET_FRAME_LEN + length));
        }
        else
        {
            //protocol not supported => free allocated buffer
#if STATIC_ETHERNET_BUFFER
            /*ECATCHANGE_START(V5.12) EOE2*/
            pIPHeader = (ETHERNET_IP_ICMP_MAX_FRAME MBXMEM *) NULL;
            /*ECATCHANGE_END(V5.12) EOE2*/
#else
            if (pIPHeader != NULL)
            {
                FREEMEM(pIPHeader);
                pIPHeader = NULL;
            }
#endif
        }
    }
    break;
    }
}
/*ECATCHANGE_END(V5.12) EOE1*/
#endif

#if FOE_SUPPORTED
/*ECATCHANGE_START(V5.12) FOE1*/
UINT16 FoE_Read(UINT16 MBXMEM * pName, UINT16 nameSize, UINT32 password, UINT16 maxBlockSize, UINT16 *pData)
{
    UINT16 i = 0;
    UINT16 sizeError = 0;

    CHAR aReadFileName[MAX_FILE_NAME_SIZE];

    
    if ((nameSize + 1) > MAX_FILE_NAME_SIZE)
    {
        return ECAT_FOE_ERRCODE_DISKFULL;
    }

    /*Read requested file name to endianess conversion if required*/
    MBXSTRCPY(aReadFileName, pName, nameSize);
    aReadFileName[nameSize] = '\0';

    if (u32FileSize == 0)
    {
        /* No file stored*/
        return ECAT_FOE_ERRCODE_NOTFOUND;
    }

    /* for test only the written file name can be read */
    for (i = 0; i < nameSize; i++)
    {
        if (aReadFileName[i] != aFileName[i])
        {
            /* file name not found */
            return ECAT_FOE_ERRCODE_NOTFOUND;
        }
    }

    sizeError = maxBlockSize;

    if (u32FileSize < sizeError)
    {
        sizeError = (UINT16)u32FileSize;
    }

    /*copy the first foe data block*/
    MEMCPY(pData, aFileData, sizeError);

    return sizeError;
}

UINT16 FoE_ReadData(UINT32 offset, UINT16 maxBlockSize, UINT16 *pData)
{
    UINT16 sizeError = 0;

    if (u32FileSize < offset)
    {
        return 0;
    }

    /*get file length to send*/
    sizeError =(UINT16) (u32FileSize - offset);


    if (sizeError > maxBlockSize)
    {
        /*transmit max block size if the file data to be send is greater than the max data block*/
        sizeError = maxBlockSize;
    }
    /*copy the foe data block 2 .. n*/
    MEMCPY(pData, &(((UINT8 *)aFileData)[offset]), sizeError);

    return sizeError;
}


UINT16 FoE_WriteData(UINT16 MBXMEM * pData, UINT16 Size, BOOL bDataFollowing)
{
#if BOOTSTRAPMODE_SUPPORTED
    if (bBootMode)
    {
        return BL_Data(pData, Size);
    }
    else
#endif
        if ((nFileWriteOffset + Size) > MAX_FILE_SIZE)
        {
            return ECAT_FOE_ERRCODE_DISKFULL;
        }

    if (Size)
    {

#if MBX_16BIT_ACCESS
        MBXMEMCPY(&aFileData[(nFileWriteOffset >> 1)], pData, Size);
#else
        MBXMEMCPY(&aFileData[nFileWriteOffset], pData, Size);
#endif

    }

    if (bDataFollowing)
    {
        /* FoE-Data services will follow */
        nFileWriteOffset += Size;
        
    }
    else
    {
        /* last part of the file is written */
        u32FileSize = nFileWriteOffset + Size;
        nFileWriteOffset = 0;
    }

    return 0;
}


UINT16 FoE_Write(UINT16 MBXMEM * pName, UINT16 nameSize, UINT32 password)
{
#if BOOTSTRAPMODE_SUPPORTED
    if ((nameSize >= SIZEOF(aFirmwareDownloadHeader))
        && (pName[0] == aFirmwareDownloadHeader[0])
        && (pName[1] == aFirmwareDownloadHeader[1])
        && (pName[2] == aFirmwareDownloadHeader[2])
        )
    {
        if (bBootMode)
        {
            BL_StartDownload(password);
            return 0;
        }
        else
        {
            return ECAT_FOE_ERRCODE_BOOTSTRAPONLY;
        }
    }
    else
        if (bBootMode)
        {
            return ECAT_FOE_ERRCODE_NOTINBOOTSTRAP;
        }
        else
#endif
    if (nameSize < MAX_FILE_NAME_SIZE)
    {
        /* for test every file name can be written */
        MBXSTRCPY(aFileName, pName, nameSize);
        MBXSTRCPY(aFileName + nameSize, "\0", 1); //string termination


        nFileWriteOffset = 0;
        u32FileSize = 0;
        return 0;
    }
    else
    {
        return ECAT_FOE_ERRCODE_DISKFULL;
    }

}

/*ECATCHANGE_END(V5.12) FOE1*/
#endif //#if FOE_SUPPORTED

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    The function is called when an error state was acknowledged by the master

*////////////////////////////////////////////////////////////////////////////////////////

void    APPL_AckErrorInd(UINT16 stateTrans)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from INIT to PREOP when
             all general settings were checked to start the mailbox handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case this function will be called cyclically
            until a value unequal NOERROR_INWORK is returned

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from PREEOP to INIT
             to stop the mailbox handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pIntMask    pointer to the AL Event Mask which will be written to the AL event Mask
                        register (0x204) when this function is succeeded. The event mask can be adapted
                        in this function
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from PREOP to SAFEOP when
           all general settings were checked to start the input handler. This function
           informs the application about the state transition, the application can refuse
           the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartInputHandler(UINT16 *pIntMask)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from SAFEOP to PREEOP
             to stop the input handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopInputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from SAFEOP to OP when
             all general settings were checked to start the output handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from OP to SAFEOP
             to stop the output handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GenerateMapping(UINT16 *pInputSize,UINT16 *pOutputSize)
{
#if COE_SUPPORTED
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM * pPDO = NULL;
    UINT16 PDOSubindex0 = 0;
    UINT32 *pPDOEntry = NULL;
    UINT16 PDOEntryCnt = 0;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;

    /*Scan object 0x1C12 RXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);
        if(pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
/*ECATCHANGE_START(V5.12) ECAT1*/
                pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3)/2);    //goto PDO entry
/*ECATCHANGE_END(V5.12) ECAT1*/
                // we increment the expected output size depending on the mapped Entry
                OutputSize += (UINT16) ((*pPDOEntry) & 0xFF);
            }
        }
        else
        {
            /*assigned PDO was not found in object dictionary. return invalid mapping*/
            OutputSize = 0;
            result = ALSTATUSCODE_INVALIDOUTPUTMAPPING;
            break;
        }
    }

    OutputSize = (OutputSize + 7) >> 3;

    if(result == 0)
    {
        /*Scan Object 0x1C13 TXPDO assign*/
        for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
        {
            pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);
            if(pPDO != NULL)
            {
                PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
                for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
                {
/*ECATCHANGE_START(V5.12) ECAT1*/
                    pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3)/2);    //goto PDO entry
/*ECATCHANGE_END(V5.12) ECAT1*/
                    // we increment the expected output size depending on the mapped Entry
                    InputSize += (UINT16) ((*pPDOEntry) & 0xFF);
                }
            }
            else
            {
                /*assigned PDO was not found in object dictionary. return invalid mapping*/
                InputSize = 0;
                result = ALSTATUSCODE_INVALIDINPUTMAPPING;
                break;
            }
        }
    }
    InputSize = (InputSize + 7) >> 3;

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;
    return result;
#else
    *pInputSize = PD_INPUT_SIZE;
    *pOutputSize = PD_OUTPUT_SIZE;
    
    return ALSTATUSCODE_NOERROR;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to input process data

\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC18 && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void APPL_InputMapping(UINT16* pData)
{
#if MAX_PD_INPUT_SIZE > 0
    MEMCPY(pData,&InputCounter,SIZEOF(InputCounter));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC18  && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void APPL_OutputMapping(UINT16* pData)
{
#if MAX_PD_OUTPUT_SIZE > 0
    MEMCPY(&OutputCounter,pData,SIZEOF(OutputCounter));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will called from the synchronisation ISR 
            or from the mainloop if no synchronisation is supported
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_Application(void)
{
#if MAX_PD_INPUT_SIZE > 0
#if MAX_PD_OUTPUT_SIZE > 0
    /*Hardware independent sample application*/
    if(OutputCounter > 0)
    {
        InputCounter = OutputCounter+1;
    }
    else
#endif
    {
        InputCounter++;
    }
#endif
}

#if EXPLICIT_DEVICE_ID
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    The Explicit Device ID of the EtherCAT slave

 \brief     Calculate the Explicit Device ID
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GetDeviceID()
{
    return 0x5;
}
#endif

#if USE_DEFAULT_MAIN
/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This is the main function

*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC24
int main(void)
#else
void main(void)
#endif
{
    /* initialize the Hardware and the EtherCAT Slave Controller */
#if FC1100_HW
    if(HW_Init())
    {
        HW_Release();
        return;
    }
#else
    HW_Init();
#endif
    MainInit();

#if EOE_SUPPORTED
/*ECATCHANGE_START(V5.12) EOE1*/
    pAPPL_EoeReceive = EoeReceive;
/*ECATCHANGE_END(V5.12) EOE1*/
#endif

#if FOE_SUPPORTED
/*ECATCHANGE_START(V5.12) FOE1*/
    pAPPL_FoeRead = FoE_Read;
    pAPPL_FoeReadData = FoE_ReadData;
    pAPPL_FoeWriteData = FoE_WriteData;
    pAPPL_FoeWrite = FoE_Write;
    u32FileSize = 0;
/*ECATCHANGE_END(V5.12) FOE1*/
#endif

    bRunApplication = TRUE;
    do
    {
        MainLoop();

    } while (bRunApplication == TRUE);

    HW_Release();
#if _PIC24
    return 0;
#endif
}
#endif //#if USE_DEFAULT_MAIN

#endif //#if SAMPLE_APPLICATION

/** @} */

