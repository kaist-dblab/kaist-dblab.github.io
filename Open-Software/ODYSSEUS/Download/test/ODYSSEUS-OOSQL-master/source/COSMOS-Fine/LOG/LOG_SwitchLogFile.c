/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 1990-2016, KAIST                                          */
/*    All rights reserved.                                                    */
/*                                                                            */
/*    Redistribution and use in source and binary forms, with or without      */
/*    modification, are permitted provided that the following conditions      */
/*    are met:                                                                */
/*                                                                            */
/*    1. Redistributions of source code must retain the above copyright       */
/*       notice, this list of conditions and the following disclaimer.        */
/*                                                                            */
/*    2. Redistributions in binary form must reproduce the above copyright    */
/*       notice, this list of conditions and the following disclaimer in      */
/*       the documentation and/or other materials provided with the           */
/*       distribution.                                                        */
/*                                                                            */
/*    3. Neither the name of the copyright holder nor the names of its        */
/*       contributors may be used to endorse or promote products derived      */
/*       from this software without specific prior written permission.        */
/*                                                                            */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          */
/*    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,    */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT      */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN       */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         */
/*    POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/COSMOS General-Purpose Large-Scale Object Storage System --    */
/*    Fine-Granule Locking Version                                            */
/*    Version 3.0                                                             */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: odysseus.oosql@gmail.com                                        */
/*                                                                            */
/*    Bibliography:                                                           */
/*    [1] Whang, K., Lee, J., Lee, M., Han, W., Kim, M., and Kim, J., "DB-IR  */
/*        Integration Using Tight-Coupling in the Odysseus DBMS," World Wide  */
/*        Web, Vol. 18, No. 3, pp. 491-520, May 2015.                         */
/*    [2] Whang, K., Lee, M., Lee, J., Kim, M., and Han, W., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with IR Features," In Proc. */
/*        IEEE 21st Int'l Conf. on Data Engineering (ICDE), pp. 1104-1105     */
/*        (demo), Tokyo, Japan, April 5-8, 2005. This paper received the Best */
/*        Demonstration Award.                                                */
/*    [3] Whang, K., Park, B., Han, W., and Lee, Y., "An Inverted Index       */
/*        Storage Structure Using Subindexes and Large Objects for Tight      */
/*        Coupling of Information Retrieval with Database Management          */
/*        Systems," U.S. Patent No.6,349,308 (2002) (Appl. No. 09/250,487     */
/*        (1999)).                                                            */
/*    [4] Whang, K., Lee, J., Kim, M., Lee, M., Lee, K., Han, W., and Kim,    */
/*        J., "Tightly-Coupled Spatial Database Features in the               */
/*        Odysseus/OpenGIS DBMS for High-Performance," GeoInformatica,        */
/*        Vol. 14, No. 4, pp. 425-446, Oct. 2010.                             */
/*    [5] Whang, K., Lee, J., Kim, M., Lee, M., and Lee, K., "Odysseus: a     */
/*        High-Performance ORDBMS Tightly-Coupled with Spatial Database       */
/*        Features," In Proc. 23rd IEEE Int'l Conf. on Data Engineering       */
/*        (ICDE), pp. 1493-1494 (demo), Istanbul, Turkey, Apr. 16-20, 2007.   */
/*                                                                            */
/******************************************************************************/
/*
 * Module: LOG_SwitchLogFile.c
 *
 * Description:
 *  Switch the log file.
 *
 * Exports:
 *  Four LOG_SwithLogFile(XactTableEntry_T*)
 */

#include <assert.h>
#include "common.h"
#include "error.h"
#include "trace.h"
#include "TM.h"
#include "LOG.h"
#include "RM.h"
#include "perProcessDS.h"
#include "perThreadDS.h"

/*
 * Function: Four LOG_SwithLogFile(XactTableEntry_T*)
 *
 * Description:
 *  Switch the log file.
 *
 * Returns:
 *  error codes
 */
