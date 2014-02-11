/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create orders from co segment data.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 09/13/93    |  tjt Rewritten for mfc.
 * 02/24/95    |  tjt Bug in pick text generation.
 * 04/30/96    |  tjt Add stkloc as key.
 *-------------------------------------------------------------------------*/
static char create_orders_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "co.h"
#include "st.h"

#define random(x)    (rand() % (x))
#define randomize(x) (rand_r(&x))

FILE *fd;
char fd_name[40];
char order_filename[40];

char parm[16];                            /* general input area              */

long pickline;                            /* selected pickline               */
long low_order, high_order;               /* selected order range            */
long max_order;
long low_zone, high_zone;                 /* selected zone range             */
long picks;                               /* selected picks per order        */
long maxquan;                             /* selected quantity               */
char group[GroupLength + 1];              /* selected group                  */
char code;                                /* selected method                 */

short *list;                              /* list of integers                */
long  tot_sku;                            /* products in pickline            */
char  *sku;                               /* sku or mod text                 */
long  sku_len;                            /* sku or mod length               */

char *remarks;                            /* generated remarks               */
char *text;                               /* random text                     */

main(argc, argv)
long argc;
char **argv;
{
  register char *p;

  putenv("_=create_orders");              /* name to environ                 */
  p = (char *)getenv("HOME");             /* change to home directory        */
  if (p) chdir(p);

  ss_open();
  co_open();
    
  setbuf(stdin, NULL);                    /* Use unbuffered input            */
  setbuf(stdout, NULL);                   /* Use unbuffered output           */
    
  unsigned int rand_state = (time(0) % 1023);
  if (argc > 1) randomize(rand_state); // Weak pseudo random number generator state init.

  get_parms();
  generate_orders();

  ss_close();
  co_close();
  printf("\nAll Done\n\n");
}

/*-------------------------------------------------------------------------*
 *  Get Parameters
 *-------------------------------------------------------------------------*/
