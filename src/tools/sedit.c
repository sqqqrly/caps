 /*
 *  sedit.c
 *
 *  Full Screen Editor of Screen Images
 *
 *  Execution:  sedit [filename] [width] [-x] [-i] [-t] [-s] [-g]
 *
 *              filename is file name with .s extension or NO extension.
 *              width is 80 or 132.
 *              -s is save input
 *              -i is write image file
 *              -t is write include file
 *              -x is exit without edit.
 *              -g is do not convert graphics
 *
 *  Defaults:   .s ----> .s and .t  (save both)   (option -i useful)
 *  Defaults    .  ----> .          (save only .) (option -s and t useful)
 *
 *  Pure image file is filename of 1841 bytes including null terminator.
 *  C include file is filename.t of an 1841 byte array with null termination.
 *  Editable file is filename.s. Boxes are shown as asterisks.  Video 
 *    attributes ^N is normal, ^U is underline, ^D is dim, ^B is blink,
 *    ^R is reverse dim, and ^Z is end of file.
 *   
 */
/* #include "/u/mfc/src/tools/td.c" */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "td.h"

#define  BARD

//char *memchr();

FILE *fd;                                 /* data file                       */
char sd_name[64]   = {0};                 /* input data file name            */
char td_name[64]   = {0};                 /* output file name if .t          */
char base_name[64] = {0};                 /* base name without extension     */

long in_s = 0;                            /* 0 = . in; 1 = .s in             */
long do_s = 0;                            /* write .s output                 */
long do_i = 0;                            /* write .  output                 */
long do_t = 0;                            /* write .t output                 */
long do_g = 0;                            /* leave graphics as 0xeb - 0xef   */

long do_edit = 1;                         /* interactive editing             */

#define MAX 23                            /* rows in screen                  */

unsigned char image[MAX * 132];           /* crt editor space                */
unsigned char cut[MAX * 132];             /* cut and paste area              */
long cut_pos = 0;                         /* open cut position               */

td_window w = {&td_crt,0,0,MAX,MAX,80,0,-1,image,'t','x',0};

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  register unsigned char *p;
   
  putenv("_=sedit");
  
  for (k = 1; k < argc; k++)
  {
    if (strcmp(argv[k], "80") == 0)       w.td_width = 80;
    else if (strcmp(argv[k], "132") == 0) w.td_width = 132;
    else if (strcmp(argv[k], "-x") == 0)  do_edit = 0;
    else if (strcmp(argv[k], "-s") == 0)  do_s    = 1;
    else if (strcmp(argv[k], "-i") == 0)  do_i    = 1;
    else if (strcmp(argv[k], "-t") == 0)  do_t    = 1;
    else if (strcmp(argv[k], "-g") == 0)  do_g    = 1;
    else strcpy(sd_name, argv[k]);
  }       
  if (!sd_name[0])
  {
    fprintf(stderr, "File Name Is Required\n\n");
    exit(1);
  }
  strcpy(base_name, sd_name);
  
  p = memchr(base_name, '.', strlen(base_name));  /* find dot in file name   */
  
  if (p)                                  /* truncate at the dot             */
  {
    *p = 0;                               /* remove dot in base name         */
    do_s = do_t = in_s = 1;               /* default .s ---> .s and .t       */
  }
  else do_i = 1;                          /* default . ---> .                */
  
  sprintf(td_name, "%s.t", base_name);    /* c include file name             */
  
  load_file();
  if (do_edit) 
  {
    td_open(w.td_width);
    edit_file();
    td_close();
    td_screen_on();
  }
  else save_file();
  exit(1);
}
/*-------------------------------------------------------------------------*
 *  Edit Image
 *-------------------------------------------------------------------------*/
