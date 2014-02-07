#include <stdio.h>
#include "co.h"


main()
{
  register long k;

  putenv("_find_box_ops");
  chdir(getenv("HOME"));

  co_open();
  
  for (k = 0; k < coh->co_zone_cnt; k++)
  {
    if (zone[k].zt_flags & BoxOperation)
    {
      printf("PL: %d Zone: %3d  Status: %c Bay: %3d\n", 
        zone[k].zt_pl,
        zone[k].zt_zone, 
        zone[k].zt_status,
        zone[k].zt_first_bay);
    }
  }
  co_close();
}