get_parms()
{
  register long k;

  printf("Enter Name Of Text File ---> ");
  gets(order_filename);

  strcpy(fd_name, "otext/");
  strcat(fd_name, order_filename);

  fd = fopen(fd_name, "w");
  if (fd == 0)
  {
    printf("Can't Open %s\n\n", fd_name);
    exit(1);
  }
  if (rf->rf_pl && coh->co_picklines > 1)
  {
    while (1)
    {
      printf("Enter Pickline          ---> ");
      gets(parm);
      
      pickline = atol(parm);

      if (!pickline) break;

      if (pickline >= 1 && pickline <= coh->co_picklines)
      {
        if (pl[pickline  - 1].pl_pl == pickline) break;
      }
      printf("*** Pickline Is Invalid\n\n");
    }
  }
  else pickline = 1;
        
  while (1)
  {
    printf("Enter Least Zone        ---> ");
    gets(parm);
    low_zone = atol(parm);

    if (!low_zone) break;

    if (zone[low_zone - 1].zt_pl != pickline)
    {
      printf("*** Zone Not In Pickline\n\n");
      continue;
    }
    printf("Enter Greatest Zone     ---> ");
    gets(parm);
    high_zone = atol(parm);
      
    if (low_zone > high_zone)
    {
      printf("*** Invalid Range\n\n");
      continue;
    }
    if (zone[high_zone - 1].zt_pl != pickline)
    {
      printf("*** Zone Not In Pickline\n\n");
      continue;
    }
    break;  
  }
  if (sp->sp_use_stkloc == 'y') make_stkloc_list(pickline);
  else if (rf->rf_sku)          make_sku_list(pickline);
  else                          make_mod_list(pickline);

  if (rf->rf_grp)
  {
    printf("Enter Group Name        ---> ");
    gets(parm);
           
    sprintf(group, "%-*.*s", rf->rf_grp, rf->rf_grp, parm);
  }
  while (1)
  {
    max_order = 1;

    for (k = 1; k <= rf->rf_on; k++) max_order *= 10;

    printf("Enter Low Order Number  ---> ");    
    gets(parm);

    low_order = atol(parm);

    if (low_order < 0 || low_order > max_order)
    {
      printf("*** Invalid Order Number\n\n");
      continue;
    }
    printf("Enter High Order Number ---> ");
    gets(parm);
    
    high_order = atol(parm);

    if (high_order < 1 || high_order > max_order)
    {
      printf("*** Invalid Order Number\n\n");
      continue;
    }
                
    if (low_order > high_order)
    {
      printf("*** Invalid Order Number Range\n\n");
      continue;
    }
    break;
  }
  while(1)
  {
    printf("\nSelect Method For Generation Of Picks\n");
    printf("-------------------------------------\n");
    printf("    1 - All Modules\n");
    printf("    2 - Odd/Even Modules\n");
    printf("    3 - Random Modules\n");
    printf("Enter Selection         ---> ");
    gets(parm);
    
    code = parm[0];
    if (code == '1' || code == '2' || code == '3') break;
    printf("*** Invalid Selection\n\n");
  }
  while (1)
  {
    printf("\nEnter Picks per Order   ---> ");
    gets(parm);
    
    picks = atol(parm);
    if (picks <= tot_sku) break;
    printf("*** Picks Exceeds Products\n\n");
  }
  while (1)
  {
    printf("\nEnter Max Quantity      ---> ");
    gets(parm);
    
    maxquan = atol(parm);

    if      (rf->rf_quan == 1 && maxquan > 9)  maxquan = 9;
    else if (rf->rf_quan == 2 && maxquan > 99) maxquan = 99;
    else if (maxquan > 999)                    maxquan = 999;

    if (maxquan > 0) break;
    printf("*** Must Be At Least One\n\n");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make Module List For Pickline
 *-------------------------------------------------------------------------*/
make_mod_list(n)
register long n;
{
  register struct bay_item *b;
  register struct zone_item *z;
  register long j, k, m;
  register char *p;
  
  for (k = tot_sku = 0, b = bay; k < coh->co_bay_cnt; k++, b++)
  {
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;
    
    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    tot_sku += (b->bay_prod_last - b->bay_prod_first + 1);
  }
  sku_len = rf->rf_mod;

  list = (short *)malloc(2 * tot_sku);
  if (!list)
  {
    printf("*** Cannot Allocate Module Table\n\n");
    exit(1);
  }
  p = sku = (char *)malloc((sku_len + 1) * tot_sku);
  if (!sku)
  {
    printf("*** Cannot Allocate SKU/Module Table\n\n");
    exit(1);
  }
  for (j = k = m = 0, b = bay; k < coh->co_bay_cnt; k++, b++)
  {
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;

    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    for (j = b->bay_prod_first; j <= b->bay_prod_last; j++)
    {
      if (m >= tot_sku) break;
       
      list[m++] = p - sku;
      sprintf(p, "%0*d", sku_len, j);
      p += (sku_len + 1);
    }
  }
  if (low_zone) printf("Zones %d To %d Selected And This Part Of\n", 
    low_zone, high_zone);
  printf("Pickline %d Has %d Modules\n\n", n, tot_sku);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make SKU List For Pickline
 *-------------------------------------------------------------------------*/
make_sku_list(n)
register long n;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct st_item   *s;
  register struct bay_item  *b;
  register struct zone_item *z;
  register char *p;
  register long j, k;
  
  for (k = tot_sku = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (s->st_mirror) continue;

    i = &pw[s->st_mod - 1];
    h = &hw[i->pw_ptr - 1];
    if (!h->hw_bay) continue;
    b = &bay[h->hw_bay - 1];
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;

    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    tot_sku++;
  }
  sku_len = rf->rf_sku;

  list = (short *)malloc(2 * tot_sku);
  if (!list)
  {
    printf("*** Cannot Allocate Module Table\n\n");
    exit(1);
  }
  p = sku = (char *)malloc((sku_len + 1) * tot_sku);
  if (!sku)
  {
    printf("*** Cannot Allocate SKU Table\n\n");
    exit(1);
  }
  for (j = k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (s->st_mirror) continue;
    
    i = &pw[s->st_mod - 1];
    h = &hw[i->pw_ptr - 1];
    if (!h->hw_bay) continue;
    b = &bay[h->hw_bay - 1];
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;

    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    if (j >= tot_sku) break;

    list[j++] = p - sku;
    memcpy(p, s->st_sku, sku_len);

    p += sku_len;
    *p++ = 0;
  }
  if (low_zone) printf("Zones %d To %d Selected And This Part Of\n", 
    low_zone, high_zone);
  printf("Pickline %d Has %d SKU's\n\n", n, tot_sku);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make Stkloc List For Pickline
 *-------------------------------------------------------------------------*/
make_stkloc_list(n)
register long n;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct st_item   *s;
  register struct bay_item  *b;
  register struct zone_item *z;
  register char *p;
  register long j, k;
  
  for (k = tot_sku = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (s->st_mirror) continue;

    i = &pw[s->st_mod - 1];
    h = &hw[i->pw_ptr - 1];
    if (!h->hw_bay) continue;
    b = &bay[h->hw_bay - 1];
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;

    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    tot_sku++;
  }
  sku_len = rf->rf_stkloc;

  list = (short *)malloc(2 * tot_sku);
  if (!list)
  {
    printf("*** Cannot Allocate Module Table\n\n");
    exit(1);
  }
  p = sku = (char *)malloc((sku_len + 1) * tot_sku);
  if (!sku)
  {
    printf("*** Cannot Allocate Stkloc Table\n\n");
    exit(1);
  }
  for (j = k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (s->st_mirror) continue;
    
    i = &pw[s->st_mod - 1];
    h = &hw[i->pw_ptr - 1];
    if (!h->hw_bay) continue;
    b = &bay[h->hw_bay - 1];
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (n && z->zt_pl != n) continue;

    if (low_zone)
    {
      if (z->zt_zone < low_zone) continue;
      if (z->zt_zone > high_zone) continue;
    }
    if (j >= tot_sku) break;

    list[j++] = p - sku;
    memcpy(p, s->st_stkloc, sku_len);

    p += sku_len;
    *p++ = 0;
  }
  if (low_zone) printf("Zones %d To %d Selected And This Part Of\n", 
    low_zone, high_zone);
  printf("Pickline %d Has %d Stkloc's\n\n", n, tot_sku);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Allocated Text
 *-------------------------------------------------------------------------*/
allocate_text()
{
  register long len, k;
  register char *p;

  len = rf->rf_rmks;
  if (rf->rf_pick_text > len) len = rf->rf_pick_text;
  if (len < 15) len = 15;
  
  len += 256;

  p = text = (char *)malloc(len);
  
  if (!text)
  {
    printf("*** Cannot Allocate Work Space\n\n");
    exit(1);
  }
  for (k = 0; k < len; k++, p++) *p = random(26) + 'a';

  if (rf->rf_rmks)
  {
    remarks = (char *)malloc(rf->rf_rmks);
    if (!remarks)
    {
      printf("*** Cannot Allocate Remarks Space\n\n");
      exit(1);
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make Remarks Text
 *-------------------------------------------------------------------------*/
make_remarks()
{
  register long k, m;
  register char *p;
  
  memcpy(remarks, text + random(256), rf->rf_rmks);

  if (!rf->rf_box_pos) return 0;
  
  p = remarks + rf->rf_box_pos;
  m = rf->rf_box_len * rf->rf_box_count;

  for (k = 0; k < m; k++, p++)
  {
    *p = random(10) + '0';
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Generate Orders
 *-------------------------------------------------------------------------*/
generate_orders()
{
  register long count, mod, odd_even, k, n;
  char work[32];
  char *rn;
  
  allocate_text();                        /* make any text areas             */

  odd_even = 0;
  rn = 0;

  if (low_order < 1)
  {
    low_order = 1;
    if (max_order > 99999) max_order = 99999;

    if (sp->sp_use_con != 'y')
    {
      rn = (char *)malloc(max_order);
      memset(rn, 0, max_order);
    }
  }

  for (; low_order <= high_order; low_order++)
  {
    fprintf(fd, "%c", rf->rf_rp);         /* preface byte                    */

    for (k = 0; k < rf->rf_con; k++)      /* customer order no.              */
    {
      fprintf(fd, "%c", random(26) + 'a');
    }
    if (rn)
    {
      while (1)
      {
        n = random(max_order) + 1;
        if (rn[n]) continue;
        rn[n] = 1;
        fprintf(fd,"%0*d", rf->rf_on, n);
        break;
      }
    }
    else if (sp->sp_use_con != 'y')
    {
      fprintf(fd,"%0*d", rf->rf_on, low_order);          /* order number     */
    }
    if (rf->rf_pri) fprintf(fd, "m");     /* priority                        */
      
    if (rf->rf_grp)
    {
      if (*group == '-')
      {
        for (k = 0; k < rf->rf_grp; k++) fprintf(fd, "%c", 'a' + random(26));
      }
      else fwrite(group, rf->rf_grp, 1, fd); /* group                        */
    }
    if (rf->rf_pl)
    {
      fprintf(fd, "%0*d", rf->rf_pl, pickline);/* pickline                   */
    }
    if (rf->rf_rmks)
    {
      make_remarks();
      fwrite(remarks, rf->rf_rmks, 1, fd);/* remarks                         */
    }
    fprintf(fd, "\n");

    count = picks;

    if (code == '1')
    {
      if (picks < 1) count = tot_sku;     /* do all sku                      */
      mod = 0;
    }
    else if (code == '2')
    {
      odd_even ^= 1;
      mod = odd_even;
      if (picks < 1) count = tot_sku - mod + 1;
      count = count / 2;
    }
    else if (code == '3')
    {
      if (picks < 1) count = random(tot_sku) + 1;

      for (k = 0; k < tot_sku; k++)       /* shuffle list randomly           */
      {
        n = random(tot_sku);
        if (n == k) continue;
        mod     = list[k];                /* swap n and k items              */
        list[k] = list[n];
        list[n] = mod;
      }
      mod = 0;
    }
    for (; count > 0; count--)
    {
      fprintf(fd, "%-*.*s", sku_len, sku_len, sku + list[mod]);

      if (maxquan == 1)
      {
        fprintf(fd, "%0*d", rf->rf_quan, 1);
      }
      else if (rf->rf_quan > 2)
      {
        fprintf(fd, "%0*d", rf->rf_quan, random(maxquan) + 1);
      }
      else if (rf->rf_quan == 2) fprintf(fd, "%02d", random(maxquan) + 1);
      else                       fprintf(fd, "%01d", random(maxquan) + 1);
      
      if (rf->rf_pick_text)
      {
        fwrite(text + random(256), rf->rf_pick_text, 1, fd);
      }
      fprintf(fd, "\n");
      
      if (code == '2') mod += 2;
      else             mod += 1;
    }
    if (rf->rf_rt) fprintf(fd, "%c", rf->rf_rt);
  }
  if (rf->rf_eof)
  {
    fprintf(fd, "%c", rf->rf_eof);
  }
  else
  {
    if (rf->rf_rt)
    {
      fprintf(fd, "%c%c", rf->rf_rp, rf->rf_rt);
    }
    else
    {
      fprintf(fd, "%c", rf->rf_rp);
      fprintf(fd, "%c", rf->rf_rp);
    }
  }
  fclose(fd);
}


/* end of create_orders.c */
