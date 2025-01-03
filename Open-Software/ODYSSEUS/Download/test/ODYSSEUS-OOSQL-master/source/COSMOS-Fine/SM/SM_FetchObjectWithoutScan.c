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
 * Module: SM_FetchObjectWithoutScan.c
 *
 * Description:
 *  Read the given object's contents.
 *
 * Exports:
 *  Four SM_FetchObjectWithoutScan(Four, Four, ObjectID*, Four, Four, char*)
 */


#include <assert.h>
#include "common.h"
#include "error.h"
#include "trace.h"
#include "latch.h"
#include "TM.h"
#include "LM.h"
#include "OM.h"
#include "BtM.h"
#include "SM.h"
#include "SHM.h"
#include "perProcessDS.h"
#include "perThreadDS.h"



/*@================================
 * SM_FetchObjectWithoutScan( )
 *================================*/
/*
 * Function: Four SM_FetchObjectWithoutScan(Four, Four, ObjectID*, Four, Four, char*)
 *
 * Description:
 *  Read the given object's contents.
 *
 * Returns:
 *  Error code
 *    eBADPARAMETER
 *    eBADCURSOR
 *    some errors caused by function calls
 */
Four SM_FetchObjectWithoutScan(
    Four handle,
    FileID    *fid,             /* IN file where the object is inserted */
    ObjectID *oid,		/* IN object to read */
    Four start,			/* IN starting offset of data to read */
    Four length,		/* IN amount of data to read */
    char *data,			/* OUT space to return the read data */
    LockParameter *fileLockup,  /* IN request file lock or not */
    LockParameter *objLockup)   /* IN request object lock or not */
{
    Four e, e1;			/* error number */ 
    DataFileInfo finfo;         /* data file info */
    Four v;			/* index for the used volume on the mount table */
    Four i;
    LockReply lockReply;
    LockMode oldMode;


    TR_PRINT(handle, TR_SM, TR1, ("SM_FetchObjectWithoutScan()"));

    /*@ check parameters. */
    if (fid == NULL) ERR(handle, eBADPARAMETER);

    /* find the given volume in the scan manager mount table */
    for (v = 0; v < MAXNUMOFVOLS; v++)
	if (SM_MOUNTTABLE[v].volId == fid->volNo) break; /* found */

    if (v == MAXNUMOFVOLS) ERR(handle, eNOTMOUNTEDVOLUME_SM);

    if (length < 0 && length != REMAINDER) ERR(handle, eBADPARAMETER);

    if (length > 0 && data == NULL) ERR(handle, eBADPARAMETER);

    if(SM_NEED_AUTO_ACTION(handle)) {
        e = LM_beginAction(handle, &MY_XACTID(handle), AUTO_ACTION);
        if(e < eNOERROR) ERR(handle, e);
    }

    /* Check if the file is a temporary file. */
    finfo.fid = *fid;
    finfo.tmpFileFlag = FALSE; /* initialize */
    for (i = 0; i < SM_NUM_OF_ENTRIES_OF_ST_FOR_TMP_FILES(handle); i++) 
	if (!SM_IS_UNUSED_ENTRY_OF_ST_FOR_TMP_FILES(SM_ST_FOR_TMP_FILES(handle)[i]) &&
	    EQUAL_FILEID(*fid, SM_ST_FOR_TMP_FILES(handle)[i].data.fid)) {

	    finfo.tmpFileFlag = TRUE;
	    finfo.catalog.entry = &(SM_ST_FOR_TMP_FILES(handle)[i]);
	    break;
	}

    if (finfo.tmpFileFlag == FALSE) {
        if (fileLockup) {

            if(fileLockup->duration != L_COMMIT) ERR(handle, eCOMMITDURATIONLOCKREQUIRED_SM);

            /* lock on the data file */
            e = LM_getFileLock(handle,  &MY_XACTID(handle), fid, fileLockup->mode, fileLockup->duration,
                                L_UNCONDITIONAL, &lockReply, &oldMode);
            if ( e < eNOERROR ) ERR(handle, e);

            if (lockReply == LR_DEADLOCK) ERR(handle, eDEADLOCK);
        }

	/* Get catalog entry of */
	/* e = sm_GetCatalogEntryFromDataFileId(handle, v, fid, &(finfo.catalog.oid)); */
	/* if (e < eNOERROR) ERR(handle, e); */
    }

#ifndef NDEBUG
    if (objLockup) {
        assert(objLockup->duration == L_COMMIT);
    }
#endif /* NDEBUG */

    if(length != 0){
	/* Read the object. */
	e = OM_ReadObject(handle, MY_XACT_TABLE_ENTRY(handle), fid, oid, start,
			  length, data, objLockup); 
	if (e < eNOERROR) ERR(handle, e);
    }
    else e = 0;

    if(ACTION_ON(handle)){  
	e1 = LM_endAction(handle, &MY_XACTID(handle), AUTO_ACTION); 
        if(e1 < eNOERROR) ERR(handle, e1);
    }

    return(e);

} /* SM_FetchObjectWithoutScan() */


