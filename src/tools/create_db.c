/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create dummy pmfile and prodfile databases using ss.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/07/95   |  tjt  Revised for INFORMIX.
 *  12/06/95   |  tjt  sp_products is SKU's.
 *  12/06/95   |  tjt  sp_modules  is PM's.
 *  04/18/96   |  tjt  Merge Bard and Informix versions.
 *  07/30/99   |  Informix to Oracle Conversion by Ravi Jagannathan
 *-------------------------------------------------------------------------*/
static char create_db_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "co.h"
#include "Bard.h"
#include "bard/pmfile.h"
#include "bard/prodfile.h"

#define random(x)  (rand() % (x))

FILE *pm_fd, *pf_fd;

char pm_name[32], pf_name[32];

pmfile_item pm;
prodfile_item pf;

/*
 *  Work Fields
 */
char string[30];

char *um[5] = {"ea ", "doz", "box", "qt ", "gal"}; 

char *desc[18] = {"Stuffed Pandaa Bears", "Les Aspirin", 
"Denton Fender Body Putty", "Big Wheel Unicycle", 
"Liquid Dihydrogenoxide", "Tris HiFi Speaker",
"Any Other Name Silk Roses", "Buggy Whips, Assorted", 
"Fuzzy Dice, Large", "TonTon BonBon",
"Flea-Away Soft Soap", "Lefthand Metric Spanner",
"Fitibaldi Hair Club", "Zen For Dummies",
"Faux Fox Tail", "S Car Go Accelerator",
"Alfredo Fettucine Pasta", "Al Fresco Air Cleaner"};

