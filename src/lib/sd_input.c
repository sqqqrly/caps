/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Get input field.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/07/93   |  tjt  Rewrite of screen driver.
 *  07/11/95   |  tjt  Add krash on bad field length.
 *-------------------------------------------------------------------------*/
static char sd_input_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"
#include "sd.h"
#include "message_types.h"

unsigned char sd_input(f, r, rm, buf, cbuf)
register struct fld_parms *f;             /* field parameters                */
register short r, *rm;                    /* row modifiers                   */
register unsigned char *buf, *cbuf;       /* buffers                         */
{
  unsigned char mbuf[84];
  register long length;
  
  if (sd_server <= 0) return krash("sd_input", "sd not open", 1);

  length = *(f->length);

  if (length < 1 || length > 80) krash("sd_input", "zero length", 1);

  mbuf[0] = sd_row = f->irow + r - 1;     /* build request message           */
  mbuf[1] = sd_col = f->icol - 1;
  mbuf[2] = f->type;
  mbuf[3] = sd_echo_flag;                 /* usually FILL byte               */

  memcpy(mbuf + 4, buf, length);
   
  message_wait(sd_server, InputFieldRequest, mbuf, length + 4,
    InputFieldEvent, SDWAIT);

  memcpy(buf, sd_buf, length);            /* return result field             */

  if (rm) *rm = r;                        /* return offset                   */

  return sd_buf[length];                  /* return terminator               */
}

/* end of sd_input.c */
