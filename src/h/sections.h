/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Data structure definitions for Manpower Planning & Zone
 *                  Balancing By Sections.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/04/01   |  aha  Created file.
 *  11/01/01   |  aha  Added location_item data structure.
 *-------------------------------------------------------------------------*/
#ifndef SECTIONS_H
#define SECTIONS_H

static char sections_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "caps_copyright.h"

/****************************************************************************/
/*                                                                          */
/*  Record structure for the sections database table.                       */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   short          s_bay;
   char           s_caps_section[8];
   short          s_zone; 

} sections_item;


/****************************************************************************/
/*                                                                          */
/*  Record structure for the picker_error database table.                   */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   long           pe_pie_id;
   char           pe_pie_desc[16];
   long           pe_pie_num;
   long           pe_pie_sample; 

} picker_error_item;


/****************************************************************************/
/*                                                                          */
/*  Record structure for the section_prod database table.                   */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   char           scp_caps_sections[8];
   long           scp_picker_id;
   short          scp_zone;
   long           scp_cum_order_count;
   long           scp_cum_lines;
   long           scp_cum_units;
   long           scp_cum_time;
   long           scp_start_time;
   long           scp_login_time;
   long           scp_logout_time;
   short          scp_status;
   char           scp_record_date[20];

} section_prod_item;


/****************************************************************************/
/*                                                                          */
/*  Record structure for the section_prod_log database table.               */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   char           spl_caps_sections[8];
   long           spl_picker_id;
   short          spl_zone;
   long           spl_cum_order_count;
   long           spl_cum_lines;
   long           spl_cum_units;
   long           spl_cum_time;
   long           spl_start_time;
   long           spl_login_time;
   long           spl_logout_time;
   short          spl_status;
   char           spl_record_date[20];

} section_prod_log_item;


/****************************************************************************/
/*                                                                          */
/*  Record structure for the standards file (pick_rate).                    */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   char           caps_section[8];
   double         standard; 

} standards_item;


/****************************************************************************/
/*                                                                          */
/*  Data structure for picker scanning.                                     */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   long           ps_picker_id;
   short          ps_scan_zone; 

} picker_scan_item;


/****************************************************************************/
/*                                                                          */
/*  Data structure for the a picker's productivity locations.               */
/*                                                                          */
/****************************************************************************/
typedef struct
{
   char           section_name[8];
   short          zone_num; 

} location_item;


#endif

/* end of sections.h */
