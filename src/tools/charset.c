/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Shoe Entire Code Set
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  01/16/97   |  tjt  Original implementation
 *-------------------------------------------------------------------------*/
static char charset_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <locale.h>

main(argc, argv)
long argc;
char **argv;
{
 register long j, k;
 
 if (argc > 1)
 {
   setlocale(LC_ALL, argv[1]);
 }
 
 for (k = 0; k < 256; k++)
 {
   if (!(k % 8)) printf("\n\r");
   printf("0x%02x %c ", k, k > 0x20? k : '.');
 }
 printf("\n");
}
 



