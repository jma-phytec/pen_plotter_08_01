/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
\addtogroup EcatAppl EtherCAT application
@{
*/


/**
\file ecatappl.c
\author EthercatSSC@beckhoff.com
\brief Implementation
This file contains the Process Data interface

\version 5.12

<br>Changes to version V5.11:<br>
V5.12 APPL1: add optional application function called from the main loop (after mbx and esm are executed)<br>
V5.12 BOOT1: add a bootloader sample application (only the ESM and FoE is supported)<br>
V5.12 COE4: add timestamp object (0x10F8) and update diagnosis handling<br>
V5.12 ECAT1: update SM Parameter measurement (based on the system time), enhancement for input only devices and no mailbox support, use only 16Bit pointer in process data length caluclation<br>
V5.12 ECAT3: update PD monitoring variables even if the slave is not in OP, disable all interrupts in case that the pd handling is called from the mainloop<br>
V5.12 ECAT5: update Sync error counter/flag handling,check enum memory alignment depending on the processor,in case of a polled timer disable ESC interrupts during DC_CheckWatchdog<br>
V5.12 ECAT8: reset appl function pointer on startup, update timeout calculation during eeprom access<br>
V5.12 EEPROM1: get read size from register 0x502.6<br>
V5.12 EEPROM2: clear CRC Error bit only in case of a valid reload, write station alias only in case of an successful reload,handle full eeprom emlation relaod commands<br>
V5.12 EEPROM3: implement a store EEPROM timeout handler<br>
<br>Changes to version V5.10.1:<br>
V5.11 COE3: change 0x10F3.2 (Sync Error limit) from UINT32 to UINT16 (according to the ETG.1020)<br>
V5.11 ECAT1: update EEPROM access reset operation<br>
V5.11 ECAT10: change PROTO handling to prevent compiler errors<br>
V5.11 ECAT11: create application interface function pointer, add eeprom emulation interface functions<br>
V5.11 ECAT2: update EEPROM access retry cycle (add 10ms delay between two retry cycles)<br>
V5.11 ECAT3: handle bus cycle calculation for input/output only devices and create warning diag message only if calculation failed<br>
V5.11 ECAT4: enhance SM/Sync monitoring for input/output only slaves<br>
V5.11 ECAT6: add function to calculate bus cycle time<br>
V5.11 ECAT8: call PDO_InputMapping only once if DC is enabled and COE is not supported<br>
V5.11 EEPROM1: fix compiler error during pEEPROM pointer initialization<br>
V5.11 EEPROM2: write Station alias value to EEPROM data register on EEPROM reload command<br>
V5.11 EEPROM3: clear EEPROM error bits<br>
V5.11 EEPROM4: prevent the variable in the EEPROM busy loop to be removed by the compiler<br>
V5.11 ESM7: "add Sync define for 0x22 (""SYNCTYPE_SM2_SYNCHRON""), support value 0x22 for 0x1C33.1 (SM2 sync)"<br>
<br>Changes to version V5.01:<br>
V5.10 COE1: Define one entry description for all 0x1C3x objects and change data type of SI11,12,13 to UINT16 (according ETG.1020)<br>
V5.10 ECAT1: Correct calculation of blinking and flashing sequence<br>
V5.10 ECAT13: Update Synchronisation handling (FreeRun,SM Sync, Sync0, Sync1)<br>
              Compare DC UINT configuration (by ESC Config data) vs. DC activation register settings<br>
              Update 0x1C3x entries<br>
V5.10 ECAT2: Prevent EEPROM data null pointer access (if the pointer is null an command error is set)<br>
             EEPROM emulation return command error if unknown command was received<br>
V5.10 ECAT4: Update alignment marco for 8 to 15 bit alignments (16 and 32 Bit controllers)<br>
             Bugfix calculate LED blink frequency<br>
V5.10 ECAT7: Add "bInitFinished" to indicate if the initialization is complete<br>
V5.10 HW2: Change HW_GetTimer return value to UINT32<br>
<br>Changes to version V5.0:<br>
V5.01 APPL3: Include library demo application<br>
V5.01 ESC1: Change ESC access function (if EEPROM Emulation is active)<br>
V5.01 ESC2: Add missed value swapping<br>
<br>Changes to version V4.40:<br>
V5.0 TEST1: Add test application. See Application Note ET9300 for more details.<br>
V5.0 ECAT2: Application specific functions are moved to application files.<br>
V5.0 ECAT3: Global dummy variables used for dummy ESC operations.<br>
V5.0 ESC1: ESC 32Bit Access added.<br>
V5.0 ESC3: Add EEPROM emulation support.<br>
V5.0 ESM3: Handling pending ESM transitions.<br>
V5.0 ESC5: Enhance EEPROM access handling.<br>
V5.0 PDO1: AL Event flags are not rechecked in PDO_OutputMappping(). (Already checked before call function)<br>
V5.0 SYNC1: Add missed SM event indication (0x1C32/0x1C33 SI11).<br>
<br>Changes to version V4.30:<br>
V4.40 DIAG1: add diagnosis message support<br>
V4.40 PDO1: merge content of HW_InputMapping (spihw.c/mcihw.c) to PDO_InputMapping. merge content of HW_OutputMapping (spihw.c/mcihw.c) to PDO_OutputMapping.<br>
V4.40 PDO2: Generic process data length calculation<br>
V4.40 ECAT2: call cyclic CheckIfLocalError() to check the local flags<br>
V4.40 HW0: Generic hardware access functions. Add Function (PDI_Isr()), content merged from spihw.c and mcihw.c.<br>
V4.40 WD1: define (ESC_SM_WD_SUPPORTED) to choose ESC SyncManager watchdog or local watchdog<br>
V4.40 ESM2: Change state transition behaviour from SafeOP to OP<br>
V4.40 TIMER1: Change bus cycle time calculation and trigger of ECAT_CheckTimer() if ECAT_TIMER_INT is reset<br>
V4.40 HW1: Add support for fc1100 hardware<br>
<br>Changes to version V4.20:<br>
V4.30 EL9800: EL9800_x cyclic application is moved to el9800.c<br>
V4.30 OBJ 3:    add object dictionary initialization<br>
V4.30 SYNC: add CalcSMCycleTime() (calculation of bus cycle time); change synchronisation control functions<br>
V4.30 PDO: include PDO specific functions (moved from coeappl.c).<br>
               xxx_InputMapping(); xxx_OutputMapping(); xxx_ReadInputs(); xxx_ResetOutputs(); xxx_Application()<br>
V4.30 CiA402: Add CiA402_StateMachine() and CiA402_Application() call<br>
V4.20 DC 1: Add DC pending Statemachine handling<br>
V4.20 PIC24: Add EL9800_4 (PIC24) required source code<br>
V4.20 LED 1: Modified LED Handling<br>
V4.11 APPL 1: The checkWatchdog() function should not called in checkTimer() if this function is triggered by an Interrupt<br>
<br>Changes to version V4.08:<br>
V4.10 LED 1: The handling of the EtherCAT-Error-LED was added<br>
V4.10 AOE 3: The AoE fragment size has to be initialized during the state transition<br>
                 from INIT to PREOP<br>
<br>Changes to version V4.07:<br>
V4.08 LED 1: The handling of the EtherCAT-LED can be (de-)selected by the switch LEDS_SUPPORTED<br>
                 because the ET1100 and ET1200 have an output port which could be connected directly.<br>
<br>Changes to version V4.01:<br>
V4.02 ECAT 1: The watchdog timer variables shall be initialized.<br>
<br>Changes to version V4.00:<br>
V4.01 APPL 1: If the application is running in synchron mode and no SM event<br>
              is received, the application should be called from the main loop<br>
V4.01 APPL 2: In FreeRun mode the output should only be copied if the slave is in OP<br>
<br>Changes to version V3.20:<br>
V4.00 APPL 1: The watchdog checking should be done by a microcontroller<br>
                 timer because the watchdog trigger of the ESC will be reset too<br>
                 if only a part of the sync manager data is written<br>
V4.00 APPL 2: The setting of EtherCAT state LEDs were included<br>
V4.00 APPL 3: The outputs should be reset to a safe state,<br>
                   when the state OP is left<br>
V4.00 APPL 4: An example for the EEPROM access through the ESC is shown in<br>
                   the function APPL_StartMailboxHandler<br>
V4.00 APPL 5: The inputs should be read once when the state transition<br>
                   from PREOP to SAFEOP is made<br>
V4.00 APPL 6: The main function was split in MainInit and MainLoop
*/


/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecatslv.h"

#define    _ECATAPPL_ 1
#include "ecatappl.h"
#undef _ECATAPPL_
/*remove definition of _ECATAPPL_ (#ifdef is used in ecatappl.h)*/

#if COE_SUPPORTED
#include "coeappl.h"
#endif


#if DIAGNOSIS_SUPPORTED
#include "diag.h"
#endif

#define _APPL_INTERFACE_ 1
#include "applInterface.h"
#undef _APPL_INTERFACE_

#if CiA402_DEVICE
#include "cia402appl.h"
#elif EL9800_APPLICATION
#include "el9800appl.h"
#elif SAMPLE_APPLICATION
#include "sampleappl.h"
#elif TEST_APPLICATION
#include "testappl.h"
#elif SAMPLE_APPLICATION_INTERFACE
#include "SampleApplicationInterface.h"
#elif BOOTLOADER_SAMPLE
/*ECATCHANGE_START(V5.12) BOOT1*/
#include "bootloaderappl.h"
/*ECATCHANGE_END(V5.12) BOOT1*/
#else
/*CODE_INSERT_START (APPLICATION_FILE)*/
#include "tiescappl.h"
/*CODE_INSERT_END*/
#endif

/*CODE_INSERT_START (EEPROM_FILE)*/
/*CODE_INSERT_END*/


/*--------------------------------------------------------------------------------------
------
------    local Types and Defines
------
--------------------------------------------------------------------------------------*/

#if ESC_EEPROM_ACCESS_SUPPORT
#define MAX_CMD_RETIRES     10
#endif

#if COE_SUPPORTED || !ECAT_TIMER_INT
#ifndef ECAT_TIMER_INC_P_MS
/**
 * \todo Define the timer ticks per ms
 */
#warning "Define the timer ticks per ms"
#endif /* #ifndef ECAT_TIMER_INC_P_MS */
#endif /* #if COE_SUPPORTED || !ECAT_TIMER_INT */



#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
#if (MAX_PD_OUTPUT_SIZE > 0) && (MAX_PD_INPUT_SIZE > 0)
#define    MEASUREMENT_ACTIVE (((sSyncManOutPar.u16GetCycleTime & 0x1) == 0x1) || ((sSyncManInPar.u16GetCycleTime & 0x1) == 0x1))
#elif MAX_PD_OUTPUT_SIZE > 0
#define    MEASUREMENT_ACTIVE ((sSyncManOutPar.u16GetCycleTime & 0x1) == 0x1)
#elif MAX_PD_INPUT_SIZE > 0
#define    MEASUREMENT_ACTIVE ((sSyncManInPar.u16GetCycleTime & 0x1) == 0x1)
#else
#define    MEASUREMENT_ACTIVE (FALSE)
#endif
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif

/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/
#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
/*variables required to calculate values for SM Synchronisation objects (0x1C3x)*/

UINT32 u32CycleTimeStartValue; /** <\brief contains the timer start value to measure the application cycle (used in freerun and SM2 sync)*/

UINT32 u32MinCycleTimeStartValue; /** <\brief timeout counter in ms to measure the process timings (stored in 0x1C3x)*/

UINT32 u32SystemTimeReadFailure; /** <\brief System time measurement failure (the value is calculated on main init)*/

BOOL bMinCycleTimeMeasurementStarted; /** <\brief Indicates if the min cycle measurement is started*/

UINT32 u32MinCycleTimeValue; /** <\brief tmp value of the min cycle time during measurement*/

/*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif


#if ESC_EEPROM_EMULATION
/*ECATCHANGE_START(V5.12) EEPROM3*/
UINT16 u16EepromStoreTimeoutCounter; /** <\brief contains the current timer value to store the EEPROM data*/

UINT16 u16EepromStoreTimeoutValue; /** <\brief timeout in ms to store the EEPROM data*/
/*ECATCHANGE_END(V5.12) EEPROM3*/
#endif //#if ESC_EEPROM_EMULATION


#if MAX_PD_OUTPUT_SIZE > 0
#ifdef EDMA_LATENCY_ENHANCEMENT
UINT16             * volatile aPdOutputData;
#else
UINT16             aPdOutputData[(MAX_PD_OUTPUT_SIZE>>1)];
#endif
#endif
#if MAX_PD_INPUT_SIZE > 0
#ifdef EDMA_LATENCY_ENHANCEMENT
UINT16           * volatile aPdInputData;
#else
UINT16           aPdInputData[(MAX_PD_INPUT_SIZE>>1)];
#endif
#endif

/*variables are declared in ecatslv.c*/
#if ESC_32BIT_ACCESS
    extern VARVOLATILE UINT32    u32dummy;
#elif ESC_16BIT_ACCESS
    extern VARVOLATILE UINT16    u16dummy;
#else
    extern VARVOLATILE UINT8 u8dummy;
#endif

BOOL bInitFinished = FALSE; /** < \brief indicates if the initialization is finished*/

/*-----------------------------------------------------------------------------------------
------
------    local functions
------
-----------------------------------------------------------------------------------------*/
#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
UINT32 GetSystemTimeDelay(UINT32 u32StartTime);
void HandleCycleTimeMeasurement(void);
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif
#endif //#if COE_SUPPORTED

/*-----------------------------------------------------------------------------------------
------
------    Functions
------
-----------------------------------------------------------------------------------------*/
#if MAX_PD_INPUT_SIZE > 0
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief      This function will copies the inputs from the local memory to the ESC memory
*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC18 && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void PDO_InputMapping(void)
{

#if COE_SUPPORTED
#if !_PIC18
#if ((MIN_PD_CYCLE_TIME == 0) || (PD_INPUT_CALC_AND_COPY_TIME == 0))
    /*ECATCHANGE_START(V5.12) ECAT1*/
    UINT32 u32TimeValue = 0;
    UINT16 ALEvent = HW_GetALEventRegister_Isr();
    ALEvent = SWAPWORD(ALEvent);


    if (MEASUREMENT_ACTIVE)
    {
        u32TimeValue = GetSystemTimeDelay(0);
    }
    /*ECATCHANGE_END(V5.12) ECAT1*/
#endif /* ((MIN_PD_CYCLE_TIME == 0) || (PD_INPUT_CALC_AND_COPY_TIME == 0)) */
#endif//#if !_PIC18
#endif /* COE_SUPPORTED */






  
    APPL_InputMapping((UINT16*)aPdInputData);
    HW_EscWriteIsr(((MEM_ADDR *) aPdInputData), nEscAddrInputData, nPdInputSize );

    

#if COE_SUPPORTED
#if !_PIC18
#if ((MIN_PD_CYCLE_TIME == 0) || (PD_INPUT_CALC_AND_COPY_TIME == 0))
    /*ECATCHANGE_START(V5.12) ECAT1*/

    if (MEASUREMENT_ACTIVE)
    {
        u32TimeValue = GetSystemTimeDelay(u32TimeValue);

#if (PD_INPUT_CALC_AND_COPY_TIME == 0)
        if (sSyncManInPar.u32CalcAndCopyTime < u32TimeValue)
        {
            sSyncManInPar.u32CalcAndCopyTime = u32TimeValue;
        }
#endif

#if (MIN_PD_CYCLE_TIME == 0)
        /* handle the min cycle time measurement only if a new cycle was started (prevent measurement failures if the get cycle time bit is set within a process data cycle)*/
        if (bMinCycleTimeMeasurementStarted == TRUE)
        {

            /* add input mapping time to the min cycle time*/
            u32MinCycleTimeValue = u32MinCycleTimeValue + u32TimeValue;


#if MAX_PD_OUTPUT_SIZE > 0
            if (sSyncManOutPar.u32MinCycleTime < u32MinCycleTimeValue)
            {
                    sSyncManOutPar.u32MinCycleTime = u32MinCycleTimeValue;
            }
#endif

            if (sSyncManInPar.u32MinCycleTime < u32MinCycleTimeValue)
            {
                    sSyncManInPar.u32MinCycleTime = u32MinCycleTimeValue;
            }

            bMinCycleTimeMeasurementStarted = FALSE;
        }
#endif /* (MIN_PD_CYCLE_TIME == 0) */
    }

    /*ECATCHANGE_END(V5.12) ECAT1*/
#endif /* ((MIN_PD_CYCLE_TIME == 0) || (PD_INPUT_CALC_AND_COPY_TIME == 0)) */
#endif //#if !_PIC18
#endif /* COE_SUPPORTED */

}
#endif // #if MAX_PD_INPUT_SIZE > 0
#if MAX_PD_OUTPUT_SIZE > 0
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will copies the outputs from the ESC memory to the local memory.
        This function is only called in case of an SM2 (output process data) event.
*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC18  && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void PDO_OutputMapping(void)
{
#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
   UINT32 u32TimeValue = 0;
   if (MEASUREMENT_ACTIVE)
   {
#if ((MIN_PD_CYCLE_TIME == 0) || (PD_OUTPUT_CALC_AND_COPY_TIME == 0))
      u32TimeValue = GetSystemTimeDelay(0);
      u32MinCycleTimeStartValue = u32TimeValue;

      bMinCycleTimeMeasurementStarted = TRUE;
      u32MinCycleTimeValue = 0;

#endif /* ((MIN_PD_CYCLE_TIME == 0) || (PD_OUTPUT_CALC_AND_COPY_TIME == 0)) */

      HandleCycleTimeMeasurement();

   }
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif /* COE_SUPPORTED */

    HW_EscReadIsr(((MEM_ADDR *)aPdOutputData), nEscAddrOutputData, nPdOutputSize );
    APPL_OutputMapping((UINT16*) aPdOutputData);


#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
#if ((MIN_PD_CYCLE_TIME == 0) || (PD_OUTPUT_CALC_AND_COPY_TIME == 0))
    if (MEASUREMENT_ACTIVE)
    {
       u32TimeValue = GetSystemTimeDelay(u32TimeValue);

#if (PD_OUTPUT_CALC_AND_COPY_TIME == 0)
       if (sSyncManOutPar.u32CalcAndCopyTime < u32TimeValue)
       {
          sSyncManOutPar.u32CalcAndCopyTime = u32TimeValue;
       }
#endif

#if (MIN_PD_CYCLE_TIME == 0)
       /* add the first part of the min cycle time */
       u32MinCycleTimeValue = u32TimeValue;
#endif
    }
#endif /* #if ((MIN_PD_CYCLE_TIME == 0) || (PD_OUTPUT_CALC_AND_COPY_TIME == 0)) */
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif /* COE_SUPPORTED */
}
#endif //#if MAX_PD_OUTPUT_SIZE > 0

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    This function shall be called every 1ms.
 \brief If the switch ECAT_TIMER_INT is 0, the watchdog control is implemented without using
 \brief interrupts. In this case a local timer register is checked every ECAT_Main cycle
 \brief and the function is triggered if 1 ms is elapsed
 *////////////////////////////////////////////////////////////////////////////////////////

void ECAT_CheckTimer(void)
{

    /*decrement the state transition timeout counter*/
    if(bEcatWaitForAlControlRes &&  (EsmTimeoutCounter > 0))
    {
        EsmTimeoutCounter--;
    }

#if !ESC_SM_WD_SUPPORTED 
#if MAX_PD_OUTPUT_SIZE > 0
    /*The SyncManager watchdog is not supported, the local watchdog is used*/
    ECAT_CheckWatchdog();
#endif	 //#if MAX_PD_OUTPUT_SIZE > 0
#endif //#if !ESC_SM_WD_SUPPORTED 

#if UC_SET_ECAT_LED
    ECAT_SetLedIndication();
#endif

#if DC_SUPPORTED
    DC_CheckWatchdog();
#endif

#if ESC_EEPROM_EMULATION
/*ECATCHANGE_START(V5.12) EEPROM3*/

    if (u16EepromStoreTimeoutValue > 0)
    {
        u16EepromStoreTimeoutCounter++;

        if (u16EepromStoreTimeoutValue <= u16EepromStoreTimeoutCounter)
        {
            u16EepromStoreTimeoutValue = 0;
            u16EepromStoreTimeoutCounter = 0;

            if (pAPPL_EEPROM_Store != NULL)
            {
                pAPPL_EEPROM_Store();
            }

        }
    }
/*ECATCHANGE_END(V5.12) EEPROM3*/

#endif //#if ESC_EEPROM_EMULATION

#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) COE4*/

#if DC_SUPPORTED
    /* Increment the counter every ms between two updates based on the system time (32Bit overrun is handled in COE_SyncTimeStamp) */
    if (!b32BitDc || ((u64Timestamp & 0xFFFFFFFF) <= 4293000000))
    {
#endif

        /* the timestamp is stored in ns */
        u64Timestamp = u64Timestamp + 1000000;

#if DC_SUPPORTED
    }
    else if(b32BitDc)
    {
        /* in case of a 32Bit DC and almost expired time stamp check for a DC overrun*/
        u32CheckForDcOverrunCnt = CHECK_DC_OVERRUN_IN_MS;
    }

    u32CheckForDcOverrunCnt++;
#endif
/*ECATCHANGE_END(V5.12) COE4*/
#endif //#if !_PIC18
#endif
}

#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    In case of non DC synchronization the cycle time measurement is started and 0x1C3.2 (Cycle time) is updated
*////////////////////////////////////////////////////////////////////////////////////////
void HandleCycleTimeMeasurement(void)
{
    if (!bDcSyncActive) //no DC sync configured (cycle time measurement 0x1C3x.2 is only available in no DC sync modes)
    {
        if (u32CycleTimeStartValue > 0)
        {
            /* bus cycle completed*/
            u32CycleTimeStartValue = GetSystemTimeDelay(u32CycleTimeStartValue);

#if (MAX_PD_OUTPUT_SIZE > 0)
            if ((sSyncManOutPar.u32CycleTime == 0) || (sSyncManOutPar.u32CycleTime > u32CycleTimeStartValue))
            {
                    sSyncManOutPar.u32CycleTime = u32CycleTimeStartValue;
            }
#endif

#if (MAX_PD_INPUT_SIZE > 0)
            if ((sSyncManInPar.u32CycleTime == 0) || (sSyncManInPar.u32CycleTime > u32CycleTimeStartValue))
            {
                    sSyncManInPar.u32CycleTime = u32CycleTimeStartValue;
            }
#endif
        }
        /* get next start value */
        u32CycleTimeStartValue = GetSystemTimeDelay(0);
        
    }/* No DC sync configured */
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param    u32StartTime  Old system time (0x910:0x913) value 

\return   System time delta in ns

\brief    Calculates the difference between the old and current system time value in ns.
          NOTE: This function only handles a 32Bit system time values (therefore the maximum delay about 4sec).
*////////////////////////////////////////////////////////////////////////////////////////

UINT32 GetSystemTimeDelay(UINT32 u32StartTime)
{
   UINT32 u32CurValue = 0;
   UINT32 u32Delta = 0;

   
   HW_EscReadDWordIsr(u32CurValue, ESC_SYSTEMTIME_OFFSET);

   if (u32CurValue > 0)
   {
      if (u32StartTime <= u32CurValue)
      {
         u32Delta = u32CurValue - u32StartTime;
      }
      else
      {
         //The 32Bit timer is wrapped around
         u32Delta = u32CurValue + (0xFFFFFFFF - u32StartTime);
      }
   }// current value successfully read out

   if (u32StartTime > 0)
   {

       /*the difference between two timestamps are calculated => subtract measurement failure*/
       if (u32SystemTimeReadFailure < u32Delta)
       {
           u32Delta = u32Delta - u32SystemTimeReadFailure;
       }
       else
       {
           /*set the delta to 0 if the measurement failure is greater than the calculated difference*/
           u32Delta = 0;
       }
   }
   return u32Delta;
}
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif //#if COE_SUPPORTED

#if AL_EVENT_ENABLED
#if _PIC18
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void PDI_Isr(void)
{
    if(bEscIntEnabled)
    {
        /* get the AL event register */
        UINT16  ALEvent = HW_GetALEventRegister_Isr();
        ALEvent = SWAPWORD(ALEvent);

#if MAX_PD_OUTPUT_SIZE > 0
        if ( ALEvent & PROCESS_OUTPUT_EVENT )
        {
#if DC_SUPPORTED
            if(bDcRunning && bDcSyncActive)
            {
                /* Reset SM/Sync0 counter. Will be incremented on every Sync0 event*/
                u16SmSync0Counter = 0;
            }
#if COE_SUPPORTED
            if(sSyncManOutPar.u16SmEventMissedCounter > 0)
            {
                sSyncManOutPar.u16SmEventMissedCounter--;
            }

/*ECATCHANGE_START(V5.12) ECAT5*/
#if MAX_PD_INPUT_SIZE > 0
            sSyncManInPar.u16SmEventMissedCounter = sSyncManOutPar.u16SmEventMissedCounter;
#endif
/*ECATCHANGE_END(V5.12) ECAT5*/

 #else
            if(u16SmEventMissedCounter > 0)
            {
                u16SmEventMissedCounter--;
            }
#endif //#else #if COE_SUPPORTED

#endif //#if DC_SUPPORTED

        /* Outputs were updated, set flag for watchdog monitoring */
        bEcatFirstOutputsReceived = TRUE;

#if !ESC_SM_WD_SUPPORTED
        /* reset watchdog counter */
        EcatWdCounter = 0;
#endif

        /*
            handle output process data event
        */
        if ( bEcatOutputUpdateRunning )
        {
            /* slave is in OP, update the outputs */
            PDO_OutputMapping();
        }
        else
        {
            /* Just acknowledge the process data event in the INIT,PreOP and SafeOP state */
#if ESC_32BIT_ACCESS
            HW_EscReadDWordIsr(u32dummy,nEscAddrOutputData);
            HW_EscReadDWordIsr(u32dummy,(nEscAddrOutputData+nPdOutputSize-4));
#elif ESC_16BIT_ACCESS
            HW_EscReadWordIsr(u16dummy,nEscAddrOutputData);
            HW_EscReadWordIsr(u16dummy,(nEscAddrOutputData+nPdOutputSize-2));
#else
            HW_EscReadByteIsr(u8dummy,nEscAddrOutputData);
            HW_EscReadByteIsr(u8dummy,(nEscAddrOutputData+nPdOutputSize-1));
#endif
        }
        }
#elif MAX_PD_INPUT_SIZE > 0

        if (ALEvent & PROCESS_INPUT_EVENT)
        {
#if DC_SUPPORTED
            if (bDcRunning && bDcSyncActive)
            {
                /* Reset SM/Sync0 counter. Will be incremented on every Sync0 event*/
                u16SmSync0Counter = 0;
            }
#if COE_SUPPORTED
            if (sSyncManInPar.u16SmEventMissedCounter > 0)
            {
                sSyncManInPar.u16SmEventMissedCounter--;
            }
#else
            if (u16SmEventMissedCounter > 0)
            {
                u16SmEventMissedCounter--;
            }
#endif //#else #if COE_SUPPORTED
#endif //#if DC_SUPPORTED
        }

#endif //#elif MAX_PD_INPUT_SIZE > 0 #if MAX_PD_OUTPUT_SIZE > 0

        /*
            Call ECAT_Application() in SM Sync mode
        */
#if COE_SUPPORTED
#if MAX_PD_OUTPUT_SIZE > 0
        if (sSyncManOutPar.u16SyncType == SYNCTYPE_SM_SYNCHRON)
        {
            /* The Application is synchronized to process data Sync Manager event*/
            ECAT_Application();
        }
#elif MAX_PD_INPUT_SIZE > 0
        if(sSyncManInPar.u16SyncType == SYNCTYPE_SM_SYNCHRON)
        {
            /* The Application is synchronized to process data Sync Manager event*/
            ECAT_Application();
        }
#endif
#else
        if(!bDcSyncActive)
        {
            /* The Application is synchronized to process data Sync Manager event*/
            ECAT_Application();
        }

#endif //#else #if COE_SUPPORTED

#if MAX_PD_INPUT_SIZE > 0
    if ( bEcatInputUpdateRunning 
#if COE_SUPPORTED
       && ((sSyncManInPar.u16SyncType == SYNCTYPE_SM_SYNCHRON) || (sSyncManInPar.u16SyncType == SYNCTYPE_SM2_SYNCHRON))
#else
        && !bDcSyncActive
#endif
        )
    {
        /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
        PDO_InputMapping();
    }
#endif //#if MAX_PD_INPUT_SIZE > 0

    /*
      Check if cycle exceed
    */
 #if !TIESC_HW
    /*if next SM event was triggered during runtime increment cycle exceed counter*/
    ALEvent = HW_GetALEventRegister_Isr();
    ALEvent = SWAPWORD(ALEvent);

#if MAX_PD_OUTPUT_SIZE > 0
    if ( ALEvent & PROCESS_OUTPUT_EVENT )
    {
#if COE_SUPPORTED
        sSyncManOutPar.u16CycleExceededCounter++;
#if MAX_PD_INPUT_SIZE > 0
        sSyncManInPar.u16CycleExceededCounter = sSyncManOutPar.u16CycleExceededCounter;
#endif
#endif

      /* Acknowledge the process data event*/
#if ESC_32BIT_ACCESS
            HW_EscReadDWordIsr(u32dummy,nEscAddrOutputData);
            HW_EscReadDWordIsr(u32dummy,(nEscAddrOutputData+nPdOutputSize-4));
#elif ESC_16BIT_ACCESS
            HW_EscReadWordIsr(u16dummy,nEscAddrOutputData);
            HW_EscReadWordIsr(u16dummy,(nEscAddrOutputData+nPdOutputSize-2));
#else
            HW_EscReadByteIsr(u8dummy,nEscAddrOutputData);
            HW_EscReadByteIsr(u8dummy,(nEscAddrOutputData+nPdOutputSize-1));
#endif
    }
#elif MAX_PD_INPUT_SIZE > 0
    if ( ALEvent & PROCESS_INPUT_EVENT )
    {
#if COE_SUPPORTED
/*ECATCHANGE_START(V5.12)*/
        sSyncManInPar.u16CycleExceededCounter++;
/*ECATCHANGE_END(V5.12)*/
#endif

      /* Acknowledge the process data event*/
#if ESC_32BIT_ACCESS
        u32dummy = 0;
        HW_EscWriteDWordIsr(u32dummy,nEscAddrInputData);
#elif ESC_16BIT_ACCESS
        u16dummy = 0;
        HW_EscWriteWordIsr(u16dummy,nEscAddrInputData);
#else
        u8dummy = 0;
        HW_EscWriteByteIsr(u8dummy,nEscAddrInputData);
#endif

    }
#endif
#else //!TIESC_HW
    /* In TI ESC above approach won't work to figure out cycle exceeded scenario as it may take longer
       to clear AL event IRQ by firmware, so we measure PDI ISR period instead */
#ifdef SUPPORT_TI_ESC_CYCLE_EXCEEDED_COUNTER
    static Uint32 curPdiTimeStamp = 0, lastPdiTimeStamp = 0, pdiIsrPeriod;
    bsp_get_local_sys_time(&curPdiTimeStamp, 0);
    if (curPdiTimeStamp < lastPdiTimeStamp)
        pdiIsrPeriod = 0xFFFFFFFF-lastPdiTimeStamp+curPdiTimeStamp;
    else  
        pdiIsrPeriod =  curPdiTimeStamp - lastPdiTimeStamp;
    lastPdiTimeStamp = 	curPdiTimeStamp;
#if MAX_PD_OUTPUT_SIZE > 0		
    if (pdiIsrPeriod	< sSyncManOutPar.u32MinCycleTime)
#elif MAX_PD_INPUT_SIZE > 0
    if (pdiIsrPeriod	< sSyncManInPar.u32MinCycleTime)
#endif
    {	
#if MAX_PD_OUTPUT_SIZE > 0
    if ( ALEvent & PROCESS_OUTPUT_EVENT )
    {
#if COE_SUPPORTED
        sSyncManOutPar.u16CycleExceededCounter++;
#if MAX_PD_INPUT_SIZE > 0
        sSyncManInPar.u16CycleExceededCounter = sSyncManOutPar.u16CycleExceededCounter;
#endif
#endif
    }
#elif MAX_PD_INPUT_SIZE > 0
    if ( ALEvent & PROCESS_INPUT_EVENT )
    {
        sSyncManInPar.u16CycleExceededCounter++;
	}	
#endif
    }
#endif //SUPPORT_TI_ESC_CYCLE_EXCEEDED_COUNTER
#endif //!TIESC_HW
    } //if(bEscIntEnabled)

/*ECATCHANGE_START(V5.12) ECAT5*/
#if COE_SUPPORTED
    COE_UpdateSyncErrorStatus();
#endif
/*ECATCHANGE_END(V5.12) ECAT5*/

}
#endif //#if AL_EVENT_ENABLED

#if DC_SUPPORTED
void Sync0_Isr(void)
{
     Sync0WdCounter = 0;

    if(bDcSyncActive)
    {
#if MAX_PD_OUTPUT_SIZE > 0
#if !AL_EVENT_ENABLED
        /*ECATCHANGE_START(V5.12) ECAT3*/
        UINT16  ALEvent = HW_GetALEventRegister_Isr();
        ALEvent = SWAPWORD(ALEvent);


        if (ALEvent & PROCESS_OUTPUT_EVENT)
        {
            /* Reset SM/Sync0 counter.*/
            u16SmSync0Counter = 0;

#if COE_SUPPORTED
            if (sSyncManOutPar.u16SmEventMissedCounter > 0)
            {
                sSyncManOutPar.u16SmEventMissedCounter--;
            }

            /*ECATCHANGE_START(V5.12) ECAT5*/
#if MAX_PD_INPUT_SIZE > 0
            sSyncManInPar.u16SmEventMissedCounter = sSyncManOutPar.u16SmEventMissedCounter;
#endif
            /*ECATCHANGE_END(V5.12) ECAT5*/

#else
            if (u16SmEventMissedCounter > 0)
            {
                u16SmEventMissedCounter--;
            }
#endif //#if COE_SUPPORTED

            /* Outputs were updated, set flag for watchdog monitoring */
            bEcatFirstOutputsReceived = TRUE;


#if !ESC_SM_WD_SUPPORTED
            /* reset watchdog counter */
            EcatWdCounter = 0;
#endif

            if (bEcatOutputUpdateRunning)
            {
                /* Output mapping was not done by the PDI ISR */
                PDO_OutputMapping();
            }
        }

        /*ECATCHANGE_END(V5.12) ECAT3*/
#endif //#if !AL_EVENT_ENABLED
#endif // MAX_PD_OUTPUT_SIZE > 0

#if MAX_PD_INPUT_SIZE > 0
        if ( bEcatInputUpdateRunning )
        {
            LatchInputSync0Counter++;
        }
#endif

#if MAX_PD_OUTPUT_SIZE || MAX_PD_INPUT_SIZE 
        if(u16SmSync0Value > 0)
        {
           /* Check if Sm-Sync sequence is invalid */
           if (u16SmSync0Counter > u16SmSync0Value)
           {
#if COE_SUPPORTED 
#if MAX_PD_OUTPUT_SIZE > 0
              if ((nPdOutputSize > 0) && (sSyncManOutPar.u16SmEventMissedCounter <= sErrorSettings.u16SyncErrorCounterLimit))
              {
                 sSyncManOutPar.u16SmEventMissedCounter = sSyncManOutPar.u16SmEventMissedCounter + 3;
              }
#endif

#if MAX_PD_INPUT_SIZE > 0
           if ((nPdInputSize > 0) && (nPdOutputSize == 0) && (sSyncManInPar.u16SmEventMissedCounter <= sErrorSettings.u16SyncErrorCounterLimit))
           {
               sSyncManInPar.u16SmEventMissedCounter = sSyncManInPar.u16SmEventMissedCounter + 3;
           }
#endif

#else
            if(u16SmEventMissedCounter < (UINT16)MAX_SM_EVENT_MISSED)
            {
               u16SmEventMissedCounter = u16SmEventMissedCounter + 3;
            }
#endif
           } // if (u16SmSync0Counter > u16SmSync0Value)

           
           if ((nPdOutputSize == 0) && (nPdInputSize > 0))
           {
              /* Input only with DC, check if the last input data was read*/
              UINT16  ALEvent = HW_GetALEventRegister_Isr();
              ALEvent = SWAPWORD(ALEvent);

              if ((ALEvent & PROCESS_INPUT_EVENT) == 0)
              {

                 /* no input data was read by the master, increment the sm missed counter*/
                if (u16SmSync0Counter <= u16SmSync0Value)
                {
                    u16SmSync0Counter++;
                }
              }
              else
              {
                 /* Reset SM/Sync0 counter*/
                 u16SmSync0Counter = 0;

#if COE_SUPPORTED
#if (MAX_PD_INPUT_SIZE > 0)
                 sSyncManInPar.u16SmEventMissedCounter = 0;
#endif
#else
                 u16SmEventMissedCounter = 0;
#endif //#if COE_SUPPORTED

              }
           }
           else if (u16SmSync0Counter <= u16SmSync0Value)
           {

               u16SmSync0Counter++;
           }
        }//SM -Sync monitoring enabled
#endif


        /* Application is synchronized to SYNC0 event*/
        ECAT_Application();

#if MAX_PD_INPUT_SIZE > 0
        if ( bEcatInputUpdateRunning 
           && (LatchInputSync0Value > 0) && (LatchInputSync0Value == LatchInputSync0Counter) ) /* Inputs shall be latched on a specific Sync0 event */
        {
            /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
            PDO_InputMapping();

            if(LatchInputSync0Value == 1)
            {
                /* if inputs are latched on every Sync0 event (otherwise the counter is reset on the next Sync1 event) */
                LatchInputSync0Counter = 0;
            }
        }
#endif //#if MAX_PD_INPUT_SIZE > 0

    }

/*ECATCHANGE_START(V5.12) ECAT5*/
#if COE_SUPPORTED
    COE_UpdateSyncErrorStatus();
#endif
/*ECATCHANGE_END(V5.12) ECAT5*/

}

void Sync1_Isr(void)
{
    Sync1WdCounter = 0;

#if MAX_PD_INPUT_SIZE > 0
        if ( bEcatInputUpdateRunning 
#if COE_SUPPORTED
            && (sSyncManInPar.u16SyncType == SYNCTYPE_DCSYNC1)
#endif
            && (LatchInputSync0Value == 0)) /* Inputs are latched on Sync1 (LatchInputSync0Value == 0), if LatchInputSync0Value > 0 inputs are latched with Sync0 */
        {
            /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
            PDO_InputMapping();
        }

        /* Reset Sync0 latch counter (to start next Sync0 latch cycle) */
        LatchInputSync0Counter = 0;
#endif //#if MAX_PD_INPUT_SIZE > 0
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This function shall called within a 1ms cycle.
        Set Run and Error Led depending on the Led state

*////////////////////////////////////////////////////////////////////////////////////////
#if UC_SET_ECAT_LED
void ECAT_SetLedIndication(void)
{
    static UINT16 ms = 0;
    static UINT16 RunCounter = 0;
    static UINT16 ErrorCounter = 0;

    static UINT8 u8PrevErrorLed = LED_OFF ;
    static UINT8 u8PrevRunLed = LED_OFF ;

    // this code should be called every ms in average
    if ( bEcatOutputUpdateRunning )
    {
        // in OP the EtherCAT state LED is always 1 and ErrorLED is 0
        bEtherCATRunLed = TRUE;
        bEtherCATErrorLed = FALSE;
    }
    else
    {
        ms++;
        if(ms == 50 || ms == 100 ||ms == 150 ||ms == 200)    //set flickering LED if required
        {
            /*Set run Led State*/
            switch ( nAlStatus & STATE_MASK)
            {
            case STATE_INIT:
                // in INIT the EtherCAT state LED is off
                u8EcatRunLed = LED_OFF;
                break;
            case STATE_PREOP:
                // in PREOP the EtherCAT state LED toggles every 200 ms
                u8EcatRunLed = LED_BLINKING;
                break;
            case STATE_SAFEOP:
                // in SAFEOP the EtherCAT state LED is 200 ms on and 1s off
                u8EcatRunLed = LED_SINGLEFLASH;
                break;
            case STATE_OP:
                u8EcatRunLed = LED_ON;
                break;
            case STATE_BOOT:
                u8EcatRunLed = LED_FLICKERING;
                break;
            default:
                u8EcatRunLed = LED_OFF;
            break;
            }//switch nAlStatus

            /*Calculate current Run LED state*/
            if((u8EcatRunLed & 0x20) || ms == 200)    //if fast flag or slow cycle event
            {
                UINT8 NumFlashes = 0;
                if ((u8EcatRunLed  & 0x1F) > 0)
                {
                    NumFlashes = (u8EcatRunLed & 0x1F)+((u8EcatRunLed & 0x1F)-1);    //total number
                }

                /*generate LED code*/
                if(u8EcatRunLed != u8PrevRunLed)    //state changed start with active LED
                {
                    if(u8EcatRunLed & 0x80)    //invert flag enable?
                    {
                            bEtherCATRunLed = FALSE;
                    }
                    else
                    {
                        bEtherCATRunLed = TRUE;
                    }

                    RunCounter = 1;
                }
                else    //second and following LED cycle
                {
                    if(u8EcatRunLed & 0x40)    //toggle LED bit on
                    {
                        bEtherCATRunLed = !bEtherCATRunLed;

                        if(NumFlashes)    //NumFlashes defined => limited LED toggle
                        {
                            RunCounter++;

                            if(RunCounter > NumFlashes)    //toggle led finished
                            {
                                if(u8EcatRunLed & 0x80)    //invert flag enable?
                                {
                                    bEtherCATRunLed = TRUE;
                                }
                                else
                                {
                                    bEtherCATRunLed = FALSE;
                                }

                                if(RunCounter >= (NumFlashes+5))        //toggle time + 5 cycles low
                                {
                                    RunCounter = 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        bEtherCATRunLed = (u8EcatRunLed & 0x01);
                    }
                }
                u8PrevRunLed = u8EcatRunLed;
            }

            /*Calculate current Error LED state*/
            if((u8EcatErrorLed & 0x20) || ms == 200)    //if fast flag or slow cycle event
            {
                UINT8 NumFlashes = 0;
                if ((u8EcatErrorLed  & 0x1F) > 0)
                {
                    NumFlashes = (u8EcatErrorLed & 0x1F)+((u8EcatErrorLed & 0x1F)-1);    //total number
                }

                /*generate LED code*/
                if(u8EcatErrorLed != u8PrevErrorLed)    //state changed start with active LED
                {
                    if(u8EcatErrorLed & 0x80)    //invert flag enable?
                    {
                        bEtherCATErrorLed = FALSE;
                    }
                    else
                    {
                        bEtherCATErrorLed = TRUE;
                    }

                    ErrorCounter = 1;
                }
                else    //second and following LED cycle
                {
                    if(u8EcatErrorLed & 0x40)    //toggle LED bit on
                    {
                        bEtherCATErrorLed = !bEtherCATErrorLed;

                        if(NumFlashes)    //NumFlashes defined => limited LED toggle
                        {
                            ErrorCounter++;

                            if(ErrorCounter > NumFlashes)    //toggle led finished
                            {
                                if(u8EcatErrorLed & 0x80)    //invert flag enable?
                                {
                                    bEtherCATErrorLed = TRUE;
                                }
                                else
                                {
                                    bEtherCATErrorLed = FALSE;
                                }
                                
                                if(ErrorCounter >= (NumFlashes+5))        //toggle time + 5 cycles low
                                {
                                    ErrorCounter = 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        bEtherCATErrorLed = (u8EcatErrorLed & 0x01);
                    }
                }

                u8PrevErrorLed = u8EcatErrorLed;
            }

            if(ms == 200)
            {
                ms = 0;
            }
        }
    }    

    /* set the EtherCAT-LED */
    HW_SetLed(((UINT8)bEtherCATRunLed),((UINT8)bEtherCATErrorLed));
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param     pObjectDictionary   Pointer to application specific object dictionary.
                                NULL if no specific object are available.
\return     0 if initialization was successful

 \brief    This function initialize the EtherCAT Sample Code

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 MainInit(void)
{
    UINT16 Error = 0;
/*Hardware init function need to be called from the application layer*/

#ifdef SET_EEPROM_PTR
    SET_EEPROM_PTR
#endif


/*ECATCHANGE_START(V5.12) ECAT8*/
/* Reset application function pointer*/
#if ESC_EEPROM_EMULATION
pAPPL_EEPROM_Read = NULL;
    pAPPL_EEPROM_Write = NULL;
    pAPPL_EEPROM_Reload = NULL;
    pAPPL_EEPROM_Store = NULL;
#endif //#if ESC_EEPROM_EMULATION


#if EOE_SUPPORTED
    pAPPL_EoeReceive = NULL;
    pAPPL_EoeSettingInd = NULL;
#endif /*#if EOE_SUPPORTED*/

#if FOE_SUPPORTED
    pAPPL_FoeRead = NULL;
    pAPPL_FoeReadData = NULL;
    pAPPL_FoeError = NULL;
    pAPPL_FoeWrite = NULL;
    pAPPL_FoeWriteData = NULL;
#endif //#if FOE_SUPPORTED

    pAPPL_MainLoop = NULL;
/*ECATCHANGE_END(V5.12) ECAT8*/

    /* initialize the EtherCAT Slave Interface */
    ECAT_Init();
#if COE_SUPPORTED
    /* initialize the objects */
    COE_ObjInit();
#endif

#if DIAGNOSIS_SUPPORTED
    /*initialize Diagnose Message memory*/
    Diag_InitMemory();
#endif

#if ESC_EEPROM_ACCESS_SUPPORT
    /*Reset PDI Access*/
    {
#if ESC_32BIT_ACCESS
    UINT32 eepromConfigControl = 0; //register (0x0500 : 0x0503) values

    HW_EscReadDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    eepromConfigControl = SWAPDWORD(eepromConfigControl);

    if((eepromConfigControl & ESC_EEPROM_ASSIGN_TO_PDI_MASK) > 0)
    {
        /*Clear access register(0x0501.1)*/
        eepromConfigControl &= ~ESC_EEPROM_LOCKED_BY_PDI_MASK;

        eepromConfigControl = SWAPDWORD(eepromConfigControl);
        HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    }
#else
    UINT16 eepromConfigControl = 0; //register (0x0500 : 0x0503) values

    HW_EscReadWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    eepromConfigControl = SWAPWORD(eepromConfigControl);

    if((eepromConfigControl & ESC_EEPROM_ASSIGN_TO_PDI_MASK) > 0)
    {
        /*Clear access register(0x0501.1)*/
        eepromConfigControl &= ~ESC_EEPROM_LOCKED_BY_PDI_MASK;

        eepromConfigControl = SWAPWORD(eepromConfigControl);
        HW_EscWriteWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    }
#endif
    }
#endif
    /*indicate that the slave stack initialization finished*/
    bInitFinished = TRUE;

#if COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) ECAT1*/

    bMinCycleTimeMeasurementStarted = FALSE;
    u32CycleTimeStartValue = 0;
    u32MinCycleTimeStartValue = 0;

    u32SystemTimeReadFailure = 0;

    /* Get the System Time read failure */
    {
       UINT32 u32TimeValue = 0;
       UINT32 u32Cnt = 0;
       UINT32 u32Delta = 0;

       while (u32Cnt < 1000)
       {
           HW_EscReadDWordIsr(u32TimeValue, ESC_SYSTEMTIME_OFFSET);
           HW_EscReadDWordIsr(u32Delta, ESC_SYSTEMTIME_OFFSET);

            if (u32TimeValue <= u32Delta)
            {
                    u32Delta = u32Delta - u32TimeValue;
            }
            else
            {
                //The 32Bit timer is wrapped around
                u32Delta = u32Delta + (0xFFFFFFFF - u32TimeValue);
            }


            if (u32SystemTimeReadFailure == 0)
            {
                u32SystemTimeReadFailure = u32Delta;
            }
            else if (u32SystemTimeReadFailure > u32Delta)
            {
                u32SystemTimeReadFailure = u32Delta;
            }

          u32Cnt++;
       }
    
    }


    /*ECATCHANGE_END(V5.12) ECAT1*/
#endif //#if !_PIC18
#endif

#if ESC_EEPROM_EMULATION
/*ECATCHANGE_START(V5.12) EEPROM1*/
    {
#if ESC_32BIT_ACCESS
        UINT32 TmpData = 0;
        HW_EscReadDWord(TmpData, ESC_EEPROM_CONFIG_OFFSET);
        TmpData = SWAPDWORD(TmpData);
#else
        UINT16 TmpData = 0;
        HW_EscReadWord(TmpData, ESC_EEPROM_CONTROL_OFFSET);
        TmpData = SWAPDWORD(TmpData);
#endif
        //get bit6 of register 0x502
        if ((TmpData & ESC_EEPROM_SUPPORTED_READBYTES_MASK) > 0)
        {
            //8Bytes EEPROM read access
            EepromReadSize = 8;
        }
        else
        {
            //4Bytes EEPROM read access
            EepromReadSize = 4;
        }
    }
/*ECATCHANGE_END(V5.12) EEPROM1*/
#endif

/*Application Init need to be called from the application layer*/
     return Error;
}


/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This function shall be called cyclically from main

*////////////////////////////////////////////////////////////////////////////////////////

void MainLoop(void)
{
    
    /*return if initialization not finished */
    if(bInitFinished == FALSE)
    {
        return;
    }



        /* FreeRun-Mode:  bEscIntEnabled = FALSE, bDcSyncActive = FALSE
           Synchron-Mode: bEscIntEnabled = TRUE, bDcSyncActive = FALSE
           DC-Mode:       bEscIntEnabled = TRUE, bDcSyncActive = TRUE */
        if (
#if MAX_PD_OUTPUT_SIZE > 0            
            (!bEscIntEnabled || !bEcatFirstOutputsReceived)     /* SM-Synchronous, but not SM-event received */
#else
            !bEscIntEnabled                                     /*free run*/
#endif
#if DC_SUPPORTED
          && !bDcSyncActive                                               /* DC-Synchronous */
#endif
            )
        {
            /* if the application is running in ECAT Synchron Mode the function ECAT_Application is called
               from the ESC interrupt routine (in mcihw.c or spihw.c),
               in ECAT Synchron Mode it should be additionally checked, if the SM-event is received
               at least once (bEcatFirstOutputsReceived = 1), otherwise no interrupt is generated
               and the function ECAT_Application has to be called here (with interrupts disabled,
               because the SM-event could be generated while executing ECAT_Application) */
            if ( !bEscIntEnabled )
            {
                /* application is running in ECAT FreeRun Mode,
                   first we have to check, if outputs were received */
                UINT16 ALEvent = HW_GetALEventRegister();
                ALEvent = SWAPWORD(ALEvent);

#if MAX_PD_OUTPUT_SIZE > 0
                if ( ALEvent & PROCESS_OUTPUT_EVENT )
                {
                    /* set the flag for the state machine behavior */
                    bEcatFirstOutputsReceived = TRUE;
#if !ESC_SM_WD_SUPPORTED
                    /* reset watchdog counter */
                    EcatWdCounter = 0;
#endif
                    if ( bEcatOutputUpdateRunning )
                    {
                        /* update the outputs */
                        PDO_OutputMapping();
                    }
                }
#if MAX_PD_INPUT_SIZE > 0
                else if ( nPdOutputSize == 0 )
                {
                    /* if no outputs are transmitted, the watchdog must be reset, when the inputs were read */
                    if ( ALEvent & PROCESS_INPUT_EVENT )
                    {
                        /* Outputs were updated, set flag for watchdog monitoring */
                        bEcatFirstOutputsReceived = TRUE;
#if !ESC_SM_WD_SUPPORTED
                        /* reset watchdog counter */
                        EcatWdCounter = 0;
#endif
                    }
                }
#endif
#endif //#if MAX_PD_OUTPUT_SIZE > 0
            }

#if INTERRUPTS_SUPPORTED
/*ECATCHANGE_START(V5.12) ECAT3*/
            DISABLE_ESC_INT();
/*ECATCHANGE_END(V5.12) ECAT3*/
#endif
             ECAT_Application();

#if MAX_PD_INPUT_SIZE > 0
            if ( bEcatInputUpdateRunning )
            {
                /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
                PDO_InputMapping();
            }
#endif //#if MAX_PD_INPUT_SIZE > 0
#if INTERRUPTS_SUPPORTED
/*ECATCHANGE_START(V5.12) ECAT3*/
            ENABLE_ESC_INT();
/*ECATCHANGE_END(V5.12) ECAT3*/
#endif
        }

#if !ECAT_TIMER_INT
        /* there is no interrupt routine for the hardware timer so check the timer register if the desired cycle elapsed*/
        {
            UINT32 CurTimer = (UINT32)HW_GetTimer();

            if(CurTimer>= ECAT_TIMER_INC_P_MS)
            {
                ECAT_CheckTimer();

                HW_ClearTimer();
            }
        }
#endif

#if DC_SUPPORTED && COE_SUPPORTED
#if !_PIC18
/*ECATCHANGE_START(V5.12) COE4*/
        if (u32CheckForDcOverrunCnt >= CHECK_DC_OVERRUN_IN_MS)
        {
            COE_SyncTimeStamp();
        }
/*ECATCHANGE_END(V5.12) COE4*/
#endif //#if !_PIC18
#endif

        /* call EtherCAT functions */
        ECAT_Main();

#if COE_SUPPORTED
        /* call lower prior application part */
       COE_Main();
#endif
       CheckIfEcatError();

#if CiA402_DEVICE
    if(bEcatInputUpdateRunning)
    {
        CiA402_StateMachine();
    }
#endif

/*ECATCHANGE_START(V5.12) APPL1*/
    if (pAPPL_MainLoop != NULL)
    {
        pAPPL_MainLoop();
    }
/*ECATCHANGE_END(V5.12) APPL1*/
}

/*The main function was moved to the application files.*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    ECAT_Application (prev. SSC versions "COE_Application")
 this function calculates and the physical process signals and triggers the input mapping
*////////////////////////////////////////////////////////////////////////////////////////
#if _PIC18 && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void ECAT_Application(void)
{
#if COE_SUPPORTED
#if !_PIC18
#if (MIN_PD_CYCLE_TIME == 0)
    /*ECATCHANGE_START(V5.12) ECAT1*/
    UINT32 u32TimeValue = 0;

    if (MEASUREMENT_ACTIVE)
    {
        u32TimeValue = GetSystemTimeDelay(0);

#if MAX_PD_OUTPUT_SIZE > 0
        if (nPdOutputSize == 0)
#endif
        {
            /* in case of an input only device the cycle starts with an ECAT_Application call*/

            u32MinCycleTimeStartValue = u32TimeValue;
            bMinCycleTimeMeasurementStarted = TRUE;
            u32MinCycleTimeValue = 0;
        }
    } /* measurement started*/
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif /* (MIN_PD_CYCLE_TIME == 0)*/

    if (MEASUREMENT_ACTIVE)
    {
#if MAX_PD_OUTPUT_SIZE > 0
        if (nPdOutputSize == 0)
#endif
        {
            /* in case of an input only device the cycle starts with an ECAT_Application call*/
            HandleCycleTimeMeasurement();
        }
    }
#endif //#if !_PIC18
#endif/* COE_SUPPORTED */

#if CiA402_DEVICE
    /*Axis configuration is written in state change from PREOP to SAFEOP
    => trigger CiA402 Application if device is in SAFEOP or OP
    (Motion Controller function is only triggered if DC Synchronisation is active and valid mode of operation is set)*/
    if(bEcatInputUpdateRunning)
#endif
    {
        APPL_Application();
    }
/* PDO Input mapping is called from the specific trigger ISR */

#if COE_SUPPORTED
#if !_PIC18
#if (MIN_PD_CYCLE_TIME == 0)
/*ECATCHANGE_START(V5.12) ECAT1*/

    if (MEASUREMENT_ACTIVE)
    {
        u32TimeValue = GetSystemTimeDelay(u32TimeValue);


        /* handle the min cycle time measurement only if a new cycle was started (prevent measurement failures if the get cycle time bit is set within a process data cycle)*/
        if (bMinCycleTimeMeasurementStarted == TRUE)
        {

            /* add application execution time to the min cycle time*/
            u32MinCycleTimeValue = u32MinCycleTimeValue + u32TimeValue;

    #if MAX_PD_INPUT_SIZE > 0
            if (nPdInputSize == 0)
    #endif
            {
                /* In case of an output only device the cycle ends with an ECAT_Application call*/


    #if MAX_PD_OUTPUT_SIZE > 0
                if (sSyncManOutPar.u32MinCycleTime < u32MinCycleTimeValue)
                {
                        sSyncManOutPar.u32MinCycleTime = u32MinCycleTimeValue;
                }
    #endif

    #if MAX_PD_INPUT_SIZE > 0
                if (sSyncManInPar.u32MinCycleTime < u32MinCycleTimeValue)
                {
                        sSyncManInPar.u32MinCycleTime = u32MinCycleTimeValue;
                }
    #endif

                bMinCycleTimeMeasurementStarted = FALSE;
            }
        }

    }/* measurement started*/
/*ECATCHANGE_END(V5.12) ECAT1*/
#endif /* #if MIN_PD_CYCLE_TIME == 0 */
#endif //#if !_PIC18
#endif /*COE_SUPPORTED*/
}

#if ESC_EEPROM_ACCESS_SUPPORT
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param        wordaddress      WORD based address in the EEPROM to be read
 \param        wordsize         size in WORD of EEPROM data to be read
 \param        pData            buffer for EEPROM to be read or with EEPROM data to be written
 \param        access           read (ESC_RD) or write (ESC_WR) access

 \return     0 - success, > 0: error code

 \brief        This function is called to read or write the EEPROM data, for BigEndian
 \brief        Controller (switch BIG_ENDIAN_FORMAT set) the data has to be swapped outside
 \brief        of this function)
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 ESC_EepromAccess(UINT32 wordaddress, UINT16 wordsize, UINT16 MBXMEM *pData, UINT8 access)
{
    UINT16 i;
    UINT8 u8TimeOut;
    UINT16 u16RetErr = 0;
    UINT16 u16WordOffset = 0;
    UINT8 RetryCnt = MAX_CMD_RETIRES; //Maximum number of retries (evaluated in case of an Acknowledge Error)

#if ESC_32BIT_ACCESS
    VARVOLATILE UINT32 eepromConfigControl = 0; //register (0x0500 : 0x0503) values

    HW_EscReadDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);

#if BIG_ENDIAN_FORMAT
    eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
#else
    UINT16 nConfig;
    VARVOLATILE UINT16 nControl;

    HW_EscReadWord(nConfig,ESC_EEPROM_CONFIG_OFFSET);
    HW_EscReadWord(nControl, ESC_EEPROM_CONTROL_OFFSET);

#if BIG_ENDIAN_FORMAT
    nConfig = SWAPWORD(nConfig);
    nControl = SWAPWORD(nControl);
#endif

#endif

#if ESC_32BIT_ACCESS
    if ( eepromConfigControl & ESC_EEPROM_ASSIGN_TO_PDI_MASK )
#else
    if ( nConfig & ESC_EEPROM_ASSIGN_TO_PDI_MASK )
#endif
    {
        /* register 0x500.0 is set (should be written by the master before sending
        the state transition request to PREOP),we have access to the EEPROM */
        UINT16 step = 1; /* we write always only 1 word with one write access */

        if ( access == ESC_RD )
        {
            /* read access requested, we have to check if we read 2 (register 0x502.6=0)
            or 4 words (register 0x502.6=1) with one access */
#if ESC_32BIT_ACCESS
            if ( eepromConfigControl & ESC_EEPROM_SUPPORTED_READBYTES_MASK )
#else
            if ( nControl & ESC_EEPROM_SUPPORTED_READBYTES_MASK )
#endif
            {
                step = 4; /* we get 4 words with one read access */
            }
            else
            {
                step = 2; /* we get 2 words with one read access */
            }
        }

        /* first we have to lock the EEPROM access that we will not be interrupted by the master
        by setting register 0x501.0 */
#if ESC_32BIT_ACCESS
        /*Clear access register(0x0501)*/
        eepromConfigControl &= 0xFFFF00FF;

        /*Set access rights to PDI*/
        eepromConfigControl |= ESC_EEPROM_LOCKED_BY_PDI_MASK;

#if BIG_ENDIAN_FORMAT
        eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
        HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
#if BIG_ENDIAN_FORMAT
        eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
#else
        nConfig = ESC_EEPROM_LOCKED_BY_PDI_MASK;
#if BIG_ENDIAN_FORMAT
        nConfig = SWAPWORD(nConfig);
#endif
        HW_EscWriteWord(nConfig,ESC_EEPROM_CONFIG_OFFSET);
#endif
        for (i = 0; i < wordsize;)
        {
            /* we have to set the start address in register 0x504-0x507 */
#if BIG_ENDIAN_FORMAT
            UINT32 d = SWAPDWORD(wordaddress);
            HW_EscWrite((MEM_ADDR *) &d, ESC_EEPROM_ADDRESS_OFFSET, 4);
#else
#if ESC_32BIT_ACCESS
            HW_EscWriteDWord(wordaddress, ESC_EEPROM_ADDRESS_OFFSET);
#else
            HW_EscWrite((MEM_ADDR *) &wordaddress, ESC_EEPROM_ADDRESS_OFFSET, 4);
#endif
#endif

            if ( access == ESC_RD )
            {
                /* read access, we start the reading by setting 0x502.8
                (will be reset automatically when reading is finished) */
#if ESC_32BIT_ACCESS
                eepromConfigControl &= ESC_EEPROM_CONFIG_MASK;
                eepromConfigControl |= ESC_EEPROM_CMD_READ_MASK;

#if BIG_ENDIAN_FORMAT
                eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
                HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
#else
                nControl = ESC_EEPROM_CMD_READ_MASK;
#if BIG_ENDIAN_FORMAT
                nControl = SWAPWORD(nControl);
#endif
                HW_EscWriteWord(nControl,ESC_EEPROM_CONTROL_OFFSET);
#endif
            }
            else
            {
                /* write access, we write the data in register 0x508-0x509 */
#if ESC_32BIT_ACCESS
                HW_EscWriteDWord(pData[i],ESC_EEPROM_DATA_OFFSET);

                /* we start the writing by setting 0x502.9
                (will be reset automatically when writing is finished) */
                eepromConfigControl &= ESC_EEPROM_CONFIG_MASK;
                eepromConfigControl |= ESC_EEPROM_CMD_WRITE_MASK;

#if BIG_ENDIAN_FORMAT
                eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
                HW_EscWriteDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);
#else
                HW_EscWriteWord(pData[i],ESC_EEPROM_DATA_OFFSET);

                /* we start the writing by setting 0x502.9
                (will be reset automatically when writing is finished) */

                nControl = SWAPWORD(ESC_EEPROM_CMD_WRITE_MASK);

                HW_EscWriteWord(nControl, ESC_EEPROM_CONTROL_OFFSET);
#endif
            }

            do
            {

                /*Wait 100 cycles before reading EEPROM status*/
                u8TimeOut = 100;
                while (u8TimeOut > 0)
                {
                    u8TimeOut--;
                }
#if ESC_32BIT_ACCESS
                HW_EscReadDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);

#if BIG_ENDIAN_FORMAT
                eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
            }
            while ( eepromConfigControl & (ESC_EEPROM_BUSY_MASK));
#else
                HW_EscReadWord(nControl, ESC_EEPROM_CONTROL_OFFSET);

#if BIG_ENDIAN_FORMAT
                nControl = SWAPWORD(nControl);
#endif
            }
            while ( nControl & (ESC_EEPROM_BUSY_MASK));
#endif

            /* we have to check if the access was without errors */
#if ESC_32BIT_ACCESS
            HW_EscReadDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);

#if BIG_ENDIAN_FORMAT
            eepromConfigControl = SWAPDWORD(eepromConfigControl);
#endif
            if ( eepromConfigControl & ESC_EEPROM_ERROR_MASK )
            {
                if(!(eepromConfigControl & ESC_EEPROM_ERROR_CMD_ACK) && (RetryCnt != 0))
                {
                    /* Only abort if non Acknowledge Error occurs
                       In case of an Acknowledge Error the operation should be repeated*/
                    u16RetErr =  ALSTATUSCODE_EE_ERROR;
                    break;
                }
            }
#else
            HW_EscReadWord(nControl, ESC_EEPROM_CONTROL_OFFSET);

#if BIG_ENDIAN_FORMAT
            nControl = SWAPWORD(nControl);
#endif
            if ( nControl & ESC_EEPROM_ERROR_MASK )
            {
                if(!(nControl & ESC_EEPROM_ERROR_CMD_ACK) && (RetryCnt != 0))
                {
                    /* Only abort if non Acknowledge Error occurs
                       In case of an Acknowledge Error the operation should be repeated*/
                    u16RetErr =  ALSTATUSCODE_EE_ERROR;
                    break;
                }
            }
#endif
            else
            {
                if ( access == ESC_RD )
                {
                    UINT16 u16BytesToCopy = (step << 1);

                    /* read access, get the data from register 0x508-0x50B(0x50F)*/
                    if((u16WordOffset + step) > wordsize)
                    {
                        /*less than "step" words are left => copy only last required Bytes*/
                        u16BytesToCopy = (wordsize - u16WordOffset) << 1;
                    }

                    HW_EscRead((MEM_ADDR *) &pData[i], ESC_EEPROM_DATA_OFFSET, u16BytesToCopy);
                }
            }

#if ESC_32BIT_ACCESS
            if(!(eepromConfigControl & ESC_EEPROM_ERROR_MASK))
#else
            if(!(nControl & ESC_EEPROM_ERROR_MASK))
#endif
            {
                /* In case of Acknowledge Error repeat same operation, otherwise increment the address and proceed*/
                wordaddress += step;
                u16WordOffset +=step;
                RetryCnt = MAX_CMD_RETIRES;

                i += step;
            }
            else
            {
                RetryCnt --;
                if(RetryCnt > 0)
                {
/*ECATCHANGE_START(V5.12) ECAT8*/
                    /* Set wait (11ms / retries) until repeat EEPROM access */
                    INT32 i32TimeoutTicks = (INT32)(ECAT_TIMER_INC_P_MS * (11/ MAX_CMD_RETIRES));
/*ECATCHANGE_END(V5.12) ECAT8*/
                    UINT16 u16CurTimer = 0;
                    UINT16 u16LastTimer = HW_GetTimer();
                    UINT16 Delta = 0;

                    /* Start wait loop */
                    while(i32TimeoutTicks > 0)
                    {
                        u16CurTimer = HW_GetTimer();

                        if(u16LastTimer < u16CurTimer)
                        {
/*ECATCHANGE_START(V5.12) ECAT8*/
                            Delta = (u16CurTimer - u16LastTimer);
/*ECATCHANGE_END(V5.12) ECAT8*/
                        }
                        else
                        {
                            /* 16bit overrun*/
                            Delta = (0xFFFF - u16LastTimer) + u16CurTimer;
                        }
                        i32TimeoutTicks = i32TimeoutTicks - Delta;

                        u16LastTimer = u16CurTimer;
                    }
                }
                else
                {
                    /* Abort EEPROM access if max retires are reached*/
                    u16RetErr =  ALSTATUSCODE_EE_ERROR;
                    break;
                }
            }
        } //for-loop over all data
    } // if EEPROM access is assigned to PDI
    else
    {
        u16RetErr = ALSTATUSCODE_EE_NOACCESS;
    }

    /* clear EEPROM control register 0x500 */
#if ESC_32BIT_ACCESS
    eepromConfigControl = 0;

    HW_EscWriteDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);
#else
    nConfig = 0;

    HW_EscWriteWord(nConfig, ESC_EEPROM_CONFIG_OFFSET);
#endif
    return u16RetErr;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     success : ALSTATUSCODE_WAITFORCOLDSTART
            failed  : EEPROM ERRROR Code

 \brief        This function recalculates the EEPROM CRC and writes the updated value to EEPROM.
            After writing the ESC Config Area a device restart is required!
 *////////////////////////////////////////////////////////////////////////////////////////
UINT16 ESC_EepromWriteCRC(void)
{
    UINT16 u16Return = ALSTATUSCODE_UNSPECIFIEDERROR;
    UINT16 EscCfgData[8];
    UINT16 u16Crc = 0x00FF;
    UINT16 i,j;

    
    u16Return = ESC_EepromAccess(0,7,(UINT16 *)EscCfgData,ESC_RD);
    if(u16Return == 0)
    {
        UINT8 *pData = (UINT8 *)EscCfgData;

        for(i = 0; i < 14; i++ )
        {
            u16Crc ^= pData[i];

            for(j=0; j<8; j++ )
            {
                if( u16Crc & 0x80 )
                {
                    u16Crc = (u16Crc<<1) ^ 0x07;
                }
                else
                {
                    u16Crc <<= 1;
                }
            }
        }

        /*only low Byte shall be written*/
        u16Crc &= 0x00FF;

        /*write new calculated Crc to Esc Config area*/
        u16Return = ESC_EepromAccess(7,1,&u16Crc,ESC_WR);
        if(u16Return == 0)
        {
            u16Return =  ALSTATUSCODE_WAITFORCOLDSTART;
        }
    }

    return u16Return;
}
#endif //ESC_EEPROM_ACCESS_SUPPORT

#if ESC_EEPROM_EMULATION
void EEPROM_CommandHandler(void)
{
   UINT16 Result = 0;
#if ESC_32BIT_ACCESS
    UINT32 EEPROMReg = 0; //Regvalue 0x500 - 0x5003
#else
    UINT16 EEPROMReg = 0; //Regvalue 0x502 - 0x5003
#endif

#if ESC_32BIT_ACCESS
    HW_EscReadDWord(EEPROMReg,ESC_EEPROM_CONFIG_OFFSET);
    EEPROMReg = SWAPDWORD(EEPROMReg);
#else
    HW_EscReadWord(EEPROMReg,ESC_EEPROM_CONTROL_OFFSET);
    EEPROMReg = SWAPWORD(EEPROMReg);
#endif

    if (EEPROMReg & ESC_EEPROM_BUSY_MASK) 
    {
        UINT32 cmd = EEPROMReg  & ESC_EEPROM_CMD_MASK;
        UINT32 addr;
        
        
        HW_EscReadDWord(addr,ESC_EEPROM_ADDRESS_OFFSET);
        addr = SWAPDWORD(addr);

/*ECATCHANGE_START(V5.12) EEPROM1*/
        //Clear error bits (except the eeprom crc error)
        //EEPROMReg &= ~(ESC_EEPROM_ERROR_MASK & ~ESC_EEPROM_ERROR_CRC);
/*ECATCHANGE_END(V5.12) EEPROM1*/
        switch (cmd) {
        case 0x00: //IDLE
            //Clear error bits (except the eeprom crc error)
            EEPROMReg &= ~(ESC_EEPROM_ERROR_MASK & ~ESC_EEPROM_ERROR_CRC);
            break;
        case ESC_EEPROM_CMD_READ_MASK:
        {
           if (pAPPL_EEPROM_Read != NULL)
           {
              Result = pAPPL_EEPROM_Read(addr);
              if (Result > 0)
              {
                 //Set Error
                 EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
              }
           }
           else
           {

              if (addr <= ESC_EEPROM_SIZE && (pEEPROM != NULL))
              {
                 UINT16 *pData = (UINT16 *)pEEPROM;
/*ECATCHANGE_START(V5.12) EEPROM1*/
                 HW_EscWrite((MEM_ADDR *)&pData[(addr)], ESC_EEPROM_DATA_OFFSET, EepromReadSize);
                //Clear error bits (except the eeprom crc error)
                EEPROMReg &= ~(ESC_EEPROM_ERROR_MASK & ~ESC_EEPROM_ERROR_CRC);
/*ECATCHANGE_END(V5.12) EEPROM1*/
              }
              else
              {
                 //Set Error
                 EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
              }
           }
        }
        break;
        case ESC_EEPROM_CMD_WRITE_MASK:
            {
                 if (pAPPL_EEPROM_Write != NULL)
                 {
                    Result = pAPPL_EEPROM_Write(addr);
                    if (Result > 0)
                    {
                       //Set Error
                       EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                    }
                 }
                 else
                 {

                    if (addr <= ESC_EEPROM_SIZE && (pEEPROM != NULL) && (EEPROMReg & 1))
                    {
                       UINT16 *pData = (UINT16 *)pEEPROM;
                       HW_EscRead((MEM_ADDR *)&pData[(addr)], ESC_EEPROM_DATA_OFFSET, EEPROM_WRITE_SIZE);
                       //Clear error bits
                       EEPROMReg &= ~(ESC_EEPROM_ERROR_MASK);
                    }
                    else
                    {
                       //Set Error
                       EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                    }
                 }

/*ECATCHANGE_START(V5.12) EEPROM3*/
                 /* init the Store EEPROM timeout value (will be reset after the timer has expired)*/
                 u16EepromStoreTimeoutValue = 1000;
/*ECATCHANGE_END(V5.12) EEPROM3*/

            }
            break;
        case ESC_EEPROM_CMD_RELOAD_MASK:
/* ECATCHANGE_START(V5.12) EEPROM2*/
            //if (EepromReadSize == 8)
            if (0) // For TI EtherCAT slaves the below code doesn't work for reload
            {
                /* "full" EEPROM emulation ( 8Bytes EEPROM data) */
                do
                {
                    if (addr > ESC_EEPROM_SIZE)
                    {
                        //Set Error
                        EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                        break;
                    }
                    else
                    {
                        if (pAPPL_EEPROM_Read != NULL)
                        {
                            Result = pAPPL_EEPROM_Read(addr);
                            if (Result > 0)
                            {
                                //Set Error
                                EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                                break;
                            }
                        }
                        else
                        {
                            if (addr <= ESC_EEPROM_SIZE && (pEEPROM != NULL))
                            {
                                UINT16 *pData = (UINT16 *)pEEPROM;
                                HW_EscWrite((MEM_ADDR *)&pData[(addr)], ESC_EEPROM_DATA_OFFSET, EepromReadSize);
                            }
                            else
                            {
                                //Set Error
                                EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                                break;
                            }
                        }

                        //Clear error bits
                        EEPROMReg &= ~(ESC_EEPROM_ERROR_MASK);

                        //ack current reload segment
#if ESC_32BIT_ACCESS
                        UINT32 TmpData = SWAPDWORD(EEPROMReg);
                        HW_EscWriteDWord(TmpData, ESC_EEPROM_CONFIG_OFFSET);
#else
                        UINT16 TmpData = SWAPWORD(EEPROMReg);
                        HW_EscWriteWord(TmpData, ESC_EEPROM_CONTROL_OFFSET);
#endif  

                        // read EEPROM control (to check if the reload is still pending)
#if ESC_32BIT_ACCESS
                        HW_EscReadDWord(EEPROMReg, ESC_EEPROM_CONFIG_OFFSET);
                        EEPROMReg = SWAPDWORD(EEPROMReg);
#else
                        HW_EscReadWord(EEPROMReg, ESC_EEPROM_CONTROL_OFFSET);
                        EEPROMReg = SWAPWORD(EEPROMReg);
#endif
                        cmd = EEPROMReg  & ESC_EEPROM_CMD_MASK;

                        HW_EscReadDWord(addr, ESC_EEPROM_ADDRESS_OFFSET);
                        addr = SWAPDWORD(addr);
                    }


                } while (cmd == ESC_EEPROM_CMD_RELOAD_MASK);
            }
            else
            {
                /* The case is only called if the "small" EEPROM emulation (4 read bytes) is configured*/
    /* ECATCHANGE_END(V5.12) EEPROM2*/

    /* ECATCHANGE_START(V5.12) EEPROM2*/
                if (pAPPL_EEPROM_Reload != NULL)
                {
                    Result = pAPPL_EEPROM_Reload();
                    if (Result > 0)
                    {
                        //Set Error
                        EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
                    }
                    else
                    {
                        EEPROMReg &= ~ESC_EEPROM_ERROR_CRC;
                    }
                }
                else
                {

                    Result = HW_EepromReload();
                    if (Result != 0 || (pEEPROM == NULL))
                    {
                        EEPROMReg |= ESC_EEPROM_ERROR_CRC;
                    }
                    else
                    {
                        //copy the configured station alias
#if ESC_32BIT_ACCESS
                        HW_EscWriteDWord(((UINT32 *)pEEPROM)[0x4], ESC_EEPROM_DATA_OFFSET);
#else
                        HW_EscWriteWord(((UINT16 *)pEEPROM)[0x4], ESC_EEPROM_DATA_OFFSET);
#endif

                        EEPROMReg &= ~ESC_EEPROM_ERROR_CRC;
                    }
/* ECATCHANGE_END(V5.12) EEPROM2*/
                }
            }
         break;
        default:
            EEPROMReg |= ESC_EEPROM_ERROR_CMD_ACK;
            break;
        }

/*ECATCHANGE_START(V5.12) EEPROM3*/
        /* reset the store eeprom timeout counter*/
        u16EepromStoreTimeoutCounter = 0;
/*ECATCHANGE_END(V5.12) EEPROM3*/

        {
#if ESC_32BIT_ACCESS
        UINT32 TmpData = SWAPDWORD(EEPROMReg);
        HW_EscWriteDWord(TmpData,ESC_EEPROM_CONFIG_OFFSET);
#else
        UINT16 TmpData = SWAPWORD(EEPROMReg);
        HW_EscWriteWord(TmpData,ESC_EEPROM_CONTROL_OFFSET);
#endif  
        }
    }
}
#endif


/** @} */

