/*-------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Allocated configuration space based on system parms.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/26/93   |  tjt  Original implementation.
 *  06/22/94   |  tjt  Fix no co_remove when config validation.
 *  11/14/94   |  tjt  Add pickline segments.
 *  01/27/95   |  tjt  Fix bug in view allocation.
 *  03/17/95   |  tjt  Remove tc and ac displays.
 *  04/16/96   |  tjt  Allocate pw by products.
 *-------------------------------------------------------------------------*/
static char co_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "ss.h"
#include "co.h"
#include "st.h"

FILE *fd;
char fd_name[40];

unsigned char    *co;
struct co_header *coh;

#define MAX 13

char *names[MAX] = {"co header", "port",      "pickline",  "segment", 
                    "zone",      "bay",       "module",    "picks",
                    "mh table",  "st_table",  
                    "blv_table", "zcv_table", "pmv_table"};

long item[MAX];
long size[MAX];
long area[MAX];
long offset[MAX];

long live = 1;

main(argc, argv)
long argc;
char **argv;
{
  register long k, total;
  
  putenv("_=co_init");                    /* name to eviron                  */
  chdir(getenv("HOME"));                  /* to home directory               */

  printf("Initialize Configuration Segment\n\n");

  strcpy(fd_name, co_name);
  if (argc > 1)  
  {
    strcpy(fd_name, argv[1]);
    live = 0;
  }
  ss_open();
   
  item[0]  = 1;
  item[1]  = sp->sp_ports;
  item[2]  = sp->sp_picklines;
  item[3]  = sp->sp_segments;
  item[4]  = sp->sp_zones;
  item[5]  = sp->sp_bays;
  item[6]  = sp->sp_lights;
  item[7]  = sp->sp_products;
  item[8]  = sp->sp_modules;
  item[9]  = 0;
  item[10] = 0;
  item[11] = 0;
  item[12] = 0;

  if (rf->rf_sku > 0) 
  {
    item[9] = sp->sp_products;
    if (sp->sp_modules > item[9]) item[9] = sp->sp_modules;
  }
  if (sp->sp_pickline_view == 'y')  
  {
    item[10] = sp->sp_bays;             /* BL                              */
    item[11] = sp->sp_bays;             /* ZC                              */
    item[12] = sp->sp_modules;          /* PM                              */
  }
  size[0]  = sizeof(struct co_header);
  size[1]  = sizeof(struct port_item);
  size[2]  = sizeof(struct pl_item);
  size[3]  = sizeof(struct seg_item);
  size[4]  = sizeof(struct zone_item);
  size[5]  = sizeof(struct bay_item);
  size[6]  = sizeof(struct hw_item);
  size[7]  = sizeof(struct pw_item);
  size[8]  = sizeof(struct mh_item);
  size[9]  = sizeof(struct st_item);
  size[10] = sizeof(struct hw_bl_view_item);
  size[11] = sizeof(struct hw_zc_view_item);
  size[12] = sizeof(struct hw_pm_view_item);
  
  for (k = total = 0; k < MAX; k++)
  {
    area[k]   =  item[k] * size[k];

    if (area[k])  offset[k] = total;
    else offset[k] = 0;
    
    total += area[k];
  }
  printf("Ports        %6d\n", sp->sp_ports);
  printf("Picklines:   %6d\n", sp->sp_picklines);
  printf("Segments:    %6d\n", sp->sp_segments);
  printf("Zones:       %6d\n", sp->sp_zones);
  printf("Bays:        %6d\n", sp->sp_bays);
  printf("Products:    %6d\n", sp->sp_products);
  printf("Modules:     %6d\n", sp->sp_modules);
  printf("Lights:      %6d\n", sp->sp_lights);
  printf("Total Bytes  %6d\n\n", total);

  printf("Component        Items  Size    Area  Offset\n");
  printf("---------------  -----  ----    ----  ------\n");

  for (k = 0; k < MAX; k++)
  {
    printf("%-15s%7d%6d%8d%8d\n",
    	names[k], item[k], size[k], area[k], offset[k]);
  }
  printf("                            --------\n");
  printf("total                       %8d\n\n", total);

  co = (unsigned char *)malloc(total);
  if (!co)
  {
    printf("*** Can't Allocate Space\n\n");
    exit(0);
  }
  memset(co, 0, total);                   /* clear to zero                   */

  if (area[10] > 0) memset(co + offset[10], 0x20, area[10]);
  if (area[11] > 0) memset(co + offset[11], 0x20, area[11]);
  if (area[12] > 0) memset(co + offset[12], 0x20, area[12]);

  coh = (struct co_header *)co;

  coh->co_ports      = sp->sp_ports;
  coh->co_picklines  = sp->sp_picklines;
  coh->co_segments   = sp->sp_segments;
  coh->co_zones      = sp->sp_zones;
  coh->co_bays       = sp->sp_bays;
  coh->co_products   = sp->sp_products;
  coh->co_modules    = sp->sp_modules;
  coh->co_lights     = sp->sp_lights;
  
  ss_close();
  
  coh->co_po_offset      = offset[1];
  coh->co_pl_offset      = offset[2];
  coh->co_seg_offset     = offset[3];
  coh->co_zone_offset    = offset[4];
  coh->co_bay_offset     = offset[5];
  coh->co_hw_offset      = offset[6];
  coh->co_pw_offset      = offset[7];
  coh->co_mh_offset      = offset[8];
  coh->co_st_offset      = offset[9];
  coh->co_bl_view_offset = offset[10];
  coh->co_zc_view_offset = offset[11];
  coh->co_pm_view_offset = offset[12];
  
  fd = fopen(fd_name, "w");
  if (fd == 0)
  {
    printf("*** Can't Open %s\n\n", fd_name);
    exit(1);
  }
  if (fwrite(co, total, 1, fd) != 1)
  {
    printf("*** Write Error On %s\n\n", fd_name);
    exit(1);
  }
  fclose(fd);

  if (live) co_remove();                  /* only remove if not live         */

  printf("All Done\n\n");
  exit(0);
}

/* end of co_init.c */
