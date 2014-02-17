/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration compilation.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/26/93   |  tjt  Original implementation.
 *  03/17/94   |  tjt  Added area controllers.
 *  04/05/94   |  tjt  Added AC for area controllers.
 *  05/10/94   |  tjt  Allow first zone of pickline to be early exit.
 *  05/12/94   |  tjt  Don't print data line at EOF on error message.
 *  05/12/94   |  tjt  Bay modules must be on same controller.
 *  07/06/94   |  tjt  Basic function controller is a bay.
 *  07/07/94   |  tjt  Add error messages (-m option).
 *  09/02/94   |  tjt  Add bay_zc is controller address in basic function.
 *  10/07/94   |  tjt  Allow master bay lamp without bay lamp.
 *  11/14/94   |  tjt  Add pickline segments.
 *  01/23/95   |  tjt  Add co_pl_config - actual pickline count.
 *  03/16/95   |  tjt  Add four digit pick modules.
 *  04/25/95   |  tjt  Add TC allowed without range.
 *  04/28/95   |  tjt  Add dummy port type.
 *  05/18/95   |  tjt  Revise basic function bay module check.
 *  06/17/95   |  tjt  Add init port count when validate.
 *  09/15/95   |  tjt  Add HasBoxFull.
 *  09/21/95   |  tjt  Add check for box feature.
 *  04/16/96   |  tjt  Remove cluster zones.
 *  05/26/96   |  tjt  Add box full for total function.
 *  07/05/96   |  tjt  Add promote HasBoxFull flag to zone above bay.
 *  07/07/96   |  tjt  Add early exit in branched picklines.
 *  07/09/96   |  tjt  Add check light limit on simulated lights.
 *  10/10/96   |  tjt  Add clear matrix display to spaces.
 *  10/12/96   |  tjt  Add generate matrix display when no sku support.
 *  11/29/96   |  tjt  Add save select config to tmp directory.
 *  12/02/96   |  tjt  Add -s option for selection.
 *  01/06/97   |  tjt  Add pickline check for sg_pl.
 *  01/07/97   |  tjt  Add PM6 module.
 *  05/25/98   |  tjt  Fix allow BF without AC if total function.
 *  05/26/98   |  tjt  Add IO Module for scanner and SCAN specification.
 *  05/26/98   |  tjt  Fix allow BF without ZC in bay notation.
 *-------------------------------------------------------------------------*/
static char configure_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *
 *  Build Configuration Tables.   
 *
 *    configure  [configuration name]   
 *               [-v]                   validate only 
 *               [-s]                   configuration selection
 *               [-m]                   send kernel messages
 *
 *    return 0 = valid configuration
 *           1 = invalid parms
 *           2 = invlaid config
 *-------------------------------------------------------------------------*
 *                          D E F I N I T I O N S
 *
 * tc              - a basic function terminal controller on RS422 line.
 *
 * pi              - a pick indicator light attached to a tc.
 *
 * pm              - a full function pick module with 2 position display
 *                   and a switch.
 *
 * zc              - a full function zone controller with 5 positon display and
 *                   2 switches
 *
 * bl              - a full function bay lamp.
 *
 * light           - a full function bl, zc, or pm - also a basic function pi.
 *
 * port            - a RS422 serial port of special full function board port.
 *                   These ports are assigned an arbitrary order by ss_init 
 *
 * hwix            - a number 1 .. n assigned to each light across all ports 
 *                   in the order specified by ss_init.
 *
 * module          - a number 1 .. m assigned to each pm or pi in hwix order.
 *                   
 * product         - a number 1 .. p assign to each bin location. Product 
 *                   numbers and module numbers are the same when carousels
 *                   are not present.  With carousels, there are more products
 *                   (bins) than there are modules (pm/pi).
 *
 * pickline        - one or more zones.
 *
 * zone            - one or more bays.    
 *
 * bay             - a basic function bay is a tc with associated
 *                   pi's and bay lamp.
 *
 * bay             - a full function bay is a contiguous group of zero 
 *                   or more pm's, a bay lamp and an optional zone controller.
 *
 * master bay lamp - a bay consisting of a single bay lamp.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 *             C O N F I G U R A T I O N    R U L E S
 *
 * 1. Bays definitions may be placed anywhere in the config file in
 *    any order, numbered 1 .. m with gaps allowed.
 *
 * 2. Bays are numbered in hwix order of their lights.
 *
 * 3. Zone definitions may be placed anywhere in the config file in any
 *    order, numbered 1 .. n with gaps allowed.
 *
 * 4. Zones are numbered so that order flow is always from lower numbered
 *    zones to higher numbered zones.
 *
 * 5. A full function bay may have zero or one bay lamp.
 *
 * 6. A full function bay may have zero or one zone controller.
 *
 * 7. An area controller bay may have zero or one bay lamp.
 *
 * 8. An area controller bay may have zero of one zone controller.
 *
 * 9. A full function or area controller zone must have one or more 
 *    zone controllers.  
 *
 * 10. Mixed configurations require port designations.
 *
 *-------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_names.h"
#include "global_types.h"
#include "message_types.h"
#include "co.h"
#include "plc.h"
#include "ss.h"

extern unsigned char gnc();
extern leave();

#define END    0x7f

FILE *fd = 0;
char fd_name[64];                         /* input configuration name        */
char config_name[32];                     /* raw configuration name          */

FILE *cd;
char cd_name[64];                         /* output configuration name       */

FILE *ed;
char ed_name[16];                         /* error file name                 */

#define LINE 128

unsigned char buffer[LINE + 1] = {0};     /* input line biffer               */
unsigned char backup[LINE] = {0};         /* backup buffer                   */

unsigned char *next = buffer;             /* next input character            */
unsigned char *last = backup;             /* next backup character           */

long line   = 0;                          /* input line number               */
long errors = 0;                          /* error message count            

long co_size = 0;                         /* length of co segment            */
long live    = 1;                         /* using real co segment           */
long select0  = 0;                        /* do not delete output.           */
                                          /* append 0 to name. cannot use select as a name */
long send_errors = 0;                     /* error messages                  */
long port    = 1;                         /* simulated port                  */
long port_function = 0;                   /* current port type               */

unsigned long Functions = IsBasicFunction | IsFullFunction | IsDummy |
                          IsTotalFunction | IsCarousel | IsPut ;

long pickline = 0;                        /* current pickline                */
long segment;                             /* current segment                 */

#define SAMS  6

char *sam[SAMS] = {
  "DZM"                                   /* short to zero                   */
  "DNM",                                  /* normal pick to blank + NEXT     */
  "DSM",                                  /* short picking + NEXT            */
  "EZM",
  "ENM",
  "ESM"};

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  
  putenv("_=configure");                  /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */

  if (argc < 2)                           /* must have config file names     */
  {
    krash("main", "No Configuration Name Parameter", 0);
    exit(1);
  }
  strcpy(config_name, argv[1]);           /* save name of config             */
  sprintf(fd_name, "config/%s", argv[1]); /* make config file name           */
  sprintf(cd_name, "tmp/%s.co", argv[1]); /* F112996                         */
  
  for (k = 2; k < argc; k++)
  {
    if (strcmp(argv[k], "-v") == 0)      live = 0;     /* validate only      */
    else if (strcmp(argv[k], "-s") == 0) {live = 0; select0 = 1;} /* selection*/
    else if (strcmp(argv[k], "-m") == 0) send_errors = 1;
    else 
    {
      krash("main", "Invalid Parameter", 0);
      exit(1);
    }
  }
  open_all_files();                       /* open all files                  */

  if (sp->sp_full_function  != 'n' && 
      sp->sp_basic_function == 'n' && 
      sp->sp_total_function == 'n') port_function = IsFullFunction;

  if (sp->sp_full_function  == 'n' && 
      sp->sp_basic_function != 'n' && 
      sp->sp_total_function == 'n') port_function = IsBasicFunction;

  if (sp->sp_full_function  == 'n' && 
      sp->sp_basic_function == 'n' && 
      sp->sp_total_function != 'n') port_function = IsTotalFunction;

  if (!collect_bays())      leave(2);     /* get all bays first              */
  if (!collect_zones())     leave(2);     /* get all zones                   */
  if (!collect_attr())      leave(2);     /* get late entry etc.             */
  if (!collect_picklines()) leave(2);     /* get order flow                  */

  if (coh->co_pl_cnt <= 0)                /* empty file ???                  */
  {
    message("No Pickline Defined In Configuration");
    leave(2);
  }
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Error Message
 *-------------------------------------------------------------------------*/
