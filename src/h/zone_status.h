/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Zone Status Codes
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/05/95   |  tjt Added ZS_INACTIVE for manual feed zones.
 *-------------------------------------------------------------------------*/
#ifndef ZONE_STATUS_H
#define ZONE_STATUS_H

static char zone_status_h[] = "%Z% %M% %I% (%G% - %U%)";

#define ZS_COMPLETE    'F'
#define ZS_AHEAD       'A'
#define ZS_UNDERWAY    'U'
#define ZS_LOCKED      'X'
#define ZS_LATE        'L'
#define ZS_EARLY       'E'
#define ZS_WAITING     'W'
#define ZS_INACTIVE    'I'
#define ZS_OFFLINE     'O'

#define OS_NOPICKS     '-'
#define OS_PICKS       'P'
#define OS_AHEAD       'A'
#define OS_AHEAD_NP    'a'
#define OS_UW_NP       'u'
#define OS_UW          'U'
#define OS_LATE_NP     'l'
#define OS_LATE        'L'
#define OS_EARLY_NP    'e'
#define OS_EARLY       'E'
#define OS_COMPLETE    'C'

#endif


/* end of zone_status.h */