main()
{
  register long k, length, prods, mods, sku;
  register struct pw_item  *i;
  register struct hw_item  *h;
  register struct bay_item *b;
  
  register char *p;
  
  printf("Create Product Databases\n\n");

  putenv("_=create_db");                  /* name of environ                 */
  chdir(getenv("HOME"));                  /* to home directory               */
  
  ss_open();
  co_open();
  length = rf->rf_sku;

#ifdef INFORMIX
  system("dbaccess -e $DATABASE $HOME/src/h/informix/pmfile.sql");
  system("dbaccess -e $DATABASE $HOME/src/h/informix/prodfile.sql");
    
  database_open();
  pmfile_open(AUTOLOCK);
  prodfile_open(AUTOLOCK);
#endif 

#ifdef ORACLE  
  system("sqlplus -s / @$HOME/src/h/oracle/pmfile.sql");
  system("sqlplus -s / @$HOME/src/h/oracle/prodfile.sql");
    
  database_open();
  pmfile_open(AUTOLOCK);
  prodfile_open(AUTOLOCK);
#endif

#ifdef BARD
  
  p = (char *)getenv("DBPATH");
  if (!p) p = "DBPATH";
  
  sprintf(pm_name, "%s/pmfile.dat", p);
  sprintf(pf_name, "%s/prodfile.dat", p);
  
  pm_fd = fopen(pm_name, "w");
  if (pm_fd == 0)
  {
    printf("Can't Open %s\n\n", pm_name);
    exit(1);
  }
  pf_fd = fopen(pf_name, "w");
  if (pf_fd == 0)
  {
    printf("Can't Open %s\n\n", pf_name);
    exit(1);
  }
#endif
/*-------------------------------------------------------------------------*
 *  Initialize Fields
 *-------------------------------------------------------------------------*/
  strncpy(pf.p_fgroup, "group", 5);
  pf.p_ipqty = 25;
  pf.p_cpack = 100;
  strncpy(pf.p_bsloc, "floor ", 6);
  strncpy(pf.p_absloc, "whse  ", 6);

  pm.p_qty     = 1000;
  pm.p_alloc   = 0;
  pm.p_restock = 950;
  pm.p_rqty    = 500;
  pm.p_lcap    = 2000;
  pm.p_piflag  = 'n';
  pm.p_rsflag  = 'n';
  pm.p_acflag  = sp->sp_autocasing == 'n' ?  'n' : 'y';

  /* pm.p_acflag  = 'n'; */

  prods = sp->sp_products;
  mods  = sp->sp_products;
  
  printf("Create %4d SKU's\n", prods);
  printf("Create %4d PM's\n", mods);
  printf("SKU Length Is %d Bytes\n", length);
  printf("Autocasing Is Set To '%c'\n\n", pm.p_acflag);
  
/*-------------------------------------------------------------------------*
 *  Create Data Base
 *-------------------------------------------------------------------------*/
  pm.p_pmodno = sku = 1;
  
  while(pm.p_pmodno <= mods)
  {
    if (sku > prods) sku = 1;
    sprintf(string, "%-015.*d", length, sku);   /* SKU number                */
    memcpy(pm.p_pmsku, string, 15);             
    
    sprintf(string, "L%05d", pm.p_pmodno);      /* stock location            */
    memcpy(pm.p_stkloc, string, StklocLength);
    memset(pm.p_display, 0x20, 4);
    
    if (sp->sp_config_status == 'y')
    {
      i = &pw[pm.p_pmodno - 1];
      h = &hw[i->pw_ptr - 1];
      if (h->hw_bay > 0)
      {
        b = &bay[h->hw_bay - 1];

        if (b->bay_flags & Multibin)
        {
          k = pm.p_pmodno - b->bay_prod_first;
          
          if (b->bay_shelves == 1)
          {
            sprintf(string, "%02d%02d  ",      
            b->bay_number, k + 1);          /* actual bay number + count     */

            memcpy(pm.p_stkloc, string, StklocLength);
            memcpy(pm.p_display, string, 4);
          }
          else
          {
            sprintf(string, "%03d-%c%c",      
            b->bay_number,                  /* actual bay number             */
            (k / b->bay_width) + '1',       /* shelf number  1 .. n          */
            (k % b->bay_width) + 'A');      /* slot  A .. Z                  */

            memcpy(pm.p_stkloc, string, StklocLength);
            memcpy(pm.p_display, string + 4, 4);
          }
        }
      }
    }
    
#ifdef INFORMIX
    pmfile_write(&pm);
#endif

#ifdef ORACLE
    pmfile_write(&pm);
#endif

#ifdef BARD
    fwrite(&pm, pmfile_size, 1, pm_fd);
#endif
    
    pm.p_pmodno++;
    sku++;
  }
  sku = 1;
    
  while(sku <= prods)
  {
    sprintf(string, "%-015.*d", length, sku);   /* SKU number                */
    memcpy(pf.p_pfsku, string, 15);             

    strncpy(pf.p_um, um[random(5)], 3);         /* unit of measure           */

/*  sprintf(string,"SKU #%04d %15c", sku, ' ');  */

    sprintf(string, "%-25.25s", desc[sku % 18]);
    memcpy(pf.p_descr, string, 25);

    sprintf(string, "ALT #%04d %15c", sku, ' ');
    memcpy(pf.p_altid, string, 25);

#ifdef INFORMIX
    prodfile_write(&pf);
#endif

#ifdef ORACLE  
    prodfile_write(&pf);
#endif

#ifdef BARD
    fwrite(&pf, prodfile_size, 1, pf_fd);
#endif
 
    sku++;
  }
  ss_close();
  co_close();

/*#ifdef INFORMIX
  pmfile_close();
  prodfile_close();
  database_close();
#endif */

#ifdef ORACLE
  pmfile_close();
  prodfile_close();
  commit_work();
  database_close();
#endif

#ifdef BARD
  fclose(pm_fd);
  fclose(pf_fd);
  
  system("dat/db/pmfile.cdx");
  system("dat/db/prodfile.cdx");
#endif

  printf("End of Create\n\n");
}

/* end of create_db.c  */
