/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Build operator file = operator.asc.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/09/93   |  tjt  Original inplementation.
 *  08/02/94   |  tjt  Printer as a parameter.
 *  10/06/95   |  tjt  Rewrite for INFORMIX.
 *-------------------------------------------------------------------------*/
static char op_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "bard/operator.h"
#include "Bard.h"
#include "oracle_defines.h"

FILE *fd;

operator_item op;

char features[8][32] = {0};

main(argc, argv)
long argc;
char **argv;
{
  register long k, len;
  register char *p;
  
  putenv("_=op_init");
  chdir(getenv("HOME"));
  
  printf("Create Operator File\n");

  database_open();
  operator_open(AUTOLOCK);
  operator_setkey(1);
  
  begin_work();
  while (!operator_next(&op, LOCK)) operator_delete();
  commit_work();
  
  memset(&op, 0, sizeof(operator_item));
  
  p = (char *)getenv("PRINTER");

  if (argc > 1)   strcpy(op.o_op_printer, argv[1]);
  else if (p > 0) strcpy(op.o_op_printer, p);
  else            strcpy(op.o_op_printer, "lp1");

  printf("Operator Default Printer Is [%s]\n\n", op.o_op_printer);

  fd = fopen(fl_table_name, "r");
  if (fd == 0)
  {
    printf("*** Can't Open %s\n\n", fl_table_name);
    exit(1);
  }
  for (k = 0; k < 8; k++)
  {
    fgets(features[k], 32, fd);
    len = strlen(features[k]) - 1;
    features[k][len] = 0;
  
    printf("feature[%d]: [%s]\n", k+1, features[k]);
  }
  fclose(fd);
  
  memcpy(op.o_op_mm,     features[0], 32);
  memcpy(op.o_op_ops,    features[1], 32);
  memcpy(op.o_op_sys,    features[2], 32);
  memcpy(op.o_op_config, features[3], 32);
  memcpy(op.o_op_prod,   features[4], 32);
  memcpy(op.o_op_sku,    features[6], 32);
  memcpy(op.o_op_label,  features[7], 32);
  
  strcpy(op.o_op_name, "admin");
  strcpy(op.o_op_desc, "CAPS Administration");
  strcpy(op.o_op_pickline, "1");
  op.o_op_level[0] = 'S';
  
  begin_work();
  operator_write(&op);
  
  for (k = 0; k <= 16; k++)
  {
    sprintf(op.o_op_name, "caps%02d", k);
    strcpy(op.o_op_desc, "CAPS Operator");
    operator_write(&op);
  }
  commit_work();
  operator_close();
  database_close();

  printf("All Done\n\n");
}

/* end of op_init.c */
