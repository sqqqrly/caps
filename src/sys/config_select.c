/* #define SKIP_LAST_ZONE */
/*-------------------------------------------------------------------------*
 *  Custom Code:    SKIP_LAST_ZONE - ignore last zone in each pickline.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration Selection.
 *
 *  Execution:      config_select [l|u]              lines of units.
 *                                [number of orders]
 *                                [number of hours]
 *                                [y|n]              current productivity.
 *                                [y|n]              cummulative.
 *                                [y|n]              pick rates.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/29/96   |  tjt Rewritten.
 *  04/18/97   |  tjt Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char config_select_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"
#include "st.h"
#include "eh_nos.h"
#include "language.h"
#include "config_select.t"
#include "Bard.h"

#define WIDTH 80
#define NAMES 12
#define NSIZE 32

double atof();
/* 
 * Global variables
 */
FILE *fd;                                   /* screen output                 */
char fd_name[16];
FILE *cd;                                   /* report output                 */
char cd_name[16];
long savefp;

short ONE = 1;

struct fld_parms fld1 = {23, 26, 1, 1, &ONE,
  "Print Details ? (y/n)       (Exit, Forward, or Backward)",'a'};

unsigned char t;
unsigned char buf[2];

long pickline, zone_limit;
char rates[12];

/*-------------------------------------------------------------------------*
 * Passed Parameters
 *-------------------------------------------------------------------------*/

char  bal_type;                            /* lines | units                  */
long  order_limit;                         /* orders to use                  */
float hour_limit;                          /* time available                 */
char  use_what;                            /* rates to use                   */

/*-------------------------------------------------------------------------*
 *  Global Values
 *-------------------------------------------------------------------------*/
long order_count;                          /* number of orders found         */

/*-------------------------------------------------------------------------*
 *  Global Tables
 *-------------------------------------------------------------------------*/

typedef struct                             /* product locations              */
{
  long bin_units;
  long bin_lines;
 
} bin_item;

typedef struct                             /* light modules                  */
{
  long mod_units;
  long mod_lines;
  
} mod_item;

typedef struct                             /* zone record                    */
{
  long  zn_units;                         
  long  zn_lines;
  long  zn_pickline;
  float zn_time;
  float zn_unit_rate;
  float zn_line_rate;
  
} zn_item;

typedef struct                             /* pickline record                */
{
  long  pn_units;                         
  long  pn_lines;
  long  pn_worst_zone;
  float pn_time;
  float pn_unit_rate;
  float pn_line_rate;
  
} pn_item;

typedef struct
{
  char  cn_name[NSIZE];
  float cn_time;
  short cn_pickline_cnt;
  short cn_worst_pickline;
  short cn_worst_zone;
  short cn_status;                         /* 0 = OK                         */
                                           /* 1 = configuration failed       */
                                           /* 2 = co open failed             */
                                           /* 3 = orphan picks               */
                                           /* 4 = no pick rate file          */
                                           /* 5 = no rate for module         */
                                           /* 6 = total is zero              */
} cn_item;

float *pickrate;                           /* module pick rates              */

bin_item    *bin;                          /* product locations              */
mod_item    *mod;                          /* pick modules                   */
zn_item     *zn;                           /* zone items                     */
pn_item     *pn;                           /* pickline items                 */
cn_item     *cn;                           /* configuration items            */

long cn_max = 0;                           /* number of configuration items  */

