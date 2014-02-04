/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Build screen image from paramters.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/20/93   |  tjt  Added to mfc.
 *  04/20/94   |  tjt  Changed for UNIX date/time.
 *-------------------------------------------------------------------------*/
static char fix_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "co.h"

extern char *getenv();

/*-------------------------------------------------------------------------*
 *  %m  is month
 *  %d  is day
 *  %y  is year
 *  %w  is day of week
 *  %t  is time
 *  %n  is page number
 *  %sl is system name left
 *  %sc                centered
 *  %sr                right
 *  %cl is company name left
 *  %cc                 centered
 *  %cr                 right
 *  %l  is pickline numbrer
 *  %p  is pickline name
 *-------------------------------------------------------------------------*/
fix(string)
register char *string;
{
  char str[32];                           /* current time string             */
  long now;
  register long j, n;
  register char *p;

  time(&now);
  memcpy(str, ctime(&now), 24);           /* store time in str               */
  str[24] = 0;

  while(*string)                          /* until end of string             */
  {
    if (*string == '%')
    {
      switch(*(string+1))
      {
        case 'm':
         
          memcpy(string, str + 4, 3);
          break;

        case 'd':
         
          memcpy(string, str + 8, 2);
          break;

        case 'y':
         
          memcpy(string, str + 20, 4);
          break;

        case 'w':
         
          memcpy(string, str, 3);
          break;
                      
        case 't':

          memcpy(string, str + 11, 8);
          break;

        case 's':

          if(sp > 0)
          {
            if(*(string + 2) == 'l')
            {
              memcpy(string, sp->sp_name, strlen(sp->sp_name));
            }
            else if(*(string + 2) == 'c')
            {
              j = (sizeof(sp->sp_name) - strlen(sp->sp_name)) / 2;

              for(; j > 0; j--) *(string++) = 0x20;

              memcpy(string, sp->sp_name, strlen(sp->sp_name));
            }
            else if(*(string + 2) == 'r')
            {
              j = sizeof(sp->sp_name) - strlen(sp->sp_name);

              for(; j > 0; j--) *(string++) = 0x20;

              memcpy(string, sp->sp_name, strlen(sp->sp_name));
            }
          }
          break;
            
        case 'c':

          if (sp > 0)
          {
            if(*(string + 2) == 'l')
            {
              memcpy(string, sp->sp_company, strlen(sp->sp_company));
            }
            else if(*(string + 2) == 'c')
            {
              j = (sizeof(sp->sp_company) - strlen(sp->sp_company))/2;

              for (; j > 0; j--) *(string++) = 0x20;

              memcpy(string, sp->sp_company, strlen(sp->sp_company));
            }
            else if(*(string + 2) == 'r')
            {
              j = sizeof(sp->sp_company) - strlen(sp->sp_company);
              for (; j > 0; j--) *(string++) = 0x20;
                      
              memcpy(string, sp->sp_company, strlen(sp->sp_company));
            }
          }
          break;

        case 'l':

          memcpy(string, "  ", 2);
          p = getenv("PICKLINE");
          if (p) 
          {
            j = strlen(p);
            memcpy(string + 2 - j, p, j);
          }
          break;
          
        case 'p':

          memcpy(string, "        ", 8);

          if (co)
          {
            p = getenv("PICKLINE");
            if (p)
            {
              j = atol(p);
              if (pl[j - 1].pl_pl)
              {
                n = strlen(pl[j - 1].pl_name);
                if (n) memcpy(string, pl[j - 1].pl_name, n);
              }
            }
          }
          break;
        }
    }
    string++;
  }
  return 0;
}

/* end of fix.c */
