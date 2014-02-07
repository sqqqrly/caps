/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear Wyse 50 Terminal Function Keys.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 7/23/93     |  tjt  Rewritten.
 *-------------------------------------------------------------------------*/
static char clear_keys_c[] = "%Z% %M% %I% (%G% - %U%)";

/* This program clears function keys and messages */

#include        <stdio.h>

/* See wyse50 manual pages 21 & 24 for ESC z codes */


main()
{
  register long k;

  printf("\n\nClear All Wyse Terminal Function Keys\n\n");

  for (k = 0; k < 16; k++)                /* clear message                   */
  {
    putchar(0x1b);
    putchar('z');
    putchar(0x30 + k);
    putchar('\r');
    fflush(stdout);
  }
  for (k = 0; k < 16; k++)                /* clear message                   */
  {
    putchar(0x1b);
    putchar('z');
    putchar(0x50 + k);
    putchar('\r');
    fflush(stdout);
  }
  for (k = 0; k < 15; k++)                /* clear function                  */
  {
    putchar(0x1b);
    putchar('z');
    putchar(0x40 + k);
    putchar('\r');
    putchar(0x7f);
    fflush(stdout);
  }
  for (k = 0; k < 15; k++)                /* clear function                  */
  {
    putchar(0x1b);
    putchar('z');
    putchar(0x60 + k);
    putchar('\r');
    putchar(0x7f);
    fflush(stdout);
  }
  printf("Too save these function key settings, do the following:\n\n");

  printf("  Hit SHIFT + SETUP\n");
  printf("  Hit SHIFT + SETUP Again\n");
  printf("  Hit the letter 'a'\n\n");
  printf("The function key setting are now saved forever !!!\n\n");

  printf("All Done\n\n");
}

/* end of clear_keys.c */