Four LOG_SwitchLogFile(
    Four 	handle,
    Four 	logRecLen)            /* IN log record length */
{
    Four  	e;                    /* error code */
    Lsn_T 	minOnlineLsn;         /* LSN to keep online */
    Lsn_T 	nextLsn;


    TR_PRINT(handle, TR_LOG, TR1, ("LOG_SwitchLogFile()"));


    /* get latch */
    e = SHM_getLatch(handle, &LOG_LATCH4LOGFILESWITCH, procIndex, M_EXCLUSIVE, M_UNCONDITIONAL, NULL);
    if (e < eNOERROR) ERR(handle, e);

    /* To get minimum online LSN (see below), Set the minOnlineLsn with the minimum of first LSN of active transactions. */
    /* This call should come here before requesting log related latches to avoid a deadlock among latches. */
    e = TM_XT_GetMinFirstLsn(handle, &minOnlineLsn);
    if (e < eNOERROR) ERR(handle, e);

    /* get latch */
    e = SHM_getLatch(handle, &LOG_LATCH4HEAD, procIndex, M_EXCLUSIVE, M_UNCONDITIONAL, NULL);
    if (e < eNOERROR) ERR(handle, e);

    if (LOG_LOGMASTER.numBytesRemained >= logRecLen) {
        e = SHM_releaseLatch(handle, &LOG_LATCH4HEAD, procIndex);
        if (e < eNOERROR) ERR(handle, e);

        e = SHM_releaseLatch(handle, &LOG_LATCH4LOGFILESWITCH, procIndex);
        if (e < eNOERROR) ERR(handle, e);

        return(eNOERROR);
    }

    e = SHM_getLatch(handle, &LOG_LATCH4TAIL, procIndex, M_EXCLUSIVE, M_UNCONDITIONAL, NULL);
    if (e < eNOERROR) {
	(Four) SHM_releaseLatch(handle, &LOG_LATCH4HEAD, procIndex);
	ERR(handle, e);
    }

    /*
     * Flush all log buffers for the current log file.
     */
    e = log_FlushLogBuffers(handle, LOG_LBI_HEAD, FALSE);
    if (e < eNOERROR) ERR(handle, e);

    LOG_LBI_HEAD = LOG_LBI_TAIL = 0;

    /* release latch */
    e = SHM_releaseLatch(handle, &LOG_LATCH4TAIL, procIndex);
    if (e < eNOERROR) ERR(handle, e);


    /*
     * Get the low water mark, minOnlineLsn.
     */
    /* Compare with the checkpoint LSN. */
    if (!IS_NIL_LSN(LOG_LOGMASTER.checkpointLsn) && LSN_CMP_LT(LOG_LOGMASTER.checkpointLsn, minOnlineLsn))
        minOnlineLsn = LOG_LOGMASTER.checkpointLsn;

    /* Compare with the recovery LSN of dirty buffer pages. */


    /*
     * Check whether the next log file is available.
     */
    assert((minOnlineLsn.wrapCount + LOG_LOGMASTER.nLogFiles - 1) >= LOG_LOGMASTER.headWrapCount);
    if ((minOnlineLsn.wrapCount + LOG_LOGMASTER.nLogFiles - 1) == LOG_LOGMASTER.headWrapCount)
        ERR(handle, eNOLOGSPACE_LOG);


    /*
     * Set next log record LSN.
     */
    nextLsn.wrapCount = LOG_LOGMASTER.headWrapCount + 1;
    nextLsn.offset = 0;
    e = LOG_SetNextLogRecordLsn(handle, &nextLsn);
    if (e < eNOERROR) ERR(handle, e);

    /* release latch */
    /* We should release the latch befor checkpointing to avoid a deadlock among latches. */
    e = SHM_releaseLatch(handle, &LOG_LATCH4HEAD, procIndex);
    if (e < eNOERROR) ERR(handle, e);


    /*
     * perform the checkpointing for keeping a stable status
     */
    e = RM_Checkpoint(handle);
    if (e < eNOERROR) ERR(handle, e);

    /* release latch */
    e = SHM_releaseLatch(handle, &LOG_LATCH4LOGFILESWITCH, procIndex);
    if (e < eNOERROR) ERR(handle, e);


    return(eNOERROR);

} /* LOG_WriteLogRecord() */
