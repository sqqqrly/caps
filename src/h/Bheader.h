/*-------------------------------------------------------------------------*
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  Bheader.h
 *
 *  Index File Header Page 
 */
#ifndef Bheader_h
#define Bheader_h

#include    <stdio.h>
#include    <fcntl.h>
#include    <errno.h>
#include    "Bmodulo.h"
#include    "Bard.h"
#include    "Bfield.h"
#include    "Berror.h"
#include    "Bpage.h"

#ifndef INTEL
extern long errno;
#endif

#define  VERSION     0x051193
#define  KEYS        16                   /* header 256 + 16*44 = 960        */
#define  HEADERSIZE  1024

#define  SHARE       0
#define  EXCLUSIVE   1

typedef struct                            /* key root and definition         */
{
  long           Bk_root;                 /* address of root page            */
  unsigned char  Bk_flags;                /* COMPRESS | NODUPS               */
  unsigned char  Bk_dparts;               /* parts in key data view          */
  unsigned char  Bk_kparts;               /* parts in composite key          */
  unsigned char  Bk_ksize;                /* length of composite key         */
  Bfield_item    Bk_dview[PARTS];         /* key view in data                */
  Bfield_item    Bk_kview[PARTS+1];       /* composite key view              */
   
} Bkey_item;                              /* size is 44 bytes                */

typedef struct                            /* index header page               */
{
  long           Bh_version;              /* version number                  */

  long           Bh_dfree;                /* free data root page             */
  long           Bh_ifree;                /* free index root page            */
  long           Bh_vroot;                /* variable offset root page       */

  unsigned short Bh_rec_size;             /* minimum data record size        */
  unsigned short Bh_rec_max;              /* maximum data record size        */
  unsigned short Bh_keys;                 /* number keys                     */
  unsigned short Bh_log_mode;             /* transaction logging mode        */
   
/* * * * * * * * * *    Information Below Is Transient   * * * * * * * * * */

  unsigned char  Bh_file[40];             /* file name from open             */
  long           Bh_file_fd;              /* data file descriptor            */
  long           Bh_file_no;              /* File number to/from engine      */
     
  long           Bh_index_fd;             /* index file descriptor           */
  long           Bh_log_fd;               /* transaction logging file        */
  long           Bh_mode;                 /* open mode                       */

  long           Bh_locked;               /* current locked record           */
  long           Bh_valid_rec;            /* current valid record            */
  long           Bh_rec_offset;           /* current data record offset      */
  long           Bh_rec_length;           /* current data record size        */

  long           Bh_count;                /* current physical record         */
  long           Bh_phys_offset;          /* current physical position       */
   
  long           Bh_key_no;               /* key number                      */
  Bkey_item      *Bh_key;                 /* pointer to current key          */
  unsigned char  Bh_key_val[MAXKEY];      /* current access key              */
  unsigned char  Bh_key_last[MAXKEY];     /* end of key range                */
  unsigned char  *Bh_buffer;              /* record buffer                   */
   
  Bpage_item     *Bh_leaf;                /* Readfast leaf page              */
  long           Bh_offset;               /* Readfast leaf page offset       */
   
/* * * * * * * * * *    Key Definition Table Size Varies    * * * * * * * * */
/*                        Size To Here is 256 Bytes                         */

  Bkey_item      Bh_key_def[KEYS];        /* key definitions                 */


} Bheader_item;

#endif

/* end of Bheader.h */
