/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find a port name or number in the port table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/31/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char port_lookup_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "co.h"

extern char *basename();

long port_lookup(name)
register char *name;
{
  register struct port_item *p;
  register long k, n;
  
  if (!*name) return 0;
  if (inanycase("ALL", name) == 0) return 0;

  if (!co)  return krash("port_lookup", "co not open", 1);

  for (k = n = 0; k < strlen(name); k++)
  {
    if (name[k] < '0' || name[k] > '9') {n = -1; break;}
    n = 10 * n + (name[k] - '0');
  }
  if (n >= 0 && n < coh->co_ports && po[n].po_id) return n + 1;

  for (k = 0, p = po; k < coh->co_ports; k++, p++)
  {
    if (inanycase(name, basename(p->po_name)) == 0) return k + 1;
  }
  return -1;
}

/* end of port_lookup.c */ 
