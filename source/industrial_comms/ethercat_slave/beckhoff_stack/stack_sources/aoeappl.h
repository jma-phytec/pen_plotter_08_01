/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
 * \addtogroup AoE ADS over EtherCAT
 * @{
 */

/**
\file	aoeappl.h
\author EthercatSSC@beckhoff.com

\version 5.11

<br>Changes to version V5.01:<br>
V5.11 ECAT10: change PROTO handling to prevent compiler errors<br>
<br>Changes to version - :<br>
V5.01 : Start file change log
 */

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecataoe.h"


#ifndef _AOEAPPL_H_
#define _AOEAPPL_H_

/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/


#endif //_AOEAPPL_H_

#if defined(_AOEAPPL_) && (_AOEAPPL_ == 1)
    #define PROTO
#else
    #define PROTO extern
#endif

/*-----------------------------------------------------------------------------------------
------
------    Global Variables
------
-----------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------------
------
------    Global Functions
------
-----------------------------------------------------------------------------------------*/

PROTO UINT16    AOEAPPL_AmsInd(AmsCmd MBXMEM *pCmd);

#undef PROTO
/** @}*/