main(argc,argv)
short argc;
char **argv;
{
  register long k;
  
  extern leave();
  
  putenv("_=config_select");
  chdir(getenv("HOME"));

  signal_catcher(0);

  open_all();

  bal_type    = tolower(argv[1][0]);
  order_limit = atol(argv[2]);
  hour_limit  = atof(argv[3]);

  if (tolower(argv[4][0]) == 'y')      use_what = 1;
  else if (tolower(argv[5][0]) == 'y') use_what = 2;
  else                                 use_what = 3;

#ifdef DEBUG
  for (k = 0; k < argc; k++)
  {
    fprintf(stderr, "arg[%d] = [%s]\n", k, argv[k]);
  }
  fprintf(stderr, "bal_type    = %c\n",    bal_type);
  fprintf(stderr, "order_limit = %d\n",    order_limit);
  fprintf(stderr, "hour_limit  = %5.3f\n", hour_limit);
  fprintf(stderr, "use_what    = %d\n",    use_what);
#endif
  
  fix(config_select);
  sd_screen_off();
  sd_clear_screen();
  sd_text(config_select);
  sd_screen_on();

  make_tables();                          /* workload tables                 */
  load_names();                           /* load configuration names        */
  sd_wait();                              /* flash wait message              */
  determine_load();                       /* determine load for bins         */

  if (use_what == 1 || use_what == 2)
  {
    use_pr_rates();
  }
  co_close(); co = 0;
  
  for (k = 0; k < cn_max; k++)            /* over all configurations         */
  {
    cn[k].cn_status = make_co(cn[k].cn_name);

    if (!cn[k].cn_status)
    {
      if (use_what == 3)
      {
        cn[k].cn_status = read_pick_rate_file(cn[k].cn_name);
      }
      if (!cn[k].cn_status)
      {
        cn[k].cn_status = calculate(k);
      }
    }
    store_data(k);
    
    if (co) free(co);
    co = 0;
  }
  show_heading();

/*
 * display data
 */
  show_heading();
  
  fseek(fd, 0, 0);
  sd_cursor(0, 9, 1);
  savefp = 0;
  show(fd, 14, 2);

  memset(buf, 0, 2);

  while(1)
  {
    t = sd_input(&fld1,sd_prompt(&fld1, 0), 0, buf, 0);

    switch(sd_print(t, code_to_caps(buf[0])))  /* F041897 */
    {
      case(0) : leave();

      case(1) : sd_cursor(0,9,1);
                sd_clear_rest();
                sd_cursor(0,9,1);
                show(fd,14,2);            /* display more data               */
                break;

      case(2) : sd_cursor(0,9,1);
                sd_clear_rest();
                sd_cursor(0,9,1);
                show(fd,14,1);
                break;

      case(3) : leave();

      case(4) : print();
                leave();
                break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Show Column Heading
 *-------------------------------------------------------------------------*/
show_heading()
{
/*
         1         2         3         4         5         6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
                                           Lines/           Units/
   Configuration Name   PL  Zone   Lines    Hour    Units    Hour    Time
  --------------------  --  ----  -------  ------  -------  ------  ------
  xxxxxxxxxxxxxxxxxxxx  xx  xxx   xxxxxx   xxxxx   xxxxxx   xxxxx    xx:xx 
*/
  sd_cursor(0, 6, 1);
  sd_text(
  "                                           Lines/           Units/");
  sd_cursor(0, 7, 1);
  sd_text(
  "   Configuration Name   PL  Zone   Lines    Hour    Units    Hour    Time");
  sd_cursor(0, 8, 1);
  sd_text(
"  --------------------  --  ----  -------  ------  -------  ------  ------");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Calculate configuration performance. 
 *-------------------------------------------------------------------------*/
calculate(n)
register long n;
{
  register long j, k, status;
  register bin_item *bi;
  register mod_item *mi;
  register zn_item  *zi;
  register pn_item  *pi;
  register cn_item  *ci;
  register struct pw_item   *i;
  register struct hw_item   *h;
  register struct bay_item  *b;
  register struct zone_item *z;
  
  status = 0;
  
  ci = &cn[n];
  
  for (k = 0, bi = bin, i = pw; k < coh->co_products; k++, bi++, i++)
  {
    if (!i->pw_ptr) {status = 3; continue;} /* orphan bin location           */

    h  = &hw[i->pw_ptr - 1];
    mi = &mod[i->pw_ptr - 1];
    
    if (!h->hw_bay) {status = 3; continue;}  /* orphan light module          */
  
    b = &bay[h->hw_bay - 1];
    if (!b->bay_zone) {status = 3; continue;}
    
    z  = &zone[b->bay_zone - 1];
    zi = &zn[b->bay_zone - 1];
     
    if (!z->zt_pl) {status = 3; continue;}
    pi = &pn[z->zt_pl - 1];
    
#ifdef DETAIL
    fprintf(stderr, "mod = %d  rate = %8.3f\n", 
      h->hw_mod, pickrate[h->hw_mod - 1]);
#endif
    
    zi->zn_pickline = z->zt_pl;
    
    if (bi->bin_lines < 1) continue;
    
    if (pickrate[h->hw_mod - 1] < 0.001) {status = 5; continue;}
  
    mi->mod_units += bi->bin_units;
    mi->mod_lines += bi->bin_lines;
  
    zi->zn_units  += bi->bin_units;
    zi->zn_lines  += bi->bin_lines;
  
    pi->pn_units  += bi->bin_units;
    pi->pn_lines  += bi->bin_lines;
  
    if (bal_type == 'u')
    {
      zi->zn_time += bi->bin_units / pickrate[h->hw_mod - 1];
    }
    else zi->zn_time += bi->bin_lines / pickrate[h->hw_mod - 1];
  }
  for (j = 0, pi = pn; j < coh->co_pl_cnt; j++, pi++)
  {
    if (pi->pn_lines < 1) continue;
    
    for (k = 0, zi = zn; k < coh->co_zone_cnt; k++, zi++)
    {
      if (zi->zn_pickline != j+1) continue;
      
      if (zi->zn_time > 0.0) 
      {
        zi->zn_unit_rate = zi->zn_units / zi->zn_time;
        zi->zn_line_rate = zi->zn_lines / zi->zn_time;
      }
#ifdef SKIP_LAST_ZONE
      if (pl[j].pl_last_zone == k + 1) continue;
#endif
      if (zi->zn_time > pi->pn_time) 
      {
        pi->pn_time       = zi->zn_time;
        pi->pn_worst_zone = k+1;
      
        if (pi->pn_time > ci->cn_time)
        {
          ci->cn_time           = pi->pn_time;
          ci->cn_worst_pickline = j+1;
          ci->cn_worst_zone     = k+1;
        }
      }
    }
    if (pi->pn_time > 0.0) 
    {
      pi->pn_unit_rate = pi->pn_units / pi->pn_time;
      pi->pn_line_rate = pi->pn_lines / pi->pn_time;
    }
  }
  if (ci->cn_time <= 0.0) status = 6;     /* all are zero                   */

#ifdef DEBUG
  dump_zones();
  dump_picklines();
#endif
  return status;
}
/*-------------------------------------------------------------------------*
 *  Load Configuration Names From sys/config_list.
 *-------------------------------------------------------------------------*/
load_names()
{
  FILE *nd;
  register long n;

#ifdef DEBUG
  fprintf(stderr, "load_names()\n");
#endif
  
  cn_max = 0;
  
  nd = fopen("sys/config_list", "r");
  if (nd == 0) return 0;
  
  while (fgets(cn[cn_max].cn_name, NSIZE - 1, nd) > 0)
  {
    n = strlen(cn[cn_max].cn_name) - 1;
    cn[cn_max].cn_name[n] = 0;             /* remove line feed               */

#ifdef DEBUG
    fprintf(stderr, "name[%d] = [%s]\n", cn_max, cn[cn_max].cn_name);
#endif

    cn_max++;
    if (cn_max >= NAMES) break;
  }
  fclose(nd);
  
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Determine Bin Location Load
 *-------------------------------------------------------------------------*/
determine_load()
{
  register long k;
  long count, block;

#ifdef DEBUG
  fprintf(stderr, "determine_load()\n");
#endif

  if (!order_limit) 
  {
    count = sp->sp_orders;
    return count_picks();
  }
  else count = order_limit;

  for (k = 0; k < coh->co_pl_cnt; k++)
  {
    block = oc->oc_tab[k].oc_high.oc_first;
    if (block) check_queue(block, &count);

    block = oc->oc_tab[k].oc_med.oc_first;
    if (block) check_queue(block, &count);

    block = oc->oc_tab[k].oc_low.oc_first;
    if (block) check_queue(block, &count);
  }
#ifdef DETAIL
  dump_bins();
#endif

  return 0;
}
/*--------------------------------------------------------------------------*
 * Produce a Configuration and Load
 *--------------------------------------------------------------------------*/
make_co(name)
register char *name;
{
  register long k;
  FILE *co_fd;
  char co_name[64];
  long pid, status;
  
#ifdef DEBUG
  fprintf(stderr, "make_co(%s)\n", name);
#endif

  if (fork() == 0)
  {
     execlp("configure", "configure", name, "-s", 0);
     krash("make_co", "load configure", 1);
  }
  pid = wait(&status);
  
  if (status) return 1;                    /* configuration failed           */
 
  sprintf(co_name, "tmp/%s.co", name);
  
  co_fd = fopen(co_name, "r");
  if (co_fd == 0) return 2;                /* no output from configure       */
  
  fseek(co_fd, 0, 2);
  co_size = ftell(co_fd);
  fseek(co_fd, 0, 0);
  
  co = (unsigned char *)malloc(co_size);   /* allocate space                 */
  if (co == 0) krash("make_co", "alloc co", 1);
  
  if (fread(co, co_size, 1, co_fd) != 1) 
  {
    krash("make_co", "read co", 1);
  }
  fclose(co_fd);
  unlink(co_name);
  
  coh   = (struct co_header *)co;

  po    = (struct port_item *) ((char *)co + coh->co_po_offset);
  pl    = (struct pl_item *)   ((char *)co + coh->co_pl_offset);
  sg    = (struct seg_item *)  ((char *)co + coh->co_seg_offset);
  zone  = (struct zone_item *) ((char *)co + coh->co_zone_offset);
  bay   = (struct bay_item *)  ((char *)co + coh->co_bay_offset);
  hw    = (struct hw_item *)   ((char *)co + coh->co_hw_offset);
  pw    = (struct pw_item *)   ((char *)co + coh->co_pw_offset);
  mh    = (struct mh_item *)   ((char *)co + coh->co_mh_offset);
  
  memset(mod, 0, sizeof(mod_item) * coh->co_modules);
  memset(zn,  0, sizeof(zn_item)  * coh->co_zones);
  memset(pn,  0, sizeof(pn_item)  * coh->co_picklines);
  
  return 0;
}
/*--------------------------------------------------------------------------*
 * check the queues for picks
 *--------------------------------------------------------------------------*/
check_queue(block, count)
register long block;
register long *count;
{
#ifdef DEBUG
  fprintf(stderr, "check_queue(block=%d, count=%x (*count=%d))\n",
    block, count, *count);
  fflush(stderr);
#endif

  pick_setkey(1);
  
  while (block && *count > 0)
  {
    *count -= 1;
    order_count++;
    
    op_rec->pi_pl  = oc->oi_tab[block - 1].oi_pl;
    op_rec->pi_on  = oc->oi_tab[block - 1].oi_on;
    op_rec->pi_mod = 1;
    
    pick_startkey(op_rec);

    op_rec->pi_mod = coh->co_prod_cnt;
    pick_stopkey(op_rec);
        
    begin_work();
    while (!pick_next(op_rec, NOLOCK))
    {
#ifdef DETAIL
  fprintf(stderr, "pl=%d  on=%d  mod=%d  quan=%d\n",
    op_rec->pi_pl, op_rec->pi_on, op_rec->pi_mod, op_rec->pi_ordered);
  fflush(stderr);
#endif

      if (op_rec->pi_mod < 1 || op_rec->pi_mod > coh->co_prod_cnt) continue;
      
      bin[op_rec->pi_mod - 1].bin_units += op_rec->pi_ordered;
      bin[op_rec->pi_mod - 1].bin_lines += 1;
    }
    commit_work();
    block = oc->oi_tab[block -1].oi_flink;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Count Using Remaining Picks
 *--------------------------------------------------------------------------*/
count_picks()
{
  register struct pw_item *i;
  register long k;
  
#ifdef DEBUG
  fprintf(stderr, "count_picks()\n");
#endif

  for (k = 0, i = pw; k < coh->co_products; k++, i++)
  {
    bin[k].bin_units = i->pw_units_to_go;
    bin[k].bin_lines = i->pw_lines_to_go;
  }
#ifdef DETAIL
  dump_bins();
#endif

  return 0;
}

/*--------------------------------------------------------------------------*
 *  Use existing productivity rates.
 *--------------------------------------------------------------------------*/
use_pr_rates()
{
  register struct zone_item *z;
  register long j, k;
  register struct hw_item  *h;
  register struct bay_item *b;
  float rate, x, y;

#ifdef DEBUG
  fprintf(stderr, "use_pr_rates()\n");
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    rate = 0.0;
    
    if (use_what == 1)                     /* current rates                  */
    {
      x = pz[k].pr_zone_cur_active;

      x = x / 3600.0;                      /* convert to hours               */

      if (x)
      {
        if (bal_type == 'l') y = pz[k].pr_zone_cur_lines; 
        else                 y = pz[k].pr_zone_cur_units;

        rate = y / x;
      }
    }
    else if (use_what == 2)                /* use cummulative rates          */
    {
      x = pz[k].pr_zone_cum_active;

      x = x / 3600.0;                      /* convert to hours               */

      if(x)
      {
        if (bal_type == 'l') y = pz[k].pr_zone_cum_lines;
        else                 y = pz[k].pr_zone_cum_units; 
   
        rate = y / x;
      }
    }
    for (j = 0, h = hw; j < coh->co_light_cnt; j++, h++)
    {
      if (!h->hw_bay) continue;
        
      if (h->hw_type == PM  || h->hw_type == PI ||
          h->hw_type == PM2 || h->hw_type == PM4 || h->hw_type == PM6)
      {
        b = &bay[h->hw_bay - 1];
        if (!b->bay_zone) continue;
         
        if (b->bay_zone == k+1)
        {
          pickrate[h->hw_mod - 1] = rate;
        }
      }
    }
  }
#ifdef DEBUG
  dump_pick_rates();
#endif
  return 0;
}
/*-------------------------------------------------------------------------*
 * read pick rate file
 *-------------------------------------------------------------------------*/
read_pick_rate_file(name)
register char *name;
{
  FILE *prt_fd;
  char work[64];
  register struct hw_item  *h;
  register struct bay_item *b;
  register long j, i, size;
   
  struct pick_rate_token
  {
    long  zone;
    float rate;
  };
  struct pick_rate_token *prt;

#ifdef DEBUG
  fprintf(stderr, "read_pick_rate_file(%x) = [%s]\n", name, name);
#endif

  sprintf(work, "dat/pick_rate/%s", name);

#ifdef DEBUG
  fprintf(stderr, "read_pick_rate_file work=[%s]\n", work);
#endif
  prt_fd = fopen(work, "r");
  if(prt_fd == 0) return 4;

  fseek(prt_fd, 0, 2);
  size = ftell(prt_fd);
  fseek(prt_fd, 0, 0);
  
  prt = (struct pick_rate_token *)malloc(size);
  if (!prt) krash("read_pick_rate_file", "malloc", 1);
  fread(prt, size, 1, prt_fd);
  fclose(prt_fd);   
   
  size = size / sizeof(struct pick_rate_token);

  memset(pickrate, 0, sizeof(float) * coh->co_modules);
  
  for (i = 0; i < size; i++)              /* assign pick rates               */
  {
    for (j = 0, h = hw; j < coh->co_light_cnt; j++, h++)
    {
      if (!h->hw_bay) continue;

      if (h->hw_type == PM  || h->hw_type == PI ||
          h->hw_type == PM2 || h->hw_type == PM4 || h->hw_type == PM6)
      {
        b = &bay[h->hw_bay - 1];
        if (!b->bay_zone) continue;
        
        if (b->bay_zone == prt[i].zone)
        {
          pickrate[h->hw_mod - 1] = prt[i].rate;
        }
      }
    }
  }
  free(prt);
  
#ifdef DEBUG
  dump_pick_rates();
#endif
  return 0;
}

/*-------------------------------------------------------------------------*
 * create a bay and zone tables
 *-------------------------------------------------------------------------*/
make_tables()
{
  register struct pl_item   *p;
  register struct zone_item *z;
  register struct bay_item  *b;
  register long k, m, n, last;

#ifdef DEBUG
  fprintf(stderr, "make_tables()\n");
#endif

  bin = (bin_item *)malloc(sizeof(bin_item) * coh->co_products);
  if (!bin) krash("make_tables", "alloc bin", 1);
  
  memset(bin, 0, sizeof(bin_item) * coh->co_products);
  
  mod = (mod_item *)malloc(sizeof(mod_item) * coh->co_modules);
  if (!mod) krash("make_tables", "alloc mod", 1);
  
  memset(mod, 0, sizeof(mod_item) * coh->co_modules);
  
  pickrate = (float *)malloc(sizeof(float) * coh->co_modules);
  if (!pickrate) krash("make_tables", "alloc pickrate", 1);
  
  memset(pickrate, 0, sizeof(float) * coh->co_modules);
  
  zn = (zn_item *)malloc(sizeof(zn_item) * coh->co_zones);
  if (!zn) krash("make_tables", "alloc zn", 1);
  
  memset(zn, 0, sizeof(zn_item) * coh->co_zones);
  
  pn = (pn_item *)malloc(sizeof(pn_item) * coh->co_picklines);
  if (!pn) krash("make_tables", "alloc pn", 1);
  
  memset(pn, 0, sizeof(pn_item) * coh->co_picklines);
  
  cn = (cn_item *)malloc(sizeof(cn_item) * NAMES);
  if (!cn) krash("make_tables", "alloc cn", 1);
  
  memset(cn, 0, sizeof(cn_item) * NAMES);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 * store data in a file
 *-------------------------------------------------------------------------*/
store_data(k)
register long k;
{
  static char err_mess[7][64] = {
  {"OK"},
  {"Invalid Configuration - see printer"},
  {"Configuration Failed"},
  {"Some Picks Not In Configuration"},
  {"No Pick Rate File"},
  {"Rates Missing For Some Modules"},
  {"Total Time Is Zero"}};
  
  register long i, j;
  long hours, mins, first;
  
#ifdef DEBUG
  fprintf(stderr, "store_data(%d) %s status=%d\n", 
    k, cn[k].cn_name, cn[k].cn_status);
#endif

  if (cn[k].cn_status)
  {
    fprintf(fd, "  %-20.20s  %-55.55s\n", 
      cn[k].cn_name, 
      err_mess[cn[k].cn_status]);
    return 0;
  }
#ifdef DEBUG
  dump_picklines();
  dump_zones();
#endif

/*
         1         2         3         4         5         6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
                                           Lines/           Units/
   Configuration Name   PL  Zone   Lines    Hour    Units    Hour    Time
  --------------------  --  ----  -------  ------  -------  ------  ------
  xxxxxxxxxxxxxxxxxxxx  xx  xxx   xxxxxx   xxxxx   xxxxxx   xxxxx    xx:xx 
*/
  first = 1;
  
  for (j = 0; j < coh->co_pl_cnt; j++)
  {
    
/*    if (pn[j].pn_lines < 1) continue;   */
    
    hour_convert(pn[j].pn_time, &hours, &mins);
    
    fprintf(fd, "  %-20.20s%4d%5d%9d%8.0f%9d%8.0f%6d:%02d     \n",
      first ? cn[k].cn_name : " ", j+1,
      pn[j].pn_worst_zone,
      pn[j].pn_lines,
      pn[j].pn_line_rate + 0.499,
      pn[j].pn_units,
      pn[j].pn_unit_rate + 0.499,
      hours, mins);

    first = 0;
  }
  for (j = 0; j < coh->co_pl_cnt; j++)
  {
/*   if (pn[j].pn_lines < 1) continue;   */

    first = 1;
    
    for (i = 0; i < coh->co_zone_cnt; i++)
    {
      if (zn[i].zn_pickline != j+1) continue;

      hour_convert(zn[i].zn_time, &hours, &mins);
    
      fprintf(cd, "  %-20.20s%4d%5d%9d%8.0f%9d%8.0f%6d:%02d     \n",
        first ? cn[k].cn_name : " ", j+1, i+1,
        zn[i].zn_lines,
        zn[i].zn_line_rate + 0.499,
        zn[i].zn_units,
        zn[i].zn_unit_rate + 0.499,
        hours, mins);
     
      first = 0;
    }
    fprintf(cd, "%-79.79s\n",
 "                        --  ----  -------  ------  -------  ------  ------");

    hour_convert(pn[j].pn_time, &hours, &mins);
    
    fprintf(cd, "  %-20.20s%4d%5d%9d%8.0f%9d%8.0f%6d:%02d %c   \n",
      "Pickline Totals:", j+1,
      pn[j].pn_worst_zone,
      pn[j].pn_lines,
      pn[j].pn_line_rate + 0.499, 
      pn[j].pn_units,
      pn[j].pn_unit_rate + 0.499,
      hours, mins,
      pn[j].pn_time > hour_limit ? '*' : 0x20);
      
   if (pn[j].pn_time > hour_limit)
   {
     fprintf(cd, "%-79.79s\n", "* Pickline Exceeds Time Limit");
   }
   fprintf(cd, "%79.79s\n", " ");
  }
  fflush(fd);
  fflush(cd);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 * convert hours to hours and minute
 *-------------------------------------------------------------------------*/
hour_convert(hrs, hour, min)
register float hrs;
register long *hour, *min;
{
  *hour = hrs;                            /* will round up                   */
  if (*hour > hrs) *hour -= 1;            /* round down                      */
  hrs -= *hour;                           /* fraction                        */
  *min = hrs * 60;
}
/*-------------------------------------------------------------------------*
 *function to display x number of lines of data on the screen               
 * Arguments:                                                               
 *           fp : the data file pointer.                                    
 *           lines : the number of lines to be displayed.                   
 *           i : the indicator of either going forward or                   
 *           backward on the file.                                          
 *-------------------------------------------------------------------------*/

show(fp,lines,index)
FILE *fp;
short lines,index;
{
  long position, size;
  char str[1920];

  memset(str, 0, 1920);

  position = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if(index == 1)
  {
    position = savefp - lines * WIDTH;
    if(position < 0) position = 0;
    savefp = position;

    fseek(fp, position, 0);
    fread(str, WIDTH, lines, fp);
    
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else if(index == 2)
  {
    if (position >= size) position = savefp;
    savefp = position;
    
    fseek(fp, position, 0);
    fread(str, WIDTH, lines, fp);
    
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else return(0);
}
/*-------------------------------------------------------------------------*
 * print the data
 *-------------------------------------------------------------------------*/
print()
{
  char print_file[16];
  long pid, status;
  
  fclose(cd); cd = 0;

  if (fork() == 0)                        /* if child process                */
  {
    close_all();
    execlp("prft", "prft", cd_name, tmp_name(print_file),
      "sys/report/config_select.h", 0);
      krash("print", "prft load", 1);
  }
  pid = wait(&status);
  if (pid < 0 || status) krash("print", "prft failed", 1);
  return 0;
}
#ifdef DEBUG
dump_zones()
{
  register long k;
  register zn_item *zi;
  
  fprintf(stderr, "Zone  Lines   Units    Time   LineRate UnitRate \n");
  fprintf(stderr, "---- ------- ------- -------- -------- --------\n");
  
  for (k = 0, zi = zn; k < coh->co_zone_cnt; k++, zi++)
  {
    fprintf(stderr, "%3d%8d%8d%10.3f%9.3f%9.3f\n",
      k+1, zi->zn_lines, zi->zn_units, zi->zn_time, 
      zi->zn_line_rate, zi->zn_unit_rate);
  }
  fprintf(stderr, "\n\n");
  return 0;
}
dump_picklines()
{
  register long k;
  register pn_item *pi;
  
  fprintf(stderr, "Pickline  Lines   Units    Time   Zn LineRate UnitRate\n");
  fprintf(stderr, "-------- ------- ------- -------- -- -------- --------\n");
  
  for (k = 0, pi = pn; k < coh->co_pl_cnt; k++, pi++)
  {
    fprintf(stderr, "%5d%10d%8d%10.3f%3d%9.3f%9.3f\n",
      k+1, pi->pn_lines, pi->pn_units, pi->pn_time, pi->pn_worst_zone,
      pi->pn_line_rate, pi->pn_unit_rate);
  
  }
  fprintf(stderr, "\n\n");
  return 0;
}
dump_bins()
{
  register long k;
  
  fprintf(stderr, "\n\n");
  fprintf(stderr, " Bin   Units  Lines\n");
  fprintf(stderr, "----- ------ ------\n");
  
  for (k = 0; k < coh->co_prod_cnt; k++)
  {
    fprintf(stderr, "%5d %6d %6d\n", 
      k+1, bin[k].bin_units, bin[k].bin_lines);
  }
  fprintf(stderr, "\n\n");
  return 0;
}
dump_pick_rates()
{
  register long k;
  
  fprintf(stderr, "\n\n");
  fprintf(stderr, " Mod    Rate\n");
  fprintf(stderr, "-----  --------\n");

  for (k = 0; k < coh->co_modules; k++)
  {
    if (pickrate[k] > 0.0)
    {
      fprintf(stderr, "%5d   %8.3f\n", k + 1, pickrate[k]);
    }
  }
  fprintf(stderr, "\n\n");
  return 0;
}
#endif
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  close_all();
  database_close();
  sd_close();
  if (fd) {fclose(fd); unlink(fd_name); fd = 0;}
  if (cd) {fclose(cd); unlink(cd_name); cd = 0;}
  execlp("pnmm", "pnmm", 0);
  krash("leave", "load pnmm", 1);
}
/*-------------------------------------------------------------------------*
 * open all files
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  oc_open();
  od_open();
  pr_open();
  
  tmp_name(fd_name);
  fd = fopen(fd_name, "w+");

  if (fd == 0) krash("open_all", "open tmp", 1);
  
  tmp_name(cd_name);
  cd = fopen(cd_name, "w+");

  if (cd == 0) krash("open_all", "open tmp", 1);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{
  ss_close();
  oc_close();
  od_close();
  pr_close();
  return 0;
}

/* end of config_select.c */

