/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create a temporary file name.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/17/93    |  tjt  Rewritten to use tempnam() call.
 *-------------------------------------------------------------------------*/
static char tmp_name_c[] = "%Z% %M% %I% (%G% - %U%)";

/*
 *  Generates a file name of "tmp/tfaaaannnnn"   (15 bytes + null).
 *
 *     where nnnnn is the program pid
 *           aaaa   is an alpha field AAAa, BAAa, etc
 *
 *  NOTE: "tmp/" must be a directory below current directory.
 */

char *tmp_name(ptr)
char *ptr;
{
  strcpy(ptr, tempnam("tmp/", "tf"));
  
  return ptr;
}

/* end tmp_name.c */ 
