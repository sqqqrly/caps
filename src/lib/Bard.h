/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Bard or Informix Header File
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/06/95   |  tjt  Revised for INFORMIX.
 *  04/18/96   |  tjt  Revised for Bard & Informix combination.
 *-------------------------------------------------------------------------*/
#ifndef BARDH
#define BARDH

static char Bard_h[] = "%Z% %M% %I% (%G% - %U%)";

#ifdef INFORMIX
#include "informix_defines.h"
#endif

#ifdef BARD
#include "bard_defines.h"
#endif

#include "oracle_defines.h"
#endif

/* end of Bard.h */
