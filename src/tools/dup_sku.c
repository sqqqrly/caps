/*
 *  dup_sku.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"
#include "co.h"
#include "st.h"
#include "ss.h"
prodfile_item p;
char last[15] = {0};
char number [4];

pmfile_item pm;

main()
{
  putenv("_=dup_sku");
  chdir(getenv("HOME"));

  database_open();

  while (1)
  {
    printf("\n\n");
    printf("     1. Check Pmfile\n");
    printf("     2. Check Prodifle\n");
    printf("     3. Check Sku Table\n");
    printf("     4. Dump Sku Table\n\n");
    
    printf("     Enter Selection ---> ");
    
    gets(number);
    
    switch (*number - 0x30)
    {
      case 0:  exit(0);
      
      case 1:  check_pm_db();
               break;
      case 2:  check_sku_db();
               break;
      case 3:  find_dup_sku();
               break;
      case 4:  dump_sku();
               break;
    }
  }
  database_close();
}
check_pm_db()
{
  long count;
  
  count = 0;
 
  pmfile_open(READONLY);
  pmfile_setkey(2);

  prodfile_open(READONLY);
  prodfile_setkey(1);
  
  printf("Checking PM DB\n");

  while(!pmfile_next(&pm, NOLOCK))
  {
/*
#ifdef INFORMIX
    printf("\rCount %d", ++count);
#else
    printf("\rCount %d  where=%d", ++count, pmfile_current());
#endif
*/
    printf("\rCount %d", ++count);
    fflush(stdout);
    
    memcpy(p.p_pfsku, pm.p_pmsku, 15);
    if (prodfile_read(&p, READONLY))
    {
      printf("\r\n  SKU %15.15s Not In Product File\n", p.p_pfsku);
    }
    if (memcmp(last, pm.p_pmsku, 15) == 0)
    {
      printf("  Duplicate SKU %15.15s\n", last);
    }
    memcpy(last, p.p_pfsku, 15);
  }
  printf("\n\nAll Done\n");

  pmfile_close();
}

check_sku_db()
{
  long count;
  
  count = 0;
  
  prodfile_open(READONLY);
  prodfile_setkey(1);
  
  printf("Checking Product DB\n");

  while(!prodfile_next(&p, NOLOCK))
  {
    printf("\rCount %d", ++count);
    fflush(stdout);
    
    if (memcmp(last, p.p_pfsku, 15) == 0)
    {
      printf("  Duplicate SKU %15.15s\n", last);
    }
    memcpy(last, p.p_pfsku, 15);
  
  }
  printf("\n\nAll Done\n");
  prodfile_close();
}
find_dup_sku()
{
  register struct st_item *s, *t;
  register long j, k;
  char last[15];
  
  co_open();

  printf("There Are %d SKU's In The st Table\n\n", coh->co_st_cnt);

  for (k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    printf("\r%5d", k + 1);
    fflush(stdout);

    for (j = k + 1, t = s + 1; j < coh->co_st_cnt; j++, t++)

    if (memcmp(s->st_sku, t->st_sku, rf->rf_sku) == 0)
    {
      printf("PL:%d SKU: %15.15s  Module: %d Are Duplicates\n", 
        s->st_pl, s->st_sku, s->st_mod);
      printf("PL:%d SKU: %15.15s  Module: %d Are Duplicates\n\n", 
        t->st_pl, t->st_sku, t->st_mod);
    }
  }
  co_close();
}
dump_sku()
{
  register struct st_item *s, *t;
  register long k;
  
  co_open();

  printf("There Are %d SKU's In The st Table\n\n", coh->co_st_cnt);

  for (k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    printf("PL:%2d SKU: %-15.15s  Module: %d\n",
        s->st_pl, s->st_sku, s->st_mod);
  }
  co_close();
}

/* end of dup_sku.c */
