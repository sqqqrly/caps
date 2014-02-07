/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Prints old packing list pseudo code.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/3/93    |  tjt Added to mfc.
 *-------------------------------------------------------------------------*/
static char code_print_c[] = "%Z% %M% %I% (%G% - %U%)";
/*
 *  code_print.c
 *
 *  print paper print code
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text_spec.h"

#define MAX 200

FILE *fd;
char fd_name[40];

struct report_spec_item r;
struct text_spec_item   t;

struct text_spec_item tab[MAX];
long max = 0;

main(argc, argv)
long argc;
char **argv;
{
  register long k;

  putenv("_=code_print");
  chdir(getenv("HOME"));
       
  if (argc < 2)
  {
    printf("Missing File Name\n\n");
    exit(1);
  }
  strcat(fd_name, "sys/");
  strcat(fd_name, argv[1]);
  strcat(fd_name, ".t");
        
  fd = fopen(fd_name, "r");
  if (fd == 0)
  {
    printf("Can't Open %s\n\n", fd_name);
    exit(1);
  }
  load_spec("system_fields");
  load_others();
  load_spec("remarks");
  load_spec("pick_text");

  printf("Opcode   P1    P2\n");
  printf("------ ----- -----\n");
        
  while (fread(&r, sizeof(struct report_spec_item), 1, fd) > 0)
  {
    printf("%4d    %3d %5d    ", r.text_op, r.text_p1, r.text_p2);

    if (r.text_op == 66)
    {
      print_literal(argv[1], r.text_p1, r.text_p2);
      printf("\n");
      continue;
    }
    for (k = 0; k < max; k++)
    {
      if (r.text_op != tab[k].text_no) continue;
      if (r.text_op >= 64)
      {
        if (r.text_p1 != tab[k].text_len) continue;
        if (r.text_p2 != tab[k].text_offset) continue;
      }
      if (k < MAX)
      {
        printf("%s [%d] [%d]", tab[k].text_name, r.text_p1, r.text_p2);
      }
      printf("\n");

    }
  }
  printf("\n");
  printf("End of List\n");
  fclose(fd);
}

load_spec(name)
char *name;
{
  FILE *sd;
  char sd_name[40];
        
  strcpy(sd_name, "sys/");
  strcat(sd_name, name);
  strcat(sd_name, ".t");
        
  sd = fopen(sd_name, "r");
  if (sd == 0)
  {
    printf("Can't Open %s\n\n", sd_name);
    return;
  }
  while (fread(&tab[max], sizeof(struct text_spec_item), 1, sd) > 0)
  {
    max++;
  }
  fclose(sd);
}

load_others()
{
  tab[max].text_no = 26; strcpy(&tab[max].text_name[0], "print");    max++;
  tab[max].text_no = 27; strcpy(&tab[max].text_name[0], "copies");   max++;
  tab[max].text_no = 28; strcpy(&tab[max].text_name[0], "if_print"); max++;
  tab[max].text_no = 29; strcpy(&tab[max].text_name[0], "if_flag");  max++;
  tab[max].text_no = 30; strcpy(&tab[max].text_name[0], "repeats");  max++;
  tab[max].text_no = 31; strcpy(&tab[max].text_name[0], "end");      max++;
}

print_literal(name, len, offset)
char *name;
long len, offset;
{
  FILE *ld;
  char ld_name[40];
  char buffer[132];
        
  strcpy(ld_name, "sys/");
  strcat(ld_name, name);
  strcat(ld_name, ".lits");

  ld = fopen(ld_name, "r");
  if (ld == 0) return;

  fseek(ld, offset, 0);
  fread(buffer, len, 1, ld);
  fclose(ld);
        
  printf("%-*.*s", len, len, buffer);
}
        
/* end of code_print.c */

