/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Save All Current Hardware Maps
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/07/94   |  tjt  Original implementation.
 *  08/06/94   |  tjt  Ignore simulated.
 *  02/24/95   |  tjt  Ignore disabled.
 *  05/26/98   |  tjt  Revise for full function.
 *-------------------------------------------------------------------------*/
static char save_hw_maps_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "co.h"
#include "ss.h"

unsigned long mask = IsFullFunction | IsBasicFunction | IsTotalFunction;

FILE *DF;

main(argc, argv)
long argc;
char **argv;
{
  register long k, size;
  register char *p;
  char md_name[40], sd_name[40];
  FILE *md, *sd;

  putenv("_=save_hw_maps");
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  DF= fopen("debug/save_maps", "w");
  fprintf(DF, "save_hw_maps()\n");
#endif
  
  ss_open();
  co_open();
  
  for (k = 0; k < coh->co_ports; k++)
  {
    if (!(po[k].po_flags & mask)) continue;
  
    if (po[k].po_disabled == 'y') continue;
    
    sprintf(md_name, "%s.%s", hw_name, basename(po[k].po_name));
    sprintf(sd_name, "%s.%s", hw_save_name, basename(po[k].po_name));

#ifdef DEBUG
    fprintf(DF, "md_name=%s  sd_name=%s\n", md_name, sd_name);
#endif
    
    md = fopen(md_name, "r");
    if (md == 0) krash("main - open", md_name, 1);
    
    sd = fopen(sd_name, "w");
    if (sd == 0) krash("main - open", sd_name, 1);
    
    fseek(md, 0, 2);
    size = ftell(md);
    fseek(md, 0, 0);
    
    p = (char *)malloc(size);
    if (!p) krash("main", "allocation error",1 );
      
    if (fread(p, 1, size, md) != size)  krash("main", "read error",1 );
    if (fwrite(p, 1, size, sd) != size) krash("main", "write error", 1); 
    
   free(p);
   fclose(md);
   fclose(sd);
  }
  co_close();
  ss_close();
  exit(0);
}

/* end of save_hw_maps.c */
