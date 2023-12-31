/*!
* \file FBTL_channel.h
*
* \brief
* FBTL channel access interface.
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-05-19
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

#if !(defined __FBTL_CHANNEL_H__)
#define __FBTL_CHANNEL_H__		1

#define FILEDUMPDP      0
#define FILESTATEDUMP   0

#include <FBTL_api.h>
#include <FBTL_queue.h>

#if (defined __cplusplus)
extern "C" {
#endif

#if ((defined FILEDUMPDP) && (FILEDUMPDP==1)) || ((defined FILESTATEDUMP) && (FILESTATEDUMP==1))
#include <string.h>
#include <stdio.h>

#if (defined FILEDUMPDP) && (1==FILEDUMPDP)
extern FILE* fileBeginDump(void);
extern void filePrint(FILE* pOutFd_p, void* sysLib, char* str);
extern void filePrintHead(FILE* pOutFd_p, void* pHead_p);
extern void fileDump(FILE* pOutFd_p, uint8_t* pData_p, uint32_t dataLen_p);
extern void fileNewLine(FILE* pOutFd_p);
extern void fileCloseDump(FILE* pOutFd_p);
#endif
#if (defined FILESTATEDUMP) && (1==FILESTATEDUMP)
extern void fileStateDump(void* sysLib, char* str, bool poll, uint32_t locState, uint32_t remState, uint32_t irqWords);
#endif
#endif

extern bool     FBTL_CHAN_isAcycWriteFree   (void*                  pFbtlHandle_p);
extern bool     FBTL_CHAN_isAcycReadAvail   (void*                  pFbtlHandle_p);
extern uint32_t FBTL_CHAN_writeAcyc         (void*                  pFbtlHandle_p
                                            ,void*                  pHeader_p
                                            ,uint8_t*               pData_p
                                            ,uint32_t               length_p);
extern uint32_t FBTL_CHAN_readAcyc          (void*                  pFbtlHandle_p);

#if (defined __cplusplus)
}
#endif

#endif /* __FBTL_CHANNEL_H__ */
