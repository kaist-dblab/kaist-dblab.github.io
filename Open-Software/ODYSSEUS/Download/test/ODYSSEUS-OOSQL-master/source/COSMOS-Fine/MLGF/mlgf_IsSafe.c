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
/******************************************************************************/
/*                                                                            */
/*    This module has been implemented based on "The Multilevel Grid File     */
/*    (MLGF) Version 4.0," which can be downloaded at                         */
/*    "http://dblab.kaist.ac.kr/Open-Software/MLGF/main.html".                */
/*                                                                            */
/******************************************************************************/

/*
 * Module: mlgf_IsSafe.c
 *
 * Description:
 * Check that the given page is safe.
 *
 * Exports:
 *
 */

#include <string.h>
#include "common.h"
#include "error.h"
#include "trace.h"
#include "Util.h"
#include "TM.h"
#include "RDsM.h"
#include "BfM.h"
#include "MLGF.h"
#include "perProcessDS.h"
#include "perThreadDS.h"



Boolean mlgf_IsSafeDir(
    Four 		handle,
    Buffer_ACC_CB 	*dirPage_BCBP,
    MLGF_KeyDesc  	*kdesc,
    Four 		mergedEntryNo)         	/* IN entry to be merged */
{
    Four 		e;			/* error code */
    Four 		entryLen;		/* the length of a directory entry */
    Four 		buddyEntryNo;		/* entry no of buddy entry of merged entry */
    Four 		buddyKey;		/* attribute no of buddy key */
    mlgf_DirectoryPage 	*dirPage;
    Boolean 		found;			/* TRUE if buddy entry is found */
    Boolean 		mergeFlag;          	/* TRUE if a merge occurs */
    Two  		tmpNEntries;
    Two  		tmpTheta;
    mlgf_DirectoryEntry *buddyEntry;  		/* entry for buddy page of merged page */
    mlgf_DirectoryEntry *mergedEntry; 		/* entry for merged page */
    mlgf_DirectoryEntry tmpMergedEntry;

    TR_PRINT(handle, TR_MLGF, TR1, ("mlgf_IsSafeDir(dirPage_BCBP=%P, kdesc=%P, mergedEntryNo=%ld)",
			    dirPage_BCBP, kdesc, mergedEntryNo));

    dirPage = (mlgf_DirectoryPage*)dirPage_BCBP->bufPagePtr;

    /* Calculate the length of a directory entry. */
    tmpNEntries = dirPage->hdr.nEntries;

    entryLen = MLGF_DIRENTRY_LENGTH(kdesc->nKeys);

    /* check that this directory page is safe when overflow */
    if(dirPage->hdr.nEntries >= MLGF_MAX_DIRENTRIES(kdesc->nKeys)){
	TR_PRINT(handle, TR_MLGF, TR1, ("Unsafe(Overflow): Entries: %ld, Max: %ld\n",
	       dirPage->hdr.nEntries, MLGF_MAX_DIRENTRIES(kdesc->nKeys)));
	return(FALSE);
    }

    /* if mergedEntryNo == MLGF_NOENTRY then there will be no underflow */
    if(mergedEntryNo == MLGF_NOENTRY){
	TR_PRINT(handle, TR_MLGF, TR1, ("Safe: No entry\n"));
	return(TRUE);
    }

    mergedEntry = MLGF_ITH_DIRENTRY(dirPage, mergedEntryNo, entryLen);

    mergeFlag = FALSE;          /* Intialize mergeFlag to FALSE. */

    tmpMergedEntry = *mergedEntry;

    tmpMergedEntry.theta = 0;

    /* check that this directory page is safe when underflow */
    while (tmpMergedEntry.theta < MLGF_DP_THRESHOLD) {
	TR_PRINT(handle, TR_MLGF, TR1, ("Theta: %ld\n", tmpMergedEntry.theta));

	/* Find the buddy entry. */
	found = FALSE;
	for (buddyEntryNo = 0; buddyEntryNo <= tmpNEntries; buddyEntryNo++) {

	    if (buddyEntryNo < 0 || buddyEntryNo >= dirPage->hdr.nEntries) continue;

	    buddyEntry = MLGF_ITH_DIRENTRY(dirPage, buddyEntryNo, entryLen);

	    if (mlgf_BuddyTest(handle, kdesc->nKeys, &tmpMergedEntry, buddyEntry, &buddyKey)) {
		found = TRUE;
		break;
	    }
	}

	/* if there is not buddy region, exit the loop. */
	if (!found) break;

	/* there is not enough space to merge two pages */
	if (tmpMergedEntry.theta + buddyEntry->theta + sizeof(Two) > PAGESIZE - MLGF_DP_FIXED)
	    break;

	tmpTheta = tmpMergedEntry.theta + buddyEntry->theta;

	TR_PRINT(handle, TR_MLGF, TR1, ("Theta: %ld, BuddyTheta: %ld, Sum: %ld\n", tmpMergedEntry.theta, buddyEntry->theta, tmpTheta));

	tmpMergedEntry = *buddyEntry;
	tmpMergedEntry.nValidBits[buddyKey] --;
	tmpMergedEntry.theta = tmpTheta;

	mergedEntryNo = buddyEntryNo;

	tmpNEntries--;
    }

    /* calculate that this page is safe */
    if(tmpNEntries * entryLen < MLGF_DP_THRESHOLD) return(FALSE);
    else return(TRUE);

}
