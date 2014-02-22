/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Add SKU's to configuration segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/2/93    |  tjt  Added to mfc.
 *  06/30/95   |  tjt  Add co_st_changed flag if table changed.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  08/31/95   |  tjt  Scan picks for changes.
 *  10/16/95   |  tjt  Add better check of mirrored.
 *  10/17/95   |  tjt  No st changed flag on errors.
 *  05/03/96   |  tjt  Fix remove mirror checking for old scheme.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  01/07/97   |  tjt  Add 4 character display.
 *-------------------------------------------------------------------------*/
static char st_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "ss.h"
#include "co.h"
#include "st.h"
#include "of.h"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

extern compare();

FILE *ed;                                 /* error file                      */
char ed_name[16];                         /* error file                      */
long errors = 0;
long dups   = 0;

prodfile_item   sku_rec;
pmfile_item     pkm_rec;

struct st_item *save_st = 0;              /* old copy of st                  */

/*-------------------------------------------------------------------------*
 *  M A I N
 *-------------------------------------------------------------------------*/
main( argc, argv )
long  argc;
char  **argv;
{
  register struct hw_item *h;
  register struct pw_item *i;
  register struct bay_item *b, *c;
  register struct zone_item *y, *z;
  register struct st_item *s;
  register long j, k, m, max, size;
  long pid, stat;
  char text[80], prt_name[16];
  
  putenv("_=st_init");                    /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */

  database_open();

  tmp_name(ed_name);
  ed = fopen(ed_name, "w");
  if (ed == 0) krash("st_init", "tmp file", 1);

  prodfile_open( READONLY );
  prodfile_setkey(1);

  pmfile_open( READONLY );
  pmfile_setkey(0);                       /* physical order                  */

  ss_open();
  co_open();                              /* open shared segment             */
  
  size = coh->co_products * sizeof(struct st_item);

  if (coh->co_st_cnt > 0)
  {
    save_st = (struct st_item *)malloc(size);
    if (!save_st) krash("main", "alloc save st table", 1);

    memcpy(save_st, st, size);
  }
  memset(st, 0, size);

  coh->co_st_cnt = coh->co_st_changed = max = 0;

  for (k = 0, i = pw; k < coh->co_prod_cnt; k++, i++)
  {
    i->pw_flags |= (PicksInhibited | BinUnassigned);
  }

/*-------------------------------------------------------------------------*
 *      Build SKU table
 *-------------------------------------------------------------------------*/
  begin_work();
  while( ! pmfile_next( &pkm_rec, NOLOCK ) )
  {
#ifdef DEBUG
  fprintf(stderr, "%15.15s  max=%d\n", pkm_rec.p_pmsku, max);
#endif
        
    if (pkm_rec.p_pmodno < 1) continue;
    if (pkm_rec.p_pmsku[0] <= 0x20) continue;
    if (pkm_rec.p_pmodno > coh->co_prod_cnt) continue;

    i = &pw[pkm_rec.p_pmodno - 1];
    k = i->pw_ptr;

    if (k <= 0 || k > coh->co_hw_cnt) continue;
    
    h = &hw[k - 1];

    i->pw_flags &= ~BinUnassigned;
    
    if (pkm_rec.p_piflag != 'y') i->pw_flags &= ~PicksInhibited;

    memcpy(i->pw_display, pkm_rec.p_display, 4);
    
    i->pw_case = 1000;                    /* default autocasing              */
    i->pw_pack = 100;
    
    if(pkm_rec.p_acflag == 'y' && sp->sp_autocasing != 'n')
    {
      strncpy(sku_rec.p_pfsku, pkm_rec.p_pmsku, 15);

      if (! prodfile_read( &sku_rec, NOLOCK))
      {
         i->pw_case = sku_rec.p_cpack;
         i->pw_pack = sku_rec.p_ipqty;
      }
    }
    if (h->hw_bay) 
    {
      b = &bay[h->hw_bay - 1];
      if (b->bay_zone) z = &zone[b->bay_zone - 1];
      else continue;
    }
    else continue;

    if (z->zt_pl) st[max].st_pl = z->zt_pl;
    else continue;
    
    strncpy(st[max].st_sku, pkm_rec.p_pmsku, SkuLength);
    strip_space(st[max].st_sku, SkuLength);

    strncpy(st[max].st_stkloc, pkm_rec.p_stkloc, StklocLength);
    strip_space(st[max].st_stkloc, StklocLength);

    st[max].st_mod = pkm_rec.p_pmodno;

    st[max].st_mirror = 0;

    max++;
    
    commit_work();
    begin_work();
  }
  commit_work();
  
  qsort(st, max, sizeof(struct st_item), compare);

  coh->co_st_cnt = max;

  for (k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    m = pw[s->st_mod - 1].pw_ptr;
    h = &hw[m - 1];
    b = &bay[h->hw_bay - 1];
    y = &zone[b->bay_zone - 1];
    
    for (j = 1; k + j < coh->co_st_cnt; j++)
    {
       if (memcmp(s, s + j, SkuLength + 1) != 0) break;
       s->st_mirror += 1;
    
       if (sp->sp_mirroring != 'y') continue;

       m = pw[(s + j)->st_mod - 1].pw_ptr;
       h = &hw[m - 1];
       c = &bay[h->hw_bay - 1];
       z = &zone[c->bay_zone - 1];
       
       y->zt_flags  |= IsMirror;
       z->zt_flags  |= IsMirror;
       b->bay_flags |= IsMirror;
       c->bay_flags |= IsMirror;
       sg[y->zt_segment - 1].sg_flags |= IsMirror;
       sg[z->zt_segment - 1].sg_flags |= IsMirror;
    }
    if (s->st_mirror && sp->sp_mirroring != 'y')
    {
       fprintf(ed, "Sku %s Duplicated For Modules %d And %d\n",
         s->st_sku, s->st_mod, (s + 1)->st_mod);
	    dups++;
    }
#ifdef DEBUG
  Bdump(s, sizeof(struct st_item));
#endif

  }
#ifndef DELL
    
  if (save_st && !errors)
  {
    if (memcmp(save_st, st, size) != 0)
    {
      oc_open();
      od_open();

      pick_setkey(0);
      while (!pick_next(op_rec, NOLOCK))
      {
        s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);
        if (s)
        {
          if (s->st_mod == op_rec->pi_mod) continue;
        }
        coh->co_st_changed = 1;
        break;
      }
      od_close();
      oc_close();
    }
  }
#endif  
  
  pmfile_close();
  prodfile_close();
  co_close_save();
  ss_close();
  database_close();
  
  fclose(ed);
  
  if (errors || dups)
  {
    if ((pid = fork()) == 0)
    {
      execlp("prft", "prft", ed_name, tmp_name(prt_name),
        "sys/report/st_report.h", 0);
      krash("st_init", "load prft", 0);
      exit(1);
    }
    pid = wait(&stat);
    if (errors) exit(2);
  }
  else unlink(ed_name);

  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Compare Two Items
 *-------------------------------------------------------------------------*/
compare(p, q)
register unsigned char *p, *q;
{
  return memcmp(p, q, sizeof(struct st_item));
}

/* end of st_init.c */
