/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Operator environmental parmeters.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  8/11/93    | tjt  Original implemtmentation.
 *-------------------------------------------------------------------------*/
#ifndef GETPARMS_H
#define GETPARMS_H

#include "co.h"

static char getparms_h[] = "%Z% %M% %I% (%G% - %U%)";

long SUPER_OP      = 0;
long DATA_OP       = 0;
long op_pl         = 0;
char op_legal[32]  = {0};
char op_printer[9] = {0};
long op_refresh    = 0;

#define LEGAL(x)         (memchr(op_legal, x, 32))
#define IS_ONE_PICKLINE  (coh->co_pl_config == 1)

#endif

/*  end of getparms.h */

