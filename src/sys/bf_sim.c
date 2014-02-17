/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Basic Function Simulator.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/22/94   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char bf_sim_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "co.h"
#include "message_types.h"
#include "caps_messages.h"

long who;                                 /* message sender                  */
long type;                                /* message type                    */
TCapsMessageItem x;                       /* message text                    */
long len;                                 /* message length                  */

unsigned char list[] = {ShutdownRequest, TCInputPacket, ConfigureEvent,
  RestoreplaceEvent};

#define ENABLED      0x0001
#define TWOBUTTON    0x0002
#define ONEBUTTON    0x0004
#ifdef BOXFULL
#   undef BOXFULL
#endif
#define BOXFULL      0x0008
#define IMMED        0x0010
#define PICKS        0x0020
#define RECALL       0x0040
#define SHORT1       0x0100
#define SHORT2       0x0200
#define SHORT3       0x0400
#define SHORT4       0x0800
#define SHORTS       0x0f00

typedef struct
{
  unsigned short bf_status;
  unsigned char  bf_max;
  unsigned char  bf_pick;
  unsigned char  bf_pi[50];
  unsigned short bf_quan[50];
  unsigned short bf_picked[50];
  char           bf_order[6];
  char           bf_save[10];

} bf_status_item;

long port[PortMax + 1];
bf_status_item *bf = 0;

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=bf_sim");
  chdir(getenv("HOME"));
  
  ss_open();
  co_open();
  
  message_open();
  message_select(list, sizeof(list));
  
  while (1)
  {
    message_get(&who, &type, &x, &len);

#ifdef DEBUG
  fprintf(stderr, "message_get(): who:%d type:%d len:5d\n", who, type, len);
  if (len > 0) Bdump(&x, len);
#endif

    switch (type)
    {
      case ShutdownRequest:
    
        ss_close();
        co_close();
        message_close();
        exit(0);

      case TCInputPacket:  process_packet(&x); break;
    
      case ConfigureEvent:
      case RestoreplaceEvent:  setup(); break;
      default: break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Setup Tables
 *-------------------------------------------------------------------------*/
setup()
{
  register struct bay_item *b;
  register long j, k;
  
  for (k = 0; k <= PortMax; k++) port[k] = coh->co_bay_cnt;
  
  for (k = 0, b = bay; k < coh->co_bay_cnt; k++, b++)
  {
    if (b->bay_port <= 0) continue;
    j = b->bay_port - 1;
    if (k < port[j]) port[j] = k;
  }
  if (bf) free(bf);
  
  bf = (bf_status_item *)malloc(coh->co_bay_cnt * sizeof(bf_status_item));
  memset(bf, 0, coh->co_bay_cnt * sizeof(bf_status_item));

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Packet
 *-------------------------------------------------------------------------*/
process_packet(x)
register TPacketMessage *x;
{
  register struct bay_item *b;
  register bf_status_item *t;
  register long i, j, k, first_bay, last_bay;
  long tc, pi, cmd, quan, swt;
  TPacketMessage y;
  char text[16];
  
  sscanf(x->m_tc,  "%04d", &tc);
  sscanf(x->m_pi,  "%02d", &pi);
  sscanf(x->m_cmd, "%02d", &cmd);
  
#ifdef DEBUG
  fprintf(stderr, "process_packet(): port:%d tc:%d pi:%d cmd:%d\n",
    x->m_port, tc, pi, cmd);
#endif

  if (!bf) return 0;
  
  if (tc == 9999) 
  {
    first_bay = port[x->m_port];
    last_bay  = port[x->m_port + 1];
  }
  else first_bay = last_bay = port[x->m_port] + tc - 1;
  
  for (k = first_bay; k <= last_bay; k++)
  {
    b = &bay[k];
    t = &bf[k];
  
    switch (cmd)
    {
      case 0:                             /* switch action                  */
    
        if (!(t->bf_status & ENABLED)) return 0;
        sscanf(x->m_quan, "%04d", &swt);
        process_switch(k, tc, pi, swt);
        break;
      
      case 1:                             /* soft reset                     */
    
        t->bf_status = ENABLED;
        t->bf_max    = 0;
        t->bf_pick   = 0;
        sprintf(text, "CAPS PI=%02d", b->bay_mod_last - b->bay_mod_first + 1);
        memcpy(zcv[k].hw_display, text, 10);
        blv[k].hw_display[0] = '0';
        for (j = b->bay_mod_first - 1; j < b->bay_mod_last; j++)
        {
          pmv[j].hw_display[0] = '0';
        }
        break;
      
      case 5:                             /* return picks                   */
    
        if (!(t->bf_status & PICKS)) return 0;

        for (i = 0; i < t->bf_pick; i++)
        {
          sprintf((char *)&y, "%04d%02d23%6.6s%04d", tc, t->bf_pi[i],
            t->bf_order, t->bf_picked[i]);
          y.m_port = b->bay_port - 1;  
          message_put(0, TCInputPacketEvent, &y, sizeof(TPacketMessage));
        }
        sprintf((char *)&y, "%04d00000000000000", tc);
        y.m_port = b->bay_port - 1;  
        message_put(0, TCInputPacketEvent, &y, sizeof(TPacketMessage));
        sprintf(text, "CAPS PI=%02d", b->bay_mod_last - b->bay_mod_first + 1);
        memcpy(zcv[k].hw_display, text, 10);
        blv[k].hw_display[0] = '0';
        for (j = b->bay_mod_first - 1; j < b->bay_mod_last; j++)
        {
          pmv[j].hw_display[0] = '0';
        }
        break;

      case 6:                             /* flush and blank                */
    
        t->bf_status &= ENABLED;
        t->bf_max = t->bf_pick = 0;
        memset(zcv[k].hw_display, 0x20, 10);
        blv[k].hw_display[0] = '0';
        for (j = b->bay_mod_first - 1; j < b->bay_mod_last; j++)
        {
          pmv[j].hw_display[0] = '0';
        }
        break;

      case 7:                             /* immediate message              */
    
        memcpy(zcv[k].hw_display, x->m_order, 10);
        break;

      case 8:                             /* no pick                        */
    
        if  (pi) t->bf_status |= (TWOBUTTON | IMMED);
        else     t->bf_status |= (ONEBUTTON | IMMED);
        memcpy(zcv[k].hw_display, x->m_order, 10);
        break;

      case 16:                             /* switch enable/disable          */
    
        if (x->m_quan[3] == '1') t->bf_status &= ~ENABLED;
        else                     t->bf_status |=  ENABLED;
        break;
      
      case 23:                             /* pick                           */
  
        sscanf(x->m_quan, "%04d", &quan);

        if (!(t->bf_status & PICKS))
        {
          j = b->bay_mod_first + pi - 1;
          sprintf(text, "%6.6s%4d", x->m_order, quan);
          memcpy(zcv[k].hw_display, text, 10);
          blv[k].hw_display[0] = '1';
          pmv[j].hw_display[0] = '1';
          memcpy(t->bf_order, x->m_order, 6);
          t->bf_status |= PICKS;
        }
        t->bf_quan[t->bf_max]   = quan;
        t->bf_picked[t->bf_max] = quan;
        t->bf_pi[t->bf_max]     = pi;
        t->bf_max += 1;
        break;
    }
#ifdef DEBUG
  fprintf(stderr, "bay:%d order [%6.6s] [%10.10s] status:%x max:%d pick:%d\n",
   k+1, t->bf_order, zcv[k].hw_display, t->bf_status, t->bf_max, t->bf_pick);
#endif

  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Process Switch:  0 = Box Full; 1 = Short; 2 = Recall; 3 = Next; 4 = Pick
 *-------------------------------------------------------------------------*/
process_switch(k, tc, pi, swt)
register long k, tc, pi, swt;
{
  register bf_status_item *t;
  register struct bay_item *b;
  register long i, j;
  TPacketMessage x;
  char text[16];

  b = &bay[k];
  t = &bf[k];

  if (!(t->bf_status & (IMMED | PICKS))) return 0;

  if (swt == 0)                             /* box full depressed            */
  {
    if (sp->sp_box_full != 'y') return 0;

    if (!(t->bf_status & BOXFULL))
    {
      memcpy(t->bf_save, zcv[k].hw_display, 10);
    }
    memcpy(zcv[k].hw_display, " BOX FULL ", 10);
    t->bf_status |= BOXFULL;
    sprintf((char *)&x, "%04d0100 BOX FULL ", tc);
    x.m_port = b->bay_port - 1;
    message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
    return 0;
  }
  if (t->bf_status & BOXFULL)
  {
    if (swt == 1)                           /* box full + short switch       */
    {
      memcpy(zcv[k].hw_display, "CONVEYABLE", 10);
      sprintf((char *)&x, "%04d0300CONVEYABLE", tc);
      x.m_port = b->bay_port - 1;
      message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
      return 0;
    }
    else if (swt == 2)                     /* box full + recall switch       */
    {
      memcpy(zcv[k].hw_display, t->bf_save, 10);
      t->bf_status &= ~BOXFULL;
      sprintf((char *)&x, "%04d0200 BOX FULL ", tc);
      x.m_port = b->bay_port - 1;
      message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
      return 0;
    }
    else if (swt == 3)                      /* box full + next switch        */
    {
      t->bf_status &= ~BOXFULL;
      sprintf(text, "CAPS PI=%02d", b->bay_mod_last - b->bay_mod_first + 1);
      memcpy(zcv[k].hw_display, text, 10);
      blv[k].hw_display[0] = '0';
      for (j = b->bay_mod_first - 1; j < b->bay_mod_last; j++)
      {
        pmv[j].hw_display[0] = '0';
      }
      for (i = 0; i < t->bf_pick; i++)
      {
        sprintf((char *)&x, "%04d%02d23%6.6s%04d", tc, t->bf_pi[i],
          t->bf_order, t->bf_picked[i]);
        x.m_port = b->bay_port - 1;  
        message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
      }
      t->bf_pick = t->bf_max = 0;
      sprintf((char *)&x, "%04d00000000000000", tc);
      x.m_port = b->bay_port - 1;
      message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
      return 0;
    }
    return 0;
  }
  if (t->bf_status & IMMED)
  {
    if (swt == 1) return 0;                 /* CANT.SHORT not shown          */

    else if (swt == 2)                      /* recall switch                 */
    {
      if (t->bf_status & TWOBUTTON)
      {
        t->bf_status &= ~TWOBUTTON;
        t->bf_status |=  ONEBUTTON;
        memcpy(zcv[k].hw_display, "PRESS NEXT", 10);
      }
      return 0;                             /* NO..RECALL no shown           */
    }
    else if (swt == 3)                      /* next swtich                   */
    {
      if (t->bf_status & ONEBUTTON)
      {
        sprintf((char *)&x, "%04d0008%10.10s", tc, zcv[k].hw_display);
        x.m_port = b->bay_port - 1;

        sprintf(text, "CAPS PI=%02d", b->bay_mod_last - b->bay_mod_first + 1);
        memcpy(zcv[k].hw_display, text, 10);
        blv[k].hw_display[0] = '0';

        message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));

        sprintf((char *)&x, "%04d00000000000000", tc);
        x.m_port = b->bay_port - 1;
        message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
      }
      return 0;                             /* CANT..NEXT not shown          */
    }
    else if (swt == 4) return 0;            /* pi switch ignored             */
  }
  if (swt == 1)                             /* short switch                  */
  {
    if (t->bf_pick >= t->bf_max) return 0;
    
    if (!(t->bf_status & SHORTS)) t->bf_status |= SHORT1;

    i = t->bf_pick;

    if (t->bf_picked[i] == t->bf_quan[i]) t->bf_picked[i] = 0;
    else if (t->bf_status & SHORT1) t->bf_picked[i] += 1;
    else if (t->bf_status & SHORT2) t->bf_picked[i] += 10;
    else if (t->bf_status & SHORT3) t->bf_picked[i] += 100;
    else if (t->bf_status & SHORT4) t->bf_picked[i] += 1000;
  
    if (t->bf_picked[i] > t->bf_quan[i]) t->bf_picked[i] = t->bf_quan[i];

    sprintf(text, "SHORT %4d", t->bf_picked[i]);
    memcpy(zcv[k].hw_display, text, 10);
    return 0;
  }
  if (swt == 2)                             /* recall switch                 */
  {
    if (t->bf_status & SHORT1)      
    {
      t->bf_status ^= (SHORT1 | SHORT2);
      zcv[k].hw_display[8] = '*';
      return 0;
    }
    else if (t->bf_status & SHORT2) 
    {
      t->bf_status ^= (SHORT2 | SHORT3);
      zcv[k].hw_display[7] = '*';
      return 0;
    }
    else if (t->bf_status & SHORT3) 
    {
      t->bf_status ^= (SHORT3 | SHORT4);
      zcv[k].hw_display[6] = '*';
      return 0;
    }
    else if (t->bf_status & SHORT4) return 0;
    
    if (!t->bf_pick) return 0;              /* still on first pi             */
    if (sp->sp_box_full == 'y')
    {
      if (t->bf_status & RECALL) return 0;
      t->bf_status |= RECALL;
    }
    if (t->bf_pick < t->bf_max)
    {
      j = b->bay_mod_first + t->bf_pi[t->bf_pick] - 1;
      pmv[j].hw_display[0] = '0';
    }
    t->bf_pick -= 1;
    sprintf(text, "%6.6s%4d", t->bf_order, t->bf_picked[t->bf_pick]);
    memcpy(zcv[k].hw_display, text, 10);
    j = b->bay_mod_first + t->bf_pi[t->bf_pick] - 1;
    blv[k].hw_display[0] = '1';
    pmv[j].hw_display[0] = '1';
    return 0;
  }
  if (swt == 3)                             /* next switch                   */
  {
    if (t->bf_pick < t->bf_max) return 0;   /* MORE.PICKS not shown          */
    for (i = 0; i < t->bf_max; i++)
    {
      sprintf((char *)&x, "%04d%02d23%6.6s%04d", tc, t->bf_pi[i], 
        t->bf_order, t->bf_picked[i]);
      x.m_port = b->bay_port - 1;
      message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
    }
    sprintf((char *)&x, "%04d00000000000000", tc);
    x.m_port = b->bay_port - 1;
    message_put(0, TCInputPacketEvent, &x, sizeof(TPacketMessage));
    t->bf_status = ENABLED;
    return 0;
  }
  if (swt == 4)                             /* pick switch                   */
  {
    if (pi != t->bf_pi[t->bf_pick]) return 0;/* SWITCH..nn not shown         */

    t->bf_status &= ~(SHORTS | RECALL);

    j = b->bay_mod_first + pi - 1;
    pmv[j].hw_display[0]  = '0';
    
    t->bf_pick += 1;

    if (t->bf_pick < t->bf_max)
    {
      sprintf(text, "%6.6s%4d", t->bf_order, t->bf_picked[t->bf_pick]);
      memcpy(zcv[k].hw_display, text, 10);
      j = b->bay_mod_first + t->bf_pi[t->bf_pick] - 1;
      pmv[j].hw_display[0] = '1';
    }
    else  
    {
      memcpy(zcv[k].hw_display, "PRESS NEXT", 10);
      blv[k].hw_display[0] = '0';
    }
    return 0;
  }
  return 0;
}


/* end of bf_sim.c */
