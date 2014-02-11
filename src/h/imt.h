/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Item movement times
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/4/93    |  tjt  Added to mfc - this is a kludge!
 *-------------------------------------------------------------------------*/
#ifndef IMT_H
#define IMT_H
static char imt_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "global_types.h"
#include "file_names.h"

/*-------------------------------------------------------------------------*
 * IMT Date/Time Item
 *-------------------------------------------------------------------------*/
struct imt_item
{
  long imt_cur;                           /* Current Start Date/Time         */
  long imt_cum;                           /* Cummulative Start Date/Time     */
};

/*-------------------------------------------------------------------------*
 * IMT Record
 *-------------------------------------------------------------------------*/
struct imt_item imt[PicklineMax];         /* IMT Record for All Picklines    */

/*-------------------------------------------------------------------------*
 *  Load IMT Table
 *-------------------------------------------------------------------------*/
imt_load()
{
  FILE *imt_fd;

  imt_fd = fopen(imt_name, "r");
  if (imt_fd == 0) krash("imt_load", "Open IMT", 1);

  fread (imt, sizeof(imt), 1, imt_fd);

  fclose(imt_fd);
  
  return;
}
/*-------------------------------------------------------------------------*
 *  Unload IMT Table
 *-------------------------------------------------------------------------*/
imt_unload()
{
  FILE *imt_fd;

  imt_fd = fopen(imt_name, "w");
  if (imt_fd == 0) krash("imt_unload", "Open IMT", 1);

  fwrite (imt, sizeof(imt), 1, imt_fd);

  fclose(imt_fd);
  
  return;
}
#endif

/* end of imt.h  */