edit_file()
{
  register unsigned char *p, *q, *r;
  register long j, k;
  long rx, cx, ry, cy;
  long overstrike = 1;
  long position = 0;
  long flag = 0;
  char buf[80], video[16];
  unsigned char c;

  td_tab(1, "1  EXIT ");
  td_tab(2, "2  SAVE ");
  td_tab(3, "3 STATUS");
  td_tab(4, "4 VIDEO ");
  td_tab(5, "5  MARK ");
  td_tab(6, "6   BOX ");
  td_tab(7, "7   CUT ");
  td_tab(8, "8 PASTE ");

  sleep(1);
   
  td_update(&w);
  td_refresh();
  td_screen_on();
   
  while (1)
  {
    p = image + td_rowx * w.td_width + td_colx;

    switch (*p)
    {               
      case NORMAL:    strcpy(video, "NORMAL ");    break;
      case UNDERLINE: strcpy(video, "UNDERLINE "); break;
      case DIM:       strcpy(video, "DIM ");       break;
      case BLINK:     strcpy(video, "BLINK ");     break;
      case REVDIM:    strcpy(video, "REVDIM ");    break;
      default:        strcpy(video, "");          break;
    }
    sprintf(buf, "%s ROW:%d COL:%d BYTE:0x%02x %s%s",
      base_name, td_rowx+1, td_colx+1, *p, video, 
      overstrike ? "Overstrike" : "");
    
    if (position) td_message(buf);
    
    td_cursor(td_rowx, td_colx);
    
    c = td_get_byte(&w);

    if (c >= 0x20 && c < 0x7f)
    {
      if (!overstrike) td_open_byte(&w, td_rowx, td_colx);
      *p = c;  
      td_colx = (td_colx + 1) % w.td_width;
      td_update(&w);
      td_refresh();
      continue;
    }
    switch (c)
    {
      case UP:    if (td_rowx > 0) td_rowx -= 1;
                  break;
                
      case ENTER: td_colx = 0;
      case DOWN:  td_rowx = (td_rowx + 1) % w.td_length;
                  break;
                 
      case RIGHT: td_colx = (td_colx + 1) % w.td_width;
                  break;
                  
      case LEFT:  if (td_colx > 0) td_colx -= 1;
                  break;
      
      case TAB:   td_colx = (((td_colx / 8) * 8) + 8) % w.td_width;
                  break;
                  
      case HOME:  td_rowx = td_colx = 0;
                  break;
                  
      case SHIFTHOME: td_colx = w.td_width - 1;
                      break;
                      
      case INSERT:   overstrike ^= 1;
                     break;
      
      case DELETE:   td_delete_byte(&w, td_rowx, td_colx);
                     flag = 1;
                     break;
                   
      case OPENLINE: td_open_line(&w, td_rowx);
                     flag = 1;
                     break;
                     
      case DELINE:   p = image + td_rowx * w.td_width;
                     if (cut_pos < sizeof(cut))
                     {
                       memcpy(&cut[cut_pos], p, w.td_width);
                       cut_pos += w.td_width;
                     }
                     td_delete_line(&w, td_rowx);
                     flag = 1;
                     break;
                     
      case CLRLINE:  td_clear_line(&w, td_rowx);
                     flag = 1;
                     break;
      
      case F1:    td_message("Save Files? (y/n) ");
                  c = td_get_byte(&w);
                  if (c == 'y' || c == 'Y') save_file();
                  return 0;

      case F2:    save_file();
                  break;

      case F3:    position ^= 1;
                  td_send_screen();
                  break;
      
      case F4:    if (*p < NORMAL || *p > REVDIM) *p = NORMAL;
                  else *p = *p + 1;
                  if (*p > REVDIM) *p = SPACE;

                  td_update(&w);
                  td_refresh();
                  td_send_screen();
                
                  break;

      case F5:  rx = ry = td_rowx;        /* save mark of box                */
                cx = cy = td_colx;
                *p = '*';
                flag = 1;
                break;

      case F6:  if (td_rowx > rx) ry = td_rowx;  /* arrange rx <= ry         */
                else              rx = td_rowx;
                
                if (td_colx > cx) cy = td_colx; /* arrange cx <= cy          */
                else              cx = td_colx;
      
                for (k = 0, p = image; k < w.td_length * w.td_width; k++, p++)
                {
                  if (*p >= ULC && *p <= VERT) *p = '*';
                }
                for (k = rx; k <= ry; k++)
                {
                  image[k * w.td_width + cx] = '*';
                  image[k * w.td_width + cy] = '*';
                }
                for (k = cx; k <= cy; k++)
                {
                  image[rx * w.td_width + k] = '*';
                  image[ry * w.td_width + k] = '*';
                }
                for (k = 0, p = image; k < w.td_length; k++)
                {
                  for (j = 0; j < w.td_width; j++)
                  {
                    if (*p == '*') graphic(k, j, p);
                    p++;
                  }
                }
                flag = 1;
                break;
      
      case F7:  cut_pos = 0;              /* clear cut buffer                */
                break;

      case F8:  if (cut_pos > 0)          /* paste cut buffer                */
                {
                  for (k = 0; k < cut_pos; k += w.td_width)
                  {
                    td_open_line(&w, td_rowx);
                  }
                  p = image + td_rowx * w.td_width;
                  memcpy(p, cut, cut_pos);
                  flag = 1;
                }
                break;

      default:  break;                    /* ignore                          */
    }
    if (flag > 0)
    {
      flag = 0;
      td_update(&w);
      td_refresh();
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Load File Into Workspace
 *-------------------------------------------------------------------------*/
load_file()
{
  register long k, line;
  register unsigned char *p;
  unsigned char c;

  td_clear(&w);

  if (!in_s)
  {
    fd = fopen(base_name, "r");
    if (fd == 0) return 0;
    fread(image, w.td_length * w.td_width, 1, fd);
    fclose(fd);
    return 0;
  }
  fd = fopen(sd_name, "r");

  if (fd == 0) return 0;
  
  line = 0;

  p = image;

  while (fread(&c, 1, 1, fd) == 1)
  {
    if (c == 0x1a) break;                 /* end of screen image             */

    if (c == '\n')                        /* end of line                     */
    {
      line += 1;
      p = image + line * w.td_width;
      continue;
    }
    switch (c)
    {
      case 0x0e: c = NORMAL;    break;
      case 0x15: c = UNDERLINE; break;
      case 0x02: c = BLINK;     break;
      case 0x04: c = DIM;       break;
      case 0x12: c = REVDIM;    break;
      default:                  break;
    }
    *p++ = c;
  }
  p = image;
  for (k = 0, p = image; k < w.td_length * w.td_width; k++, p++)
  {
    if (*p == '*') graphic(k / w.td_width, k % w.td_width, p);
  }
  fclose(fd);
  return;
}
/*-------------------------------------------------------------------------*
 *  Test Graphics
 *-------------------------------------------------------------------------*/
graphic(k, j, p)
register unsigned char *p;
register long k, j;
{
  register unsigned char *q;

  static unsigned char xlate[] = {'*',VERT,VERT,VERT,HORT,LRC,URC,RTEE,
    HORT,LLC,ULC,LTEE,HORT,BTEE,UTEE,CROSS};

  if (*p != '*') return;
  
  q = xlate;

  if (k > 0)
  {
    if (*(p - w.td_width) >= ULC && *(p - w.td_width) <= VERT) q += 1;
  }
  if (j > 0) if (*(p-1) >= ULC && *(p-1) <= VERT)        q += 4;
  if (j < w.td_width - 1)  if (*(p+1) == '*')            q += 8;
  if (k < w.td_length - 1) if (*(p + w.td_width) == '*') q += 2;

  *p = *q;

  return;
}
/*-------------------------------------------------------------------------*
 *  Save Image, Text, and/or Include Files
 *-------------------------------------------------------------------------*/
save_file()
{
  if (do_i) save_image();
  if (do_s) save_text();
  if (do_t) save_include();
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Image File
 *-------------------------------------------------------------------------*/
save_image()
{
  fd == fopen(base_name, "w");
  if (fd)
  {
    fwrite(image, w.td_length * w.td_width + 1, 1, fd);
    fclose(fd);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Text File
 *-------------------------------------------------------------------------*/
save_text()
{  
  long j, k;
  unsigned char *p, *q;
  unsigned char LF   = 0x0a;
  unsigned char work[MAX * 132];
  
  memcpy(work, image, MAX * 132);
  
  fd = fopen(sd_name, "w");
  if (fd == 0) return 0;
  
  p = work;
  q = work + w.td_length * w.td_width - 1;

  while (p <= q)
  {
    switch (*p)
    {
      case ULC:
      case URC:
      case LLC:
      case LRC:
      case UTEE:
      case LTEE:
      case RTEE:
      case BTEE:
      case CROSS:
      case HORT:
      case VERT:     *p = '*';  break;

      case NORMAL:   if (!do_g) *p = 0x0e; break;
      case UNDERLINE:if (!do_g) *p = 0x15; break;
      case DIM:      if (!do_g) *p = 0x04; break;
      case BLINK:    if (!do_g) *p = 0x02; break;
      case REVDIM:   if (!do_g) *p = 0x12; break;

      default:       break;
    }
    p++;
  }
  while (q > work && *q == 0x20) q--;
  j = (q - work) / w.td_width;
  
  for (k = 0, p = work; k <= j; k++, p += w.td_width)
  {
    fwrite(p, w.td_width, 1, fd);
    fwrite(&LF, 1, 1, fd);
  }
  fclose(fd);
  return 0;
}

/*-------------------------------------------------------------------------*
 *   Save C Include File
 *-------------------------------------------------------------------------*/
save_include()
{
  register unsigned char *p, *q;
  register long j;

  fd = fopen(td_name, "w");

  p = image;                              /* main window                     */
  q = image + w.td_length * w.td_width - 1; 

  while (q > p && *q == 0x20) q--;        /* find end of data                */
  while ((q - image) % w.td_width) q++;   /* end of line                     */
  
  fprintf(fd, "/*\n");
  fprintf(fd, " *  Screen Image File %s\n", base_name);
  fprintf(fd, " */\n");
  
  fprintf(fd, "unsigned char %s[%d] = {",
    base_name, q - image + 1);

  p = image;
  j = 0;

  while (p < q)
  {
    j--;
    if (j < 0) {fprintf(fd, "\n"); j = 15;}

    fprintf(fd, "%d,", *p++);
  }
  fprintf(fd, "0};\n\n/* end of %s.t */\n\n", base_name);
  fclose(fd);
  return 0;
}

/* end of sedit.c */
