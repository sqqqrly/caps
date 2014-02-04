/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Change Logon Name in /etc/passwd
 *
 *  return 0;       completed.
 *  return -1;      old invalid.
 *  return -2;      new invalid.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  1/26/94    |  tjt  Original inpletmentation.
 *-------------------------------------------------------------------------*/
static char change_logon_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
extern char *memchr();

#define NAME "/etc/passwd"

change_logon(old, new)
register char *old;                       /* old user name                   */
register char *new;                       /* new user name                   */
{
  register long len, no, nn;
  register char *p, *q, *r, *save;
  char buf[2000];
  FILE *fd;
  
  no = strlen(old);
  if (no < 3 || no > 8) return -1;
  
  nn = strlen(new);
  if (nn < 3 || nn > 8) return -2;
  
  fd = fopen(NAME, "r");
  if (fd == 0) krash("change_logon", "open for read", 1);
  
  fseek(fd, 0, 2);
  len = ftell(fd);
  if (len < 32 || len > 2000) krash("change_logon", "invalid length", 1);
  
  fseek(fd, 0, 0);
  if (fread(buf, 1, len, fd) != len) krash("change_logon", "read error", 1);
  fclose(fd);

  p = buf;
  q = buf + len;
  save = 0;
  
  while (p < q)
  {
    r = memchr(p, ':', q - p);             /* find first colon after name    */
    if (!r) r = p;                         /* make it a null length field    */
    
    if (no == r - p)                       /* find old logon name            */
    {
      if (memcmp(p, old, r - p) == 0) save = p;
    }
    if (nn == r - p)                       /* check dupicate logon name      */
    {
      if (memcmp(p, new, r - p) == 0) return -2;  
    }
    p = memchr(p, '\n', q - p);
    if (!p) break;
    p++;
  }
  if (!save) return -1;                    /* not found                      */
    
  fd = fopen(NAME, "r+");
  if (fd == 0) krash("change_logon", "open for update", 1);
  
  fseek(fd, save - buf, 0);
  if (fwrite(new, 1, nn, fd) != nn) krash("change_logon", "write error 1", 1);
  save += no;
  if (fwrite(save, 1, q - save, fd) != q - save)
  {
    krash("change_logon", "write error 2", 1);
  }
  fclose(fd);
  return 0;
}


/* end of change_logon.c */
