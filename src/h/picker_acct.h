/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Picker accountability definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
#ifndef PICKER_ACCT_H
#define PICKER_ACCT_H

static char picker_acct_h[] = "%Z% %M% %I% (%G% - %U%)";

#include <fcntl.h>

/*
** flags for picker_order.p_order_status once p_completion_date is non-zero
*/
#define PA_CANCELED    1
#define PA_PICKED      2
#define PA_CURRENT     4
#define PA_CUMULATIVE  8

/*
** the file where system configuration time is kept for picker accountability
*/
#define PA_DATAFILE    "dat/files/pa_data"

/*
** this file consists of one instance of the following structure
*/
typedef struct {
  long time_at_zero_current;    /* time when current counts last zeroed      */
  long time_at_zero_cumulative; /* time when current/cumulative counts zeroed*/
  long current_config_time;     /* elapsed time while configured for current */
  long cumulative_config_time;  /* elapsed time while config.d for cumulative*/
  long last_start_time;         /* time clock last started when sys config.d */
} PA_FILESTRUCT;

/*
** the following macros make access easier to this data file, and they
** all return a non-zero value (ie TRUE) if the action was done successfully.
** In each case 'fd' is an integer file descriptor, and 'data' is the
** address of a PA_FILESTRUCT.
*/

#define open_pa_file(fd)  \
  (((fd) = open(PA_DATAFILE, O_RDWR | O_CREAT, 0666)) >= 0)

#define close_pa_file(fd) \
  (close(fd) == 0)

#define read_pa_file(fd, data)                        \
  ((lseek((fd), 0L, 0) == 0) &&                       \
   (memset((char *)(data), 0, sizeof(PA_FILESTRUCT)), \
    (read((fd), (char *)(data),sizeof(PA_FILESTRUCT))  >= 0)))

#define write_pa_file(fd, data)  \
  ((lseek((fd), 0L, 0) == 0) &&  \
   (write((fd),(char *)(data),sizeof(PA_FILESTRUCT)) == sizeof(PA_FILESTRUCT)))

#endif

/* end of picker_acct.h	*/