message(p)
register char *p;                          /* error mesage                   */
{
  register long k;
  char text[80];
  
  k = next - buffer + 5;

  fprintf(ed, "*** %s\n", p);
 
  if (*next != END)                        /* F051294 - no data here         */
  {
    fprintf(ed, "%*.*sV\n", k, k, " ");
    fprintf(ed, "%4d: %s\n", line, buffer);
  }
  errors++;

  if (send_errors)
  {
    sprintf(text, "Configuration Failed %s\n", p);
    message_put(0, InitErrorMessageEvent, text, strlen(text));

    if (*next != END)
    {
      sprintf(text, "Line %d: %s\n", line, buffer);
      message_put(0, InitErrorMessageEvent, text, strlen(text));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Compile Input Configuration File - Collect Bays
 *-------------------------------------------------------------------------*/
collect_bays()
{
#ifdef DEBUG
  fprintf(stderr, "collect_bays()\n");
#endif

  fseek(fd, 0, 0);
  line = 0;
  flush();

  while (1)
  {
    if (get_symbol(END))   break;
    if (get_symbol('\n'))  continue;
    if (get_symbol('Z'))   {flush(); continue;}
    if (get_symbol('P'))   {flush(); continue;}
    if (get_symbol('M'))   {flush(); continue;}
    if (get_symbol('N'))   {flush(); continue;}
    
    if (get_symbol('B'))
    {
      if (get_bay()) continue;
      return 0;
    }
    if (get_symbol('<')) 
    {
      if (get_port()) continue;
    }
    flush();
  }
  process_bay();
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Compile Input Configuration File - Collect Zones and Master Bay Lamps
 *-------------------------------------------------------------------------*/
collect_zones()
{
#ifdef DEBUG
  fprintf(stderr, "collect_zones()\n");
#endif

  fseek(fd, 0, 0);
  line = 0;
  flush();

  while (1)
  {
    if (get_symbol(END))   break;
    if (get_symbol('\n'))  continue;
    if (get_symbol('B'))   {flush(); continue;}
    if (get_symbol('P'))   {flush(); continue;}
    if (get_symbol('N'))   {flush(); continue;}
    if (get_symbol('<'))   {flush(); continue;}

    if (get_symbol('Z'))
    {
      if (get_zone())      continue;
      return 0;
    }
    if (get_word("MB"))
    {
      if (get_mbl()) continue;
      return 0;
    }
    flush();
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Compile Input Configuration File - Zone Attributes
 *-------------------------------------------------------------------------*/
collect_attr()
{
#ifdef DEBUG
  fprintf(stderr, "collect_attr()\n");
#endif

  fseek(fd, 0, 0);
  line = 0;
  flush();

  while (1)
  {
    if (get_symbol(END))   return 1;
    if (get_symbol('\n'))  continue;
    if (get_symbol('B'))   {flush(); continue;}
    if (get_symbol('Z'))   {flush(); continue;}
    if (get_symbol('<'))   {flush(); continue;}
    if (get_symbol('P'))   {flush(); continue;}
    if (get_symbol('N'))   {flush(); continue;}

    if (get_word("EEMODE")){flush(); continue;}

    if (get_le())          continue;
    if (get_ee())          continue;
    if (get_st())          continue;
    if (get_jz())          continue;
    if (get_si())          continue;
    
    flush();
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Compile Input Configuration File - Collect Picklines
 *-------------------------------------------------------------------------*/
collect_picklines()
{
#ifdef DEBUG
  fprintf(stderr, "collect_picklines()\n");
#endif

  fseek(fd, 0, 0);
  line = 0;
  flush();

  while (1)
  {
    if (get_symbol(END))   break;
    if (get_symbol('\n'))  continue;
    if (get_symbol('B'))   {flush(); continue;}
    if (get_symbol('C'))   {flush(); continue;}
    if (get_symbol('Z'))   {flush(); continue;}
    if (get_symbol('<'))   {flush(); continue;}

    if (get_pickline())    continue;
    if (get_eemode())      continue;
    if (get_flow())        continue;

    if (get_all("EE", EarlyExit))  continue;
    if (get_all("LE", LateEntry))  continue;
    if (get_all("ST", Steering))   continue;
    if (get_all("JZ", JumpZone))   continue;
    if (get_all("SI", DemandFeed)) continue;

    if (get_word("MB")) {flush(); continue;}

    if (get_sam()) continue;
    
    message("Unrecognized", 0);
    flush();
  }
  return check_pickline();
}
/*-------------------------------------------------------------------------*
 *  Check Last Pickline Entered
 *-------------------------------------------------------------------------*/

check_pickline()
{
  register struct pl_item *p;
  register struct seg_item *s;
  register struct zone_item *z;
  register long j, k, last_zone, first_zone, found;
  char text[80];

#ifdef DEBUG
  fprintf(stderr, "check_pickline()\n");
#endif

  if (!pickline) return 1;
  p = &pl[pickline - 1];
  pickline = 0;
  
  if (p->pl_last_zone < 1)
  {
    sprintf(text, "Pickline %d Has No Zones", p->pl_pl);
    message(text);
    return 1;
  }
  for (k = 0; k < coh->co_seg_cnt; k++)      /* find first segment           */
  {
    if (sg[k].sg_pl != p->pl_pl) continue;

    for (j = k + 1; j < coh->co_seg_cnt; j++)
    {
      if (sg[k].sg_snode == sg[j].sg_snode)
      {
        sg[k].sg_below = j + 1;
        break;
      }
    }
    if (sg[k].sg_enode != 99) 
    {
      for (j = 0; j < coh->co_seg_cnt; j++)
      {
        if (j == k) continue;
        if (sg[j].sg_pl != p->pl_pl) continue;  /* F010697 */
        
        if (sg[k].sg_enode == sg[j].sg_snode)
        {
          sg[k].sg_next = j + 1;
          break;
        }
      }
      if (j >= coh->co_seg_cnt) return message("Too Many Segments");
    }
    if (sg[k].sg_snode)
    {
      for (j = 0; j < coh->co_seg_cnt; j++)
      {
        if (j == k) continue;
        if (sg[j].sg_pl != p->pl_pl) continue;  /* F010697 */
        
        if (sg[k].sg_snode == sg[j].sg_enode)
        {
          sg[k].sg_prev = j + 1;
          break;
        }
      }
      if (j >= coh->co_seg_cnt) return message("Too Many Segments");
    }
  }
  for (k = 0; k < coh->co_seg_cnt; k++)
  {
    if (sg[k].sg_pl != p->pl_pl) continue;

    j = sg[k].sg_below - 1;
    
    if (j >= 0)
    {
      if (sg[j].sg_first_zone <= sg[k].sg_last_zone)
      {
        sprintf(text, "Zone %d Cannot Feed Zone %d", 
          sg[k].sg_last_zone, sg[j].sg_first_zone);
        return message(text);
      }
    }
    j = sg[k].sg_next - 1;
    if (j >= 0)
    {
      if (sg[j].sg_first_zone <= sg[k].sg_last_zone)
      {
        sprintf(text, "Zone %d Cannot Feed Zone %d", 
          sg[k].sg_last_zone, sg[j].sg_first_zone);
        return message(text);
      }
    }
  }
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++)
  {
    if (sg[k - 1].sg_pl != p->pl_pl) continue;

    if (!sg[k - 1].sg_snode)             /* flag first zones                 */
    {
      zone[sg[k - 1].sg_first_zone - 1].zt_flags &= ~LateEntry;         
      zone[sg[k - 1].sg_first_zone - 1].zt_flags |= FirstZone;
    }
    if (sg[k - 1].sg_enode == 99)        /* flag last zones                  */
    {
      zone[sg[k - 1].sg_last_zone  - 1].zt_flags &= ~Steering;
      if (!(p->pl_flags & EarlyExitModeNext))
      {
        zone[sg[k - 1].sg_last_zone  - 1].zt_flags &= ~EarlyExit;
      }
    }
  }
  if (p->pl_first_segment != p->pl_last_segment)
  {
    p->pl_flags |= IsSegmented;
    z = &zone[p->pl_first_zone - 1];

    for (k = p->pl_first_zone; k <= p->pl_last_zone; k++, z++)
    {
      if (z->zt_pl != p->pl_pl) continue;
      z->zt_flags |= IsSegmented;
/*    z->zt_flags &= ~EarlyExit;   F070796  allow early exit */
      if (sg[z->zt_segment - 1].sg_snode || sg[z->zt_segment - 1].sg_below) 
      {
        z->zt_flags &= ~LateEntry;
      }
    }
  }
  z = &zone[p->pl_first_zone - 1];

  first_zone = p->pl_first_zone;
  
  for (k = p->pl_first_zone; k <= p->pl_last_zone; k++, z++)
  {
    if (z->zt_pl != p->pl_pl) continue;
    
    if (z->zt_flags & JumpZone) first_zone = k;
    z->zt_start_section = first_zone;
    z->zt_end_section   = p->pl_last_zone;

    found = k;
    
    for (j = k + 1; j <= p->pl_last_zone; j++)
    {
      if (zone[j - 1].zt_pl != p->pl_pl) continue;

      if (zone[j - 1].zt_flags & JumpZone) 
      {
        z->zt_end_section = found;
        break;
      }
      found = j;
    }
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get pickline
 *-------------------------------------------------------------------------*
 *  P[0-9]+ = ['] [^'\n]* [']  $
 *-------------------------------------------------------------------------*/

get_pickline()
{
  register struct pl_item *p;
  register unsigned char c;
  register long k, n;

#ifdef DEBUG
  fprintf(stderr, "get_pickline()\n");
#endif

  if (!get_symbol('P')) return 0;

  check_pickline();                       /* finish and check last entry     */

  if ((pickline = get_number()) <= 0) 
  {
    return message("Pickline Number Expected");
  }
  if (pickline > coh->co_picklines) 
  {
    return message("Pickline Number Too Large");
  }
  p = &pl[pickline - 1];                   /* point to pickline              */

  if (p->pl_pl) return message("Duplicate Pickline");

  p->pl_pl = pickline;                    /* setup pickline information      */
  strncpy(p->pl_sam, "DSM", 3);           /* default switch action           */

  if (pickline > coh->co_pl_cnt) coh->co_pl_cnt = pickline;

  if (!get_symbol('='))  {message("Equals Expected"); flush(); return 1;}
  if (!get_symbol('\'')) {message("Quote Expected", 0); flush(); return 1;}

  k = 0;
  while (1)
  {
    c = gnc();
    if (c == '\'' || c == '\n') break;
    if (k < 8) p->pl_name[k++] = c;
  }
  p->pl_name[k] = 0;

  if (!get_symbol('\n')) {message("End Of Line Expected"); flush();}
  
#ifdef DEBUG
  fprintf(stderr, "Pickline=%d [%s]\n", p->pl_pl, p->pl_name);
  Bdump(p, sizeof(struct pl_item));
#endif

  coh->co_pl_config += 1;                /* count picklines in configuration */
  
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Port Definition - Required On Mixed Configurations
 *-------------------------------------------------------------------------*
 *  < [0-9]+ = [FF | BF | AC | TF | DF]
 *-------------------------------------------------------------------------*/
get_port()
{
  register long k;
  register unsigned char c;
  
#ifdef DEBUG
  fprintf(stderr, "get_port()\n");
#endif

  port = get_number() + 1;

  if (port < 1)                return message("Port Number Expected");
  if (port > coh->co_port_cnt) return message("Port Number Too Large");

  if (!po[port - 1].po_flags)
  {
    if (live) return message("No Port Driver Program");
  }
  if (!get_symbol('=')) return message("Equals Expected");

  if (get_word("FF"))
  {
    if (live && !(po[port - 1].po_flags & IsFullFunction))
    {
      return message("Not A Full Function Port");
    }
    port_function = IsFullFunction;
  }
  else if (get_word("BF")) 
  {
    if (live && !(po[port - 1].po_flags & IsBasicFunction))
    {
      return message("Not A Basic Function Port");
    }
    port_function = IsBasicFunction;
  }
  else if (get_word("AC") || get_word("TF")) 
  {
    if (live && !(po[port - 1].po_flags & IsTotalFunction))
    {
      return message("Not A Total Function Port");
    }
    port_function = IsTotalFunction;
  }
  else if (get_word("DF")) 
  {
    if (live && !(po[port - 1].po_flags & IsDummy))
    {
      return message("Not A Dummy Port");
    }
    port_function = IsDummy;
  }
  else return message("Expecting Port Type");

#ifdef DEBUG
  fprintf(stderr, "found port = %d\n", port);
#endif
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Bay Definition
 *-------------------------------------------------------------------------*
 *  (scanner) = SCAN (,OI |,ZI |,TOTE |,BOX |,LOT |,n+)
 *  
 *  
 *  B[0-9]+ (-B[0-9]+)? = [0-9]+ (,[0-9]+)? (,T[0-9]+(-T[0-9]+)?)? (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = [0-9]+ (,[0-9]+)? (,BL)? (,ZC)? (,BF)? (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = [0-9]+ (,[0-9]+)? (,BL)? (,AC)? (,BF)? (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = BL (,ZC|,AC)? (,BF)? (,scanner)? (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = ZC|AC (,BF) (,scanner)? (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = BF (,scanner) (,X)? $
 *  B[0-9]+ (-B[0-9]+)? = scanner (,X)? $
 
 *-------------------------------------------------------------------------*/

get_bay()
{
  register struct bay_item *b;
  register long j, k, m, n;
  unsigned long flags;
  long blow, bhigh, prod, mod, tclow, tchigh;
  long need_comma;								/* F052698								  */
  long fscan;										/* F052698								  */
  
#ifdef DEBUG
  fprintf(stderr, "get_bay()\n");
#endif

  *last++ = 'B';

  prod = mod = tclow = tchigh = 0;
  
  flags = port_function;                  /* default hardware function       */

  if (!port_function)
  {
    return message("Define Ports In Mixed System");
  }
  if (!get_bay_range(&blow, &bhigh)) return 0;

  for (k = blow, b = &bay[blow - 1]; k <= bhigh; k++, b++)
  {
    if (b->bay_number) return message("Duplicate Bay");
  }
  if (!get_symbol('=')) return message("Equals Expected");

  need_comma = 0;									/* F052698 - not needed yet		  */
  fscan      = 0;									/* F052698 - none yet				  */
  
  if (get_module_range(&prod, &mod))		/* has products/modules      		  */
  {
    need_comma = 1;								/* F052698 - now needed				  */
    
    if (prod < mod) return message("Products Less Than Modules");

    if (prod > mod)
    {
      if (sp->sp_multibin_lights != 'y')
      {
        return message("Multibin Feature Required");
      }
      if (prod % mod)
      {
        return message("Products Not Multiple of Modules");
      }
      flags |= VertLights;                /* matrix picking by default       */
    }
    flags |= HasModules;

    if (get_symbol(','))
    {
      if (get_tc_range(&tclow, &tchigh)) 
      {
        if (sp->sp_basic_function != 'y' || sp->sp_basic_function != 's')
        {
          flush();
          message("Basic Function Feature Required");
        }
        if (!(port_function & IsBasicFunction))
         {
          flush();
          return message("Basic Function Port Required");
        }
        if ((tclow > 0) && ((bhigh - blow) != (tchigh - tclow)))
        {
          flush();
          return message("Bay And TC Ranges Not Same Length");
        }
      }
      else *last++ = ',';						/* replace comma	F052698			  */
    }
  }
  while (1)											/* F052698 - until new line		  */
  {
    if (need_comma) 								/* have something already			  */
    {
      if (!get_symbol(','))					/* need a comma or line feed		  */
      {
        if (get_symbol('\n')) break;		/* is end on a line feed			  */
        
        message("Comma Expected");	 		/* error message						  */
        flush();									/* clear input line					  */
        break;									 	/* comma missing						  */
      }
      if (!(flags & HasBayLamp))
      {
        if (get_bl(&flags)) continue;		/* found a bay lamp					  */
      }
      if (!(flags & (HasZoneController | IsMasterBayLamp)))
      {
        if (get_zc(&flags)) continue;		/* found a zone controller			  */
      }
      if (!(flags & (HasBoxFull | IsMasterBayLamp)))
      {
        if (get_bf(&flags)) continue;		/* found box full						  */
      }
      if (!(flags & HasIOModule))
      {
        fscan = get_scan(&flags);
        if (fscan) continue;					/* found a scanner					  */
      }
      if (flags & VertLights) 
      {
        if (get_mc(&flags)) 					/* need matrix specification		  */
        {
          if (get_symbol('\n')) break;		/* got specification					  */
        
          message("End of Line Expected"); 
          flush();
          break;
        }
        message("M,C,V,H or X Expected");	/* error in specification			  */
        flush();
        break;
      }
      if (get_x(&flags)) 						/* found an X barrier code			  */
      {
        if (get_symbol('\n')) break;		/* end of line	after (,X)			  */
      
        message("End of Line Expected");
        flush();
        break;
      }
      message("Unrecognized");
      flush();
      break;
    }
    else												/* no products in bay				  */
    {
      if (!get_blm(&flags))					/* master bay lamp only				  */
      {
        if (!get_zc(&flags))					/* master zone controller			  */
        {
          if (!get_bf(&flags))				/* box full module					  */
          {
            fscan = get_scan(&flags);		/* scanner only	  				  */
            if (!fscan)
            {
              flush();
              return message("Unrecognized");
            }
          }
        }
      }
      need_comma = 1;
    }
  }
  for (k = blow, b = &bay[blow - 1]; k <= bhigh; k++, b++)
  {
    b->bay_number = k;
    b->bay_flags |= flags;

    b->bay_prod_first = prod;
    b->bay_prod_last  = prod;

    b->bay_mod_first  = mod;
    b->bay_mod_last   = mod;
        	
    /* F052698	- save scanner application in bay_state for xfer to hw		  */

    if (b->bay_flags & HasIOModule) b->bay_state = fscan;	/* F052698 */
        
    if (flags & IsFullFunction)
    {
      if (sp->sp_full_function == 's' || !live)
      {
        if (flags & HasBayLamp)        add_module(port, BL, k, 0, 0);
        if (flags & HasZoneController) add_module(port, ZC, k, 0, 0);
        for (j = 0; j < mod; j++)      add_module(port, PM, k, 0, 0);
      }
    }
    else if (flags & IsBasicFunction)
    {
      if (sp->sp_basic_function == 's' || !live)
      {
        po[port - 1].po_controllers += 1;

        for (j = 0; j < mod; j++) 
        {
          add_module(port, PI, po[port - 1].po_controllers, j + 1, 0);
        }
      }
    }
    else if (flags & IsTotalFunction)
    {
      if (sp->sp_total_function == 's' || !live)
      {
        po[port - 1].po_controllers += 1;
        m = po[port - 1].po_controllers;
        
        n = 0;
        if (flags & HasZoneController) add_module(port, ZC2, m, ++n, 0);
        if (flags & HasIOModule)			add_module(port, IO,  m, ++n, fscan);
        if (flags & HasBayLamp)			add_module(port, BL,  m, ++n, 0);
        if (flags & HasBoxFull)			add_module(port, BF,  m, ++n, 0);
        if (mod == 1)                  add_module(port, PM6, m, ++n, 0);
        else
        {
          for (j = 0; j < mod; j++)    add_module(port, PM4, m, j + n + 1);
        }
      }
    }
    else if (flags & IsDummy)
    {
      for (j = 0; j < mod; j++)        add_module(port, PM, k, 0);
    }
#ifdef DEBUG
    fprintf(stderr, "Bay %d\n", blow);
    Bdump(b, sizeof(struct bay_item));
#endif
  }
  b->bay_state = 0;									/* F042698 - make clear			  */
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get BL
 *-------------------------------------------------------------------------*/
get_bl(flags)
register unsigned long *flags;
{
#ifdef DEBUG
  fprintf(stderr, "get_bl() flags=%08x\n", *flags);
#endif
  
  if (!get_word("BL")) return 0;

  if (!(port_function & (IsFullFunction | IsTotalFunction)))
  {
    return message("Need Port Which Supports BL");
  }
  *flags |= HasBayLamp;
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get MBL
 *-------------------------------------------------------------------------*/
get_blm(flags)
register unsigned long *flags;
{
#ifdef DEBUG
  fprintf(stderr, "get_blm() flags=%08x\n", *flags);
#endif

  if (!get_word("BL")) return 0;
/*  
  {
    return message("Expecting Master Bay Lamp");
  }
*/
  if (sp->sp_master_bay_lamps != 'y')
  {
    message("Master Bay Lamp Feature Required");
  }
  if (!(port_function & (IsFullFunction | IsTotalFunction)))
  {
    return message("Need Port Which Supports BL");
  }
  *flags |= (HasBayLamp | IsMasterBayLamp);

  return 1;
}
/*-------------------------------------------------------------------------*
 * Get Zone Controller -  ( ZC | AC )? 
 *-------------------------------------------------------------------------*/
get_zc(flags)
register unsigned long *flags;
{
#ifdef DEBUG
  fprintf(stderr, "get_zc() flags=%08x\n", *flags);
#endif

  if (!get_word("ZC"))
  {
    if (!get_word("AC")) return 0;
  }
  if (!(port_function & (IsFullFunction | IsTotalFunction)))
  {
    return message("Need Port Which Supports Zone Controllers");
  }
  *flags |= HasZoneController;

  return 1;
}
/*-------------------------------------------------------------------------*
 * Get Box Full  -  (BF)?
 *-------------------------------------------------------------------------*/
get_bf(flags)										/* F052698								  */
register unsigned long *flags;
{
#ifdef DEBUG
  fprintf(stderr, "get_bf() flags=%08x\n", *flags);
#endif

  if (!get_word("BF")) return 0;
  
  if (sp->sp_box_full == 'n')
  {
    return message("Need Box Feature");
  }
  if (!(port_function & (IsFullFunction | IsTotalFunction)))
  {
    return message("Need Port Which Supports Box Full");
  }
  if (*flags & IsFullFunction) 
  {
    if (!(*flags & HasZoneController)) return message("Need Zone Controller");
  }
  *flags |= HasBoxFull;

  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get scanner - SCAN (,OI |,ZI |,TOTE |,BOX |,LOT |,SERIAL |,nnn)?
 *-------------------------------------------------------------------------*/
get_scan(flags)
register unsigned long *flags;
{
  unsigned long ret;
  
#ifdef DEBUG
  fprintf(stderr, "get_scan() flags=%08x\n", *flags);
#endif
  
  if (!get_word("SCAN")) return 0;
  
  if (!(port_function & IsTotalFunction))
  {
    return message("Need Port Which Supports IO Module");
  }
  if (get_symbol(','))
  {
    if (get_word("OI"))				ret = SCAN_PL_INDUCT;
    else if (get_word("ZI"))		ret = SCAN_ZONE_INDUCT;
    else if (get_word("TOTE"))	ret = SCAN_BOX;
    else if (get_word("BOX"))		ret = SCAN_BOX;
    else if (get_word("LOT"))		ret = SCAN_LOT;
    else if (get_word("SERIAL"))	ret = SCAN_SERIAL;
    else 				ret = (get_number() & 0xff);
  }
  if (ret <= 0) return message("Define Scanner Use");
  
  *flags |= HasIOModule;
  
#ifdef DEBUG
  fprintf(stderr, "flags=%08x\n", *flags);
#endif
  
  return ret;
}
/*-------------------------------------------------------------------------*
 *  Get Bay Barrier  
 *-------------------------------------------------------------------------*/
get_x(flags)
register unsigned long *flags;
{
  if (!get_symbol('X')) return 0;

  *flags |= HasBayBarrier;
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Matrix, Carousel, or Put   [MCVHP]? X?
 *-------------------------------------------------------------------------*/
get_mc(flags)
register unsigned long *flags;
{
  unsigned char c;
  
  bypass_remarks();
  c = toupper(gnc());
  
  switch(c)
  {
    case 'X': *flags |= HasBayBarrier;
              return 1;
              
    case 'C': *flags |= IsCarousel;
    case 'H': *flags &= ~VertLights;
              *flags |=  HortLights;
              break;

    case 'V':    
    case 'M': *flags |= VertLights;
              break;
              
    case 'P': *flags |= IsPut;
              break;
  
    default:  *last++ = c;
              return 0;
  }
  if (!get_symbol('X')) return 1;

  *flags |= HasBayBarrier;
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Process Bay Items After All Are Collected
 *--------------------------------------------------------------------------*/
process_bay()
{
  register struct bay_item *b;
  register long k;
  long blow, bhigh, prod, mod;

#ifdef DEBUG
  fprintf(stderr, "process_bay()\n");
#endif

  for (k = 1, b = bay; k <= coh->co_bays; k++, b++)
  {
    if (!b->bay_number) continue;

    if (b->bay_flags & HasBayLamp)
    {
      if (!assign_bay_lamp(k)) return 0;
    }
    if (b->bay_flags & HasZoneController)
    {
      if (!assign_zone_controller(k)) return 0;
    }
    if (b->bay_flags & HasBoxFull)
    {
      if (!assign_box_full(k)) return 0;
    }
    if (b->bay_flags & HasModules)
    {
      if (!assign_modules(k)) return 0;
    }
    if (b->bay_flags & HasIOModule)			/* F052698 */
    {
      if (!assign_io(k)) return 0;
    }
    coh->co_bay_cnt = k;
  }  
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Zone Definition
 *-------------------------------------------------------------------------*
 *  Z[0-9]+ = B[0-9]+ (-B[0-9]+)? (, B[0-9]+ (-B[0-9]+)?)* $
 *-------------------------------------------------------------------------*/
get_zone()
{
  register struct zone_item *z;
  register struct bay_item *b;
  register long n, w, flag;
  long x, y;

#ifdef DEBUG
  fprintf(stderr, "get_zone()\n");
#endif

  n = get_number();
  if (n <= 0)            return message("Zone Number Expected");
  if (n > coh->co_zones) return message("Zone Number Too Large");

  z = &zone[n - 1];
  if (z->zt_zone) return message("Duplicate Zone Number");
  
  if (!get_symbol('=')) return message("Equals Expected");
   
  if (!get_bay_range(&x, &y)) return 0;

  if (n > coh->co_zone_cnt) coh->co_zone_cnt = n;

  z->zt_zone   = n;
  z->zt_status = 'W';
     
  w = 0;
  z->zt_first_bay = x;
  b = &bay[x - 1];
  flag         = b->bay_flags & Functions;
  z->zt_flags |= flag;
  
  while (1)
  {
    for (; x <= y; x++)
    {
      b = &bay[x - 1];

      if (!b->bay_number) return message("Bay Not Defined");
      if (b->bay_zone)    return message("Bay In Another Zone");
      b->bay_zone = n;

      z->zt_flags |= (b->bay_flags & HasBoxFull);
      
      if ((b->bay_flags & Functions) != flag) 
      {
        return message("Zone Not On Same Port Type");
      }
      if (w) bay[w - 1].bay_next = x;
      w = x;
    }
    if (get_symbol('\n')) break;
    if (!get_symbol(',')) return message("Comma Expected");
    if (!get_bay_range(&x, &y)) return 0;
  }
#ifdef DEBUG
  fprintf(stderr, "Zone %d (%d)\n", n, z - zone);
  Bdump(z, sizeof(struct zone_item));
#endif
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Master Bay Lamp Definition
 *-------------------------------------------------------------------------*
 *  MB [0-9]+ = B[0-9]+ (-B[0-9]+)? (, B[0-9]+ (-B[0-9]+)?)* $
 *-------------------------------------------------------------------------*/

get_mbl()
{
  register struct bay_item *b, *m;
  register long n;
  long x, y;
   
#ifdef DEBUG
  fprintf(stderr, "get_mbl()\n");
#endif

  if (sp->sp_master_bay_lamps != 'y')
  {
    message("Master Bay Lamp Feature Required");
  }
  n = get_number();
  if (n <= 0) return message("Bay Number Expected");
   
  if (n > coh->co_bay_cnt) return message("Bay Is Not Defined");

  m = &bay[n - 1];

  if (!(m->bay_flags & IsMasterBayLamp))
  {
    return message("Bay Has No Master Bay Lamp");
  }
  if (!(m->bay_flags & (IsFullFunction | IsTotalFunction)))
  {
    return message("Full or Total Function Bay Expected");
  }
  if (!get_symbol('=')) return message("Equals Expected");
   
  while (1)
  {
    if (!get_bay_range(&x, &y)) return 0;
   
    for (b = &bay[x - 1]; x <= y; x++, b++)
    {
/*-----------------------------------------------------------------*
      if (!(b->bay_flags & HasBayLamp) )    F100794 - bl is optional   
      {
        return message("Bay Must Have A Bay Lamp");
      }
 *-----------------------------------------------------------------*/
      if (b->bay_mbl) return message("Duplicate MBL Assignment");
      b->bay_mbl = n;
    }
    if (get_symbol(',')) continue;
    if (get_symbol('\n')) break;
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Late Entry List
 *-------------------------------------------------------------------------*
 *  LE = ALL | Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* $
 *-------------------------------------------------------------------------*/

get_le()
{
#ifdef DEBUG
  fprintf(stderr, "get_le()\n");
#endif

  if (!get_word("LE")) return 0;
   
  if (sp->sp_late_entry != 'y') message("Late Entry Feature Required");

  if (!get_symbol('=')) return message("Equals Expected");
   
  return process_list(LateEntry);
}

/*-------------------------------------------------------------------------*
 *  Get Early Exit List
 *-------------------------------------------------------------------------*
 *  EE = ALL | Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* $
 *-------------------------------------------------------------------------*/

get_ee()
{
#ifdef DEBUG
  fprintf(stderr, "get_ee()\n");
#endif

  if (!get_word("EE")) return 0;
   
  if (sp->sp_early_exit != 'y') message("Early Exit Feature Required");

  if (!get_symbol('=')) return message("Equals Expected");
   
  return process_list(EarlyExit);
}

/*-------------------------------------------------------------------------*
 *  Get All
 *-------------------------------------------------------------------------*
 *  EE | LE | ST | CZ = ALL
 *-------------------------------------------------------------------------*/

get_all(who, what)
unsigned char *who;
unsigned long what;
{
   register long k;

#ifdef DEBUG
  fprintf(stderr, "get_all(%s, %08x)\n", who, what);
#endif

  if (!get_word(who)) return 0;
   
  if (!get_symbol('=')) return message("Equals Expected");
   
  if (!get_word("ALL")) {flush(); return 1;}

  if (!pickline) return message("Pickline Required");
  if (!pl[pickline - 1].pl_first_zone) return message("Order Flow Required");

  pl[pickline - 1].pl_flags |= what;

  for (k = 0; k < coh->co_zones; k++)
  {
    if (zone[k].zt_pl == pickline) 
    {
    	zone[k].zt_pl    =  pickline;
      zone[k].zt_flags |= what;
    }
  }
  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get Steering List
 *-------------------------------------------------------------------------*
 *  ST = ALL | Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* $
 *-------------------------------------------------------------------------*/
get_st()
{
#ifdef DEBUG
  fprintf(stderr, "get_st()\n");
#endif

  if (!get_word("ST")) return 0;
   
  if (!get_symbol('=')) return message("Equals Expected");
   
  if (sp->sp_steering != 'y') message("Steering Feature Required");

  return process_list(Steering);
}

/*-------------------------------------------------------------------------*
 *  Get Jump Zone List
 *-------------------------------------------------------------------------*
 *  JZ = ALL | Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* $
 *-------------------------------------------------------------------------*/

get_jz()
{
#ifdef DEBUG
  fprintf(stderr, "get_jz()\n");
#endif

  if (!get_word("JZ")) return 0;
   
  if (!get_symbol('=')) return message("Equals Expected");
   
  if (sp->sp_jump_zone != 'y') message("Jump Zone Feature Required");

  return process_list(JumpZone);
}
/*-------------------------------------------------------------------------*
 *  Get Scanner (Demand) Induction List
 *-------------------------------------------------------------------------*
 *  SI = ALL | Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* $
 *-------------------------------------------------------------------------*/

get_si()
{
#ifdef DEBUG
  fprintf(stderr, "get_si()\n");
#endif

  if (!get_word("SI")) return 0;
   
  if (!get_symbol('=')) return message("Equals Expected");
   
  if (port_function & IsBasicFunction) return message("Not with BF");
  
  return process_list(DemandFeed);
}
/*-------------------------------------------------------------------------*
 *  Get Switch Action Modes
 *-------------------------------------------------------------------------*/
get_sam()
{
  register long k;
  
#ifdef DEBUG
  fprintf(stderr, "get_sam()\n");
#endif

  if (!pickline) return message("Pickline Not Specified");

  for (k = 0; k < SAMS; k++)
  {
    if (get_word(sam[k])) break;
  }
  if (k >= SAMS) return 0;
  
  strncpy(pl[pickline - 1].pl_sam, sam[k], 3);
  
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Early Exit Pickline Mode
 *-------------------------------------------------------------------------*
 *  EEMODE = LAST | NEXT $
 *-------------------------------------------------------------------------*/

get_eemode()
{
  register struct pl_item *p;

#ifdef DEBUG
  fprintf(stderr, "get_eemode()\n");
#endif

  if (!get_word("EEMODE")) return 0;
  
  if (!pickline) return message("Pickline Not Specified");

  p = &pl[pickline - 1];

  if (sp->sp_early_exit != 'y')
  {
    message("Early Exit Feature Required");
  }
  if (!get_symbol('=')) return message("Equals Expected");
    
  if (get_word("NEXT"))      p->pl_flags |= EarlyExitModeNext;
  else if (get_word("LAST")) p->pl_flags |= EarlyExitModeLast;
  else message("Unrecognized");

  if (!get_symbol('\n')) {message("End Of Line Expected"); flush();}

  p->pl_flags |= EarlyExit;

  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get Order Flow
 *-------------------------------------------------------------------------*
 *  N[0-9]+ , Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)* , N[0-9]+ $
 *-------------------------------------------------------------------------*/
get_flow()
{
  register struct pl_item *p;
  register struct zone_item *z;
  register struct seg_item *s;
  long n, zlow, zhigh;
  register long k, prior, found;
  char text[64];
  unsigned char c;
  
  if (!get_symbol('N')) return 0;

  if (!pickline)                          /* make a default pickline         */
  {
    pl[0].pl_pl = 1;
    strcpy(pl[0].pl_name, "Pickline");
    strcpy(pl[0].pl_sam, "DSM");
  }
  p = &pl[pickline - 1];

  coh->co_seg_cnt += 1;                   /* increment segment count         */
  segment = coh->co_seg_cnt;

  if (segment > coh->co_segments) return message("Too Many Segments");

  n = get_number();
  if (n < 0 || n > 99) return message("Node Number 0-99 Expected");
  
  s = &sg[segment - 1];                   /* point to new segment            */
  s->sg_segment = segment;                /* store segment number            */
  s->sg_pl      = pickline;               /* store pickline number           */
  s->sg_snode   = n;                      /* starting node number            */

  if (!get_symbol(',')) return message("Comma Expected");
  
  if (!get_zone_range(&zlow, &zhigh)) return message("Zone(s) Expected");

  prior = 0;
  
  while (1)
  {
    if (prior > zlow) 
    {
      sprintf(text, "Zone %d Must Be Greater Than Prior Zone", k);
      return message(text);
    }
    for (k = zlow, z = &zone[zlow - 1]; k <= zhigh; k++, z++)
    {
      if (!z->zt_zone) 
      {
        sprintf(text, "Zone %d Not Defined", k);
        return message(text);
      }
      if (z->zt_pl) 
      {
        sprintf(text, "Zone %d Already In Order Flow", k);
        return message(text);
      }
      z->zt_pl = pickline;               /* assign zone to pickline         */
      z->zt_segment = segment;           /* assign zone to segment          */
      p->pl_flags |= z->zt_flags;        /* promote flags to pickline       */
      
      if (z->zt_zone > p->pl_last_zone)  p->pl_last_zone  = z->zt_zone;
      if (z->zt_zone < p->pl_first_zone) p->pl_first_zone = z->zt_zone;
      else if (!p->pl_first_zone)        p->pl_first_zone = z->zt_zone;
      
      if (z->zt_zone > s->sg_last_zone)  s->sg_last_zone  = z->zt_zone;
      if (z->zt_zone < s->sg_first_zone) s->sg_first_zone = z->zt_zone;
      else if (!s->sg_first_zone)        s->sg_first_zone = z->zt_zone;
      
      z->zt_source = prior;
      if (prior) zone[prior - 1].zt_feeding = k;
      prior = k;
    }
    if (get_symbol('(') || get_symbol('/'))
    {
      while (1)                           /* bypass old disq and prereqs     */
      {
        c = gnc();
        if (c == ')' || c == '/') break;
        if (c == '\n') {*last++ = c; break;}
      }
    }
    if (!get_symbol(',')) return message("Comma Expected");
    if (get_symbol('N')) break;

    if (!get_zone_range(&zlow, &zhigh)) return message("Zone(s) Expected");
  }
  n = get_number();
  if (n < 1 || n > 99) return message("Node Number 1-99 Expected");

  s->sg_enode = n;

  if (s->sg_enode <= s->sg_snode) 
    return message("End Node Not Greater Than Start");

  if (!get_symbol('\n')) {message("End of Line Expected"); flush();}

  if (!p->pl_first_segment) p->pl_first_segment = segment;
  if (segment > p->pl_last_segment) 
  {
    p->pl_last_segment = segment;
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Assign IO Module To Bay - F052698 
 *-------------------------------------------------------------------------*/
assign_io(x)
register long x;
{
  register struct bay_item *b;
  static long io_mod   = 0;
  static long next_io  = 0;
  char text[80];
  
  b = &bay[x - 1];

  for (; next_io < coh->co_light_cnt; next_io++)
  {
    if (hw[next_io].hw_type == IO)
    {
      io_mod++;
      b->bay_port = assign_port(IO, io_mod);
      hw[next_io].hw_bay = x;
      hw[next_io].hw_state = b->bay_state;
      if (next_io > coh->co_hw_cnt) coh->co_hw_cnt = next_io;
      b->bay_state = 0;
	next_io++;
      return 1;
    }
  }
  sprintf(text, "Not Enougn Bay Lamps At Bay %d", x);
  return message(text);
}
/*-------------------------------------------------------------------------*
 *  Assign Bay Lamp To Bay
 *-------------------------------------------------------------------------*/
assign_bay_lamp(x)
register long x;
{
  register struct bay_item *b;
  static long bay_lamp = 0;
  static long next_bl  = 0;
  char text[80];
  
  b = &bay[x - 1];

  for (; next_bl < coh->co_light_cnt; next_bl++)
  {
    if (hw[next_bl].hw_type == BL)
    {
      bay_lamp++;
      b->bay_port = assign_port(BL, bay_lamp);
      hw[next_bl].hw_bay = x;
#ifdef DEBUG
      fprintf(stderr, "assign_bay hwix = %d, bay =%d bl =%d\n",
                next_bl, x, bay_lamp);
      fflush(stderr);
#endif
      b->bay_bl = ++next_bl;
      if (next_bl > coh->co_hw_cnt) coh->co_hw_cnt = next_bl;
      return 1;
    }
  }
  sprintf(text, "Not Enougn Bay Lamps At Bay %d", x);
  return message(text);
}
/*-------------------------------------------------------------------------*
 *  Assign Zone Controllers To Bays
 *-------------------------------------------------------------------------*/
assign_zone_controller(x)
register long x;
{
  register struct bay_item *b;
  register struct hw_item *h;
  static long zone_controller = 0;
  static long next_zc = 0;
  register long k, m;
  char text[80];
  
#ifdef DEBUG
  fprintf(stderr, "assign_zone_controller(%d)\n", x);
#endif

  b = &bay[x - 1];

  for (; next_zc < coh->co_light_cnt; next_zc++)
  {
    if (hw[next_zc].hw_type == ZC || hw[next_zc].hw_type == ZC2)
    {
      zone_controller++;
      m = assign_port(hw[next_zc].hw_type, zone_controller);
      if (b->bay_port)
      {
        if (b->bay_port != m)
        {
           sprintf(text, "Bay %d Modules Not On Same Port", x);
           message(text);
        }
      }  
      b->bay_port = m;
      hw[next_zc].hw_bay = x;
      b->bay_zc = ++next_zc;
      if (next_zc > coh->co_hw_cnt) coh->co_hw_cnt = next_zc;

      return 1;
    }
  }
  sprintf(text, "Not Enough Zone Controllers At Bay %d", x);
  return message(text);
}
/*-------------------------------------------------------------------------*
 *  Assign Box Full To Bay 
 *-------------------------------------------------------------------------*/
assign_box_full(x)
register long x;
{
  register struct bay_item *b;
  register struct hw_item *h;
  static long box_full_module = 0;
  static long next_bf = 0;
  register long k, m;
  char text[80];
  
#ifdef DEBUG
  fprintf(stderr, "assign_box_full(%d)\n", x);
#endif

  b = &bay[x - 1];
  
  if (b->bay_flags & IsTotalFunction)
  {
    for (; next_bf < coh->co_light_cnt; next_bf++)
    {
      if (hw[next_bf].hw_type != BF) continue;
      box_full_module++;
      
      m = assign_port(hw[next_bf].hw_type, box_full_module);
      if (b->bay_port)
      {
        if (b->bay_port != m)
        {
           sprintf(text, "Bay %d Modules Not On Same Port", x);
           message(text);
        }
      }  
      b->bay_port = m;
      hw[next_bf].hw_bay = x;
      b->bay_bf = ++next_bf;
      if (next_bf > coh->co_hw_cnt) coh->co_hw_cnt = next_bf;

      return 1;
    }
    sprintf(text, "Not Enough Box Full Modules At Bay %d", x);
    return message(text);
  }
  else if (b->bay_flags & IsFullFunction)
  {
    box_full_module++;
      
    h = &hw[b->bay_zc];                    /* first module after zc          */

    if (h->hw_type != PM)
    {
      sprintf(text, "Bay %d - PM Needed For Box Full", x);
      message(text);
      return 0;
    }
#ifdef DEBUG
  fprintf(stderr, "BoxFull hwix=%d\n", b->bay_zc);
#endif
    po[b->bay_port - 1].po_count[PM - 1] -= 1;
    po[b->bay_port - 1].po_count[BF - 1] += 1;
        
    b->bay_bf = b->bay_zc + 1;            /* hwix of box full module         */
        
    h->hw_bay   = x;
    h->hw_type  = BF;
    h->hw_mod   = box_full_module;  
    h->hw_state = PMBOX1;
        
    h++;                                  /* renumber all modules            */
        
    for (k = b->bay_zc + 2; k < coh->co_light_cnt; k++, h++)
    {
      if (h->hw_type == PM) h->hw_mod -= 1;
    }
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Assign Modules To A Bay
 *-------------------------------------------------------------------------*/
assign_modules(x)
register long x;                          /* bay number                      */
{
  static long next_pm = 0;
  static long pm = 0;
  register struct bay_item *b;
  register long k, m, prods, mods, tc, pi_count;
  char string[4], text[80];
  
#ifdef DEBUG
  fprintf(stderr, "assign_modules(%x)\n", x);
#endif

  b = &bay[x - 1];
  
  b->bay_prod_first = coh->co_prod_cnt + 1;
  b->bay_mod_first  = coh->co_mod_cnt + 1;

  prods = b->bay_prod_last;
  mods  = b->bay_mod_last;
  
  m = prods / mods;                        /* product ratio                  */
  
  if (mods == 1)                           /* F101096 - force to 1 shelf     */
  {
    b->bay_flags &= ~HortLights;
    b->bay_flags |=  VertLights;
  }
  if (b->bay_flags & VertLights)
  {
    b->bay_shelves = mods;
    b->bay_width   = m;
  }
  else if (b->bay_flags & HortLights)
  {
    b->bay_shelves = m;
    b->bay_width   = mods;
  }
  if (coh->co_prod_cnt + prods > coh->co_products) 
  {
    return message("Too Many Products Needed");
  }
  if (coh->co_mod_cnt + mods > coh->co_modules) 
  {
    return message("Too Many Modules Needed");
  }
  if (b->bay_flags & IsBasicFunction) 
  {
    b->bay_controller = hw[next_pm].hw_controller;

    if (sp->sp_check_controllers == 'y')
    {
      for (k = next_pm, pi_count = 0; k < coh->co_light_cnt; k++)
      {
        if (hw[k].hw_controller != b->bay_controller) break;
        if (hw[k].hw_type != PI) continue;
        pi_count++;
      }
      if (mods != pi_count)
      {
        sprintf(text, "Bay %d Has %d Modules - Config Has %d", 
                       x, pi_count, mods);
        message(text);
        mods = prods = pi_count;
      }
    }
  }
  for (k = 0; k < mods; k++)
  {
    for (; next_pm < coh->co_light_cnt; next_pm++)
    {
      if (hw[next_pm].hw_type == PM)  break;
      if (hw[next_pm].hw_type == PI)  break;
      if (hw[next_pm].hw_type == PM2) break;
      if (hw[next_pm].hw_type == PM4) break;
      if (hw[next_pm].hw_type == PM6) break;
    }
    if (next_pm >= coh->co_light_cnt) 
    {
      sprintf(text, "Not Enough Pick Lights At Bay %d", x);
      return message(text); 
    }
    hw[next_pm].hw_bay   = x;
    hw[next_pm].hw_first = coh->co_prod_cnt + 1;
    
    pw[coh->co_prod_cnt].pw_mod = hw[next_pm].hw_mod;
    pw[coh->co_prod_cnt].pw_ptr = ++next_pm;
    
    mh[coh->co_mod_cnt].mh_ptr = next_pm;
    
    if (next_pm > coh->co_hw_cnt) coh->co_hw_cnt = next_pm;

    coh->co_prod_cnt++;
    coh->co_mod_cnt++;
    
    m = assign_port(hw[next_pm - 1].hw_type, coh->co_mod_cnt);
    if (b->bay_port)
    {
      if (b->bay_port != m)
      {
        if (!k) sprintf(text, "Bay %d Port Conflict", x);
        else sprintf(text, "Bay %d Modules Not On Same Port", x);
        message(text);
      }
    }  
    b->bay_port = m;
    
    if (b->bay_flags & VertLights)         /* need bay_width products        */
    {
      for (m = 1; m < b->bay_width; m++)   /* assign bay_width -1 more mods  */
      {
        pw[coh->co_prod_cnt].pw_mod = hw[next_pm - 1].hw_mod;
        pw[coh->co_prod_cnt].pw_ptr = next_pm;
    
        coh->co_prod_cnt++;
      }
    }
  }
  b->bay_prod_last = coh->co_prod_cnt;
  b->bay_mod_last  = coh->co_mod_cnt;

  po[b->bay_port - 1].po_products += prods;  /* products on port             */
  
  if (!(b->bay_flags & Multibin)) return 1;
  
  if (b->bay_flags & HortLights)
  {
    for (k = 1; k < b->bay_shelves; k++)     /* multiple shelf copies of pw  */
    {
      memcpy(&pw[coh->co_prod_cnt],
        &pw[coh->co_prod_cnt - b->bay_width], 
        sizeof(struct pw_item) * b->bay_width);
        
      coh->co_prod_cnt += b->bay_width;
      b->bay_prod_last += b->bay_width;
    }
  }
  if (sp->sp_sku_support == 'n')             /* F101296                      */
  {
    for (k = 0; k < prods; k++)              /* generate matrix display      */
    {
       if (mods == 1) sprintf(string, "%d%c",/* 1A .. 1J to 9A .. 9J         */
         ((k / 10) + 1) % 10, (k % 10) + 'A');
       else sprintf(string, "%d%c",
         ((k / b->bay_width) + 1) % 10, (k % b->bay_width) + 'A');
       
       memcpy(pw[b->bay_prod_first + k - 1].pw_display, string, 2);
    }
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Assign Port
 *-------------------------------------------------------------------------*/
assign_port(type, n)
register long type, n;
{
  register k, total;
  
  total = 0;

  for (k = 0; k < PortMax; k++)
  {
     if (type == BL) 		total += po[k].po_count[BL - 1];
     else if (type == BF)	total += po[k].po_count[BF - 1];
     else if (type == IO)	total += po[k].po_count[IO - 1];
     else if (type == ZC || type == ZC2)  
     {
       total += po[k].po_count[ZC  - 1];
       total += po[k].po_count[ZC2 - 1];
     }
     else
     {
       total += po[k].po_count[PM  - 1];
       total += po[k].po_count[PM2 - 1];
       total += po[k].po_count[PM4 - 1];
       total += po[k].po_count[PM6 - 1];
       total += po[k].po_count[PI  - 1];
     }
     if (n <= total) return (k + 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Add A Simulated Module
 *-------------------------------------------------------------------------*/
add_module(n, x, z, j, state)
register long n, x, z, j, state;          /* port + type + bay + pi			  */
{
  static long hw_cnt = 0;
  static long bl_cnt = 0;
  static long zc_cnt = 0;
  static long pm_cnt = 0;
  static long bf_cnt = 0;
  static long io_cnt = 0;						/* F052698 */
  char text[80];
  
#ifdef DEBUG
 fprintf(stderr, "add_module(port=%d, type=%d, bay=%d)\n", n, x, z);
#endif

  po[n - 1].po_lights += 1;
  coh->co_light_cnt   += 1;
  
  if (coh->co_light_cnt > sp->sp_lights)    /* F070996 - check space         */
  {
    sprintf(text, "Not Enough Pick Lights At Bay %d", z);
    message(text);
    leave(2);
  }
  if (x == BL)      
  {
    po[n - 1].po_count[BL - 1] += 1;

    hw[hw_cnt].hw_type  = BL;
    hw[hw_cnt].hw_mod   = ++bl_cnt;
    if (j > 0)
    {
      hw[hw_cnt].hw_controller = z;
      hw[hw_cnt].hw_mod_address = j - 1;
    }
    else hw[hw_cnt].hw_state = BLOFF1;
  }
  else if (x == ZC || x == ZC2) 
  {
    po[n - 1].po_count[ZC - 1] += 1;

    hw[hw_cnt].hw_type  = x;
    hw[hw_cnt].hw_mod   = ++zc_cnt;
    if (j > 0)
    {
      hw[hw_cnt].hw_controller = z;
      hw[hw_cnt].hw_mod_address = j - 1;
    }
    else hw[hw_cnt].hw_state = ZCIDLE1;
  }
  else if (x == BF)
  {
    po[n - 1].po_count[BF - 1] += 1;

    hw[hw_cnt].hw_type = BF;
    hw[hw_cnt].hw_mod  = ++bf_cnt;
    if (j > 0)
    {
      hw[hw_cnt].hw_controller = z;
      hw[hw_cnt].hw_mod_address = j - 1;
    }
    else hw[hw_cnt].hw_state = PMBOX1;
  }
  else if (x == PM || x == PM2 || x == PM4 || x == PM6) 
  {
    po[n - 1].po_count[x - 1] += 1;

    hw[hw_cnt].hw_type  = x;
    hw[hw_cnt].hw_mod   = ++pm_cnt;
    if (j > 0)
    {
      hw[hw_cnt].hw_controller = z;
      hw[hw_cnt].hw_mod_address = j - 1;
    }
    else hw[hw_cnt].hw_state = PMIDLE;
  }
  else if (x == PI) 
  {
    po[n - 1].po_count[PI - 1] += 1;

    hw[hw_cnt].hw_type  = PI;
    hw[hw_cnt].hw_mod   = ++pm_cnt;
    hw[hw_cnt].hw_controller = z;
    hw[hw_cnt].hw_mod_address = j - 1;
  }
  else if (x == IO)								/* F052698 - scanner module		  */
  {
    po[n - 1].po_count[IO - 1] += 1;

    hw[hw_cnt].hw_type = IO;
    hw[hw_cnt].hw_mod  = ++io_cnt;
    hw[hw_cnt].hw_controller = z;
    hw[hw_cnt].hw_mod_address = j - 1;
  
    hw[hw_cnt].hw_state = state;
  }
  hw_cnt++;
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Process List Of Zones
 *-------------------------------------------------------------------------*
 *   ALL | 
 *   Z[0-9]+ (-Z[0-9]+)? (, Z[0-9]+ (-Z[0-9]+)?)*
 *-------------------------------------------------------------------------*/
process_list(flag)
register long flag;
{
  register struct zone_item *z;
  register long k;
  long x, y;

#ifdef DEBUG
  fprintf(stderr, "process_list(%x)\n", flag);
#endif

  if (get_word("ALL")) {flush(); return 1;}

  while (1)
  {
    if (!get_zone_range(&x, &y)) return message("Zone(s) Expected");
      
    for (z = &zone[x - 1]; x <= y; x++, z++)
    {
      z->zt_flags |= flag;
    }
    if (get_symbol(',')) continue;
    break;
  }
  if (!get_symbol('\n')) {message("End Of Line Expected"); flush();}
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get A Bay Range  -  B[0-9]+ (- B[0-9]+)?
 *-------------------------------------------------------------------------*/
get_bay_range(x, y)
register long *x, *y;
{
  register struct bay_item *b;
  register long k;
  
#ifdef DEBUG
  fprintf(stderr, "get_bay_range()\n");
#endif

  if (!get_symbol('B')) return message("Bays Expected");

  *x = *y = get_number();
   
  if (*x <= 0)           return message("Bay Number Number Expected");
  if (*x > coh->co_bays) return message("Bay Number Too Large");
  
  if (get_symbol('-'))
  {
    if (!get_symbol('B'))  return message("Bay Number Expected");
   
    *y = get_number();
      
    if (*y <= 0)           return message("Bay Number Expected");
    if (*y > coh->co_bays) return message("Bay Number Too Large");
    if (*x > *y)           return message("Invalid Bay Range");
  }
  else *y = *x;

  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Zone Range  -  Z[0-9]+ (- Z[0-9]+ )?
 *-------------------------------------------------------------------------*/

get_zone_range(x, y)
register long *x, *y;
{
#ifdef DEBUG
  fprintf(stderr, "get_zone_range()\n");
#endif

  if (!get_symbol('Z')) return 0;

  *x = *y = get_number();
   
  if (*x <= 0)            return message("Zone Number Expected");
  if (*x > coh->co_zones) return message("Zone Number Too Large");
   
  if (get_symbol('-'))
  {
    if (!get_symbol('Z'))  return message("Zone Expected");
   
    *y = get_number();
      
    if (*y <= 0)            return message("Zone Number Expected");
    if (*y > coh->co_zones) return message("Zone Number Too Large");
  }
  else *y = *x;

  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A TC Range  -  T[0-9]+ (- T[0-9]+)?
 *-------------------------------------------------------------------------*/

get_tc_range(x, y)
register long *x, *y;
{
#ifdef DEBUG
  fprintf(stderr, "get_tc_range()\n");
#endif

  if (!get_symbol('T')) return 0;

  if (sp->sp_basic_function != 'y' && sp->sp_basic_function != 's')
  {
    message("Basic Function Feature Required");
  }
  *x = *y = 0;

  if (get_symbol('C')) return 1;          /* TC only is allowed F042595      */

  *x = get_number();
   
  if (*x <= 0)           return message("TC Number Expected");
  if (*x > coh->co_bays) return message("TC Number Too Large");
  
  if (get_symbol('-'))
  {
    if (!get_symbol('T')) return message("TC Number Expected");
   
    *y = get_number();
      
    if (*y <= 0)            return message("TC Number Expected");
    if (*y > coh->co_bays)  return message("TC Number Too Large");
    if (*x > *y)            return message("Invalid TC Range");
  }
  else *y = *x;

  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Module Range  -  [0-9]? (,[0-9]+)?
 *-------------------------------------------------------------------------*/

get_module_range(x, y)
register long *x, *y;
{
  long n;

#ifdef DEBUG
  fprintf(stderr, "get_module_range()\n");
#endif

  *x = *y = get_number();                 /* get number of products          */
   
  if (*x <= 0) return 0;                  /* no number found                 */
   
  if (sp->sp_multibin_lights)             /* product count is allowd         */
  {
    if (get_symbol(','))                  /* check comma follows             */
    {
      n = get_number();                   /* get possible modules            */
      
      if (n) *y = n;                      /* have products + modules         */
      else *last++ = ',';                 /* replace comma                   */
    }
  }
  return 1;                               /* return found                    */
}
/*-------------------------------------------------------------------------*
 *  Get A Word
 *-------------------------------------------------------------------------*/
get_word(p)
register unsigned char *p;
{
  register unsigned char *q, c;
  unsigned char work[16];

  q = work;
  bypass_remarks();                       /* skip over any remarks           */

#ifdef DEBUG
  fprintf(stderr, "  get_word(%s)\n", p);
#endif

  while (*p)
  {
    *q++ = c = toupper(gnc());            /* get next byte                   */

    if (*p != c)
    {
      while (q > work) *last++ = *(--q);
      return 0;
    }
    p++;
  }
#ifdef DEBUG
  *q = 0;
  fprintf(stderr, "  found word = [%s]\n", work);
#endif

  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get A Symbol
 *-------------------------------------------------------------------------*/

get_symbol(x)
register unsigned char x;
{
  register unsigned char c;
   
  bypass_remarks();                       /* skip over any remarks           */

  c = toupper(gnc());                     /* get next byte                   */

#ifdef DEBUG
  if (x > 0x20 && c > 0x20 && x < 0x7f && c < 0x7f)
  {
    fprintf(stderr, "  get_symbol(%c) = %c\n", x, c);
  }
  else fprintf(stderr, "  get_symbol(0x%02x) = 0x%02x\n", x, c);
#endif

  if (c == x) return 1;

  *last++ = c;                            /* unget current byte              */
  return 0;
}

/*-------------------------------------------------------------------------*
 *   Get a number - Return of zero is nothing found.
 *-------------------------------------------------------------------------*/

get_number()
{
  register unsigned char c;
  register long x;
   
  bypass_remarks();                       /* skip over any remarks           */

  x = 0;                                  /* clear number                    */

  while (1)                               /* convert number                  */
  {
    c = gnc();
    if (c < '0' || c > '9') break;
      
    x = 10 * x + (c - '0');
  }
  *last++ = c;                            /* unget last byte                 */

#ifdef DEBUG
  fprintf(stderr, "get_number() = %d\n", x);
#endif

  return x;
}

/*-------------------------------------------------------------------------*
 *  Bypass whitespace and remarks
 *-------------------------------------------------------------------------*/
bypass_remarks()
{
  register unsigned char c;

  while (1)
  {
    c = gnc();
    if (c == 0x20 || c == 0x09) continue;
    break;
  }
  if (c == '*')
  {
    while (1)
    {
      c = gnc();
      if (c == END || c == '\n') break;
    }
  }
  *last++ = c;
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Get next character
 *-------------------------------------------------------------------------*/
unsigned char gnc()
{
  register long ret;
  register unsigned char c;
  
  if (last > backup) return *(--last);    /* pop a backup character          */

  if (!*next) gnl();                      /* get another line                */

  c = *next++;                            /* get the byte                    */

  return c;                               /* return a byte                   */
}
/*-------------------------------------------------------------------------*
 *  Flush Input Buffer
 *-------------------------------------------------------------------------*/
flush()
{
  next = buffer;
  last = backup;
  *next = 0;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get a line of input
 *-------------------------------------------------------------------------*/
gnl()
{
  next = buffer;                          /* reset pointer                   */

  memset(buffer, 0, LINE + 1);            /* clear the buffer                */

  if (!fgets(buffer, LINE, fd))           /* get next line                   */
  {
    memset(buffer, END, LINE);            /* fill with end of file           */
  }
  line++;                                 /* count number of lines           */
#ifdef DEBUG
  fprintf(stderr, "*** Line %3d: %s", line, buffer);
#endif
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Close files and exit.
 *-------------------------------------------------------------------------*/
leave(n)
register long n;
{
  long pid, stat;
  char text[80], prt_name[16];

  if (live) co_close_save();              /* save real shared segment        */
  else
  {
    if (select0 && co_size > 0) fwrite(co, co_size, 1, cd);
    fclose(cd);
    if (n > 0 || errors > 0) unlink(cd_name);
  }
  if (fd)   fclose(fd);                   /* close input file                */
  if (ed)   fclose(ed);                   /* close error file                */
  ss_close();                             /* close system parms              */

  if (send_errors) message_close();

  if (errors)
  {
    n = 1;
    
    if (fork() == 0)
    {
      execlp("prft", "prft", ed_name, tmp_name(prt_name),
        "sys/report/report_report.h", "Configuration Errors", 0);
      krash("leave", "load prft", 0);
      exit(1);
    }
    pid = wait(&stat);
  }
  else unlink(ed_name);

  exit(n);                                /* exit with code                  */
}

/*-------------------------------------------------------------------------*
 *  Open files and setup needed pointers
 *-------------------------------------------------------------------------*/
open_all_files()
{
  register struct hw_item *h;
  register struct pw_item *i;
  register long k;
  char command[80];
  long status;
  
  ss_open();                              /* open system parms               */
  if (send_errors) message_open();
  
  tmp_name(ed_name);                      /* error file name                 */
  ed = fopen(ed_name, "w");               /* open error file                 */
  if (ed == 0)                            /* open errors failed              */
  {  
    krash("open_all_files", "tmp file", 1);
  }
  fd = fopen(fd_name, "r");               /* open config input file          */
  if (fd == 0)                            /* open input failed               */
  {
    message("Invalid Configuration Name");
    leave(1);
  }
  if (live) co_open();                    /* using real co segment           */
  else
  {
    sprintf(command, "co_init %s 1>/dev/null 2>&1", cd_name);
    system(command);

    cd = fopen(cd_name, "r+");            /* open co segment                 */
    if (cd == 0)                          /* open segment failed             */
    {
      message("Failed To Build Dummy co_init");
      leave(1);
    }
    fseek(cd, 0, 2);
    co_size = ftell(cd);                  /* get configuration size          */
    fseek(cd, 0, 0);
  
    co = (unsigned char *)malloc(co_size);/* allocate space                  */
    if (co == 0) krash("open_all_files", "alloc co", 1);
  
    if (fread(co, co_size, 1, cd) != 1) krash("open_all_files", "read co", 1);
    
    fseek(cd, 0, 0);                     /* rewind file position             */
    
    coh   = (struct co_header *)co;
    coh->co_port_cnt = 2;                /* full and/or basic                */

    po    = (struct port_item *) ((char *)co + coh->co_po_offset);
    pl    = (struct pl_item *)   ((char *)co + coh->co_pl_offset);
    sg    = (struct seg_item *)  ((char *)co + coh->co_seg_offset);
    zone  = (struct zone_item *) ((char *)co + coh->co_zone_offset);
    bay   = (struct bay_item *)  ((char *)co + coh->co_bay_offset);
    hw    = (struct hw_item *)   ((char *)co + coh->co_hw_offset);
    pw    = (struct pw_item *)   ((char *)co + coh->co_pw_offset);
    mh    = (struct mh_item *)   ((char *)co + coh->co_mh_offset);
  }
  memset(pl,   0, coh->co_picklines * sizeof(struct pl_item));
  memset(sg,   0, coh->co_segments  * sizeof(struct seg_item));
  memset(bay,  0, coh->co_bays      * sizeof(struct bay_item));
  memset(zone, 0, coh->co_zones     * sizeof(struct zone_item));
  memset(pw,   0, coh->co_products  * sizeof(struct pw_item));
  memset(mh,   0, coh->co_modules   * sizeof(struct mh_item));
  
  for (k = 0, h = hw; k < coh->co_lights; k++, h++) 
  {
    h->hw_bay     = 0;
    h->hw_first   = 0;
    h->hw_current = 0;
    h->hw_flags   = SwitchesDisabled;
  }
  for (k = 0, i = pw; k < coh->co_products; k++, i++) 
  {
    i->pw_case  = 1000;
    i->pw_pack  = 100;
    
    i->pw_display[0] = i->pw_display[1] = 0x20;    /* clear display values */
    
    if (rf->rf_sku > 0) i->pw_flags |= PicksInhibited;
  }
  for (k = 0; k < coh->co_bays; k++) bay[k].bay_flags = SwitchesDisabled;
  
  for (k = 0; k < coh->co_picklines; k++) 
  {
    pl[k].pl_flags = (OrdersLocked | SwitchesDisabled); 
  }
  for (k = 0; k < coh->co_ports; k++) po[k].po_products = 0;

  coh->co_datetime = time(0);
  strncpy(coh->co_config_name, config_name, 31);
  
  coh->co_pl_config = 0;
  coh->co_pl_cnt    = 0;
  coh->co_seg_cnt   = 0;
  coh->co_zone_cnt  = 0;
  coh->co_bay_cnt   = 0;
  coh->co_prod_cnt  = 0;
  coh->co_mod_cnt   = 0;
  coh->co_hw_cnt    = 0;
  coh->co_st_cnt    = 0;

  if (!live || sp->sp_full_function  == 's'  || 
               sp->sp_basic_function == 's' || 
               sp->sp_total_function == 's')
  {
    coh->co_light_cnt = 0;
    coh->co_port_cnt  = coh->co_ports;
  }
  return 0;
}

/* end of configure.c */
