#define INTEL
/*
 *  td.c
 *
 *  Terminal Driver Using Windows and Panes
 *
 *
 *                       window
 *                    {length, width}
 *    window       +-------------------+           screen window
 * {length,width}  |                   |              {24, 80}
 * +-----------+   |                   |       +----------------------+
 * |           |   |      pane         |       |                      |
 * |           |   |  {row,col,size}   |       |     screen pane      |
 * |      base |------>+---------+     | base  |  {row,col,size       |--->CRT
 * |           |   |   |         |     |--------->+--------------+    |
 * |           |   |   +---------+     |       |  |              |    |
 * |           |   |                   |       +--+--------------+----+
 * |           |   |                   |
 * +-----------+   |                   |
 *                 +-------------------+
 *
 *      Window Type Codes (td_type)     Storage Codes (td_form)
 *      ---------------------------     -----------------------
 *      n = unsigned numeric            1 = 1 byte numeric
 *      N = signed numeric              2 = short numeric
 *      d = unsigned decimal            3 = 3 byte numeric
 *      D = signed decimal              4 = word numeric
 *      m = left numeric                x = byte image
 *      M = signed left numeric
 *      z = left numeric, zero fill
 *      a = alpha, no space
 *      A = upper case, no space
 *      t = any text
 *      T = any upper case text
 *      y = yes/no
 *      j = date
 */
#include <stdio.h>
#include <termio.h>
#include "td.h"

  static struct termio td_save;           /* to save terminal state          */

/*
 *  Table to Check and Encode First Byte
 */
  extern unsigned char td_in_byte[256];
  extern unsigned char ta_in_byte[128];
/*
 *  Table to Check and Encode Second Byte if Escape
 */
  extern unsigned char td_in_escape[128];
  extern unsigned char ta_in_escape[128];
/*
 *  Table of Graphics and Video Attributes
 */
  extern unsigned char td_attr[16][4];
  extern unsigned char ta_attr[16][12];
/*
 *  Function Keys
 */
  extern unsigned char td_func[];
  extern unsigned char w370_func[];
/*
 *  Current Screen Areas
 */
  extern short td_first[24];              /* first change in row             */
  extern short td_last[24];               /* last change in row              */

  extern long td_row_offset;              /* window in screen                */
  extern long td_col_offset;              /* window in screen                */

  extern char td_last_message[50];        /* last sreen message              */

  extern unsigned char td_tab_key[];      /* current tab keys                */
  extern unsigned char ta_tab_key[8][9];
  extern unsigned char td_tab_clear[];    /* cleared tab keys                */
  extern unsigned char ta_tab_clear[];
  extern unsigned char td_control[11][6]; /* various control functions       */
  extern unsigned char ta_control[11][6];
  
  extern long td_lock_flag;               /* exclusion flag                  */
  extern long td_need_refresh;            /* screen needs refresh            */

#define td_p(w, x, y)   (w->td_image + (x) * w->td_width + (y))

  unsigned char td_cur_row = 0;
  unsigned char td_cur_col = 0;

  long ansi = 0;                          /* is ansi terminal flag           */
/*
 *  Display Null Terminated String
 */
  td_put_string(p)
  unsigned char *p;
  {
    td_lock_flag = 1;                     /* prevent refresh                 */
    while (*p) putchar(*p++);             /* display string                  */
    fflush(stdout);
    td_lock_flag = 0;                     /* allow refresh                   */
  }

/*
 *  Set Screen n Columns
 */
  td_set(n)
  register long n;
  {
    if (n > 80)
    {
      td_set132();                        /* 132 columns                     */
      td_crt.td_width = 132;
    }
    else
    {
      td_set80();
      td_crt.td_width = 80;               /* 80 columns                      */
    }
    fflush(stdout);
    sleep(1);
    return;
  }
/*
 *  Open Terminal
 */
  td_open(size)
  long size;
  {
    register unsigned char *p, *q;
    register long k;

    ioctl(fileno(stdin), TCFLSH, 2);      /* flush input                     */
    ioctl(fileno(stdout), TCFLSH, 2);     /* flush output                    */
    ioctl(fileno(stdin), TCGETA, &td_save);/* save term state                */
    system("stty -echo");
    system("stty raw");

    p = (unsigned char *)getenv("TERM");
    if (p)
    {
      if (strcmp(p, "ansi") == 0)			ansi = 1;
      else if (strcmp(p, "wyse370") == 0)	ansi = 2;
    }
    if (ansi)
    {  
		memcpy(td_control, ta_control, 66);
		if (ansi == 2)
		{
		  putchar(0x90);
		  td_put_string(w370_func);
		  putchar(0x9c);
		  fflush(stdout);
		}
    }
    td_cursor_off();
    td_screen_clear();
    td_scroll_off();
    td_screen_off();

    if (size != 80) td_set(size);         /* set screen size                 */
    else td_crt.td_width = 80;
   
    p = td_screen;
    q = p + 3168;
    while (p < q) *p++ = SPACE;           /* clear actual image              */

    for (k = 0; k < 24; k++)
    {
      td_last[k]  = -1;                   /* mark unchanged                  */
      td_first[k] = 132;                  /* mark unchanged                  */
    }
    if (!ansi) 
    {
      td_put_string(td_func);             /* setup all functions             */
      td_tabs_on();                       /* set reverse dim                 */
      td_put_string(td_tab_key);          /* load tab keys                   */
      td_message(" ");                    /* clear message area              */
      return;
    }
    for (k = 0; k < 8; k++)               /* ansi key tabs                   */
    {
      td_tab(k+1, ta_tab_key[k]);
    }
    return;
  }
/*
 *  Close Terminal
 */
  td_close()
  {
    register long k;

    td_cursor_off();                      /* cursor off                      */
    td_screen_off();                      /* screen off                      */
    td_screen_clear();                    /* clear entire screen             */
    td_scroll_on();                       /* allow scrolling                 */
    td_tabs_off();                        /* tabs to black (normal)          */
    if (ansi) td_put_string(ta_tab_clear);
    else td_put_string(td_tab_clear);     /* clear all key tabs              */
    if (td_crt.td_width != 80) td_set(80);/* set screen to 80 cols           */
    td_screen_on();                       /* restore screen                  */
    td_cursor_on();                       /* show cursor                     */

    ioctl(fileno(stdin), TCSETAF, &td_save);

    return;
  }
/*
 *  Display In Message Field
 */
  td_message(p)
  register unsigned char *p;
  {
    char text[16];
    
    if (ansi) 
    {
      sprintf(text, "%c[24;1H", ESC);
      td_put_string(text);
      sprintf(td_last_message, "  %-46.46s ", p);
      td_put_string(td_last_message);
      return;
    }
    sprintf(td_last_message, "%cF%.46s\r", ESC, p);
    td_put_string(td_last_message);
    return;
  }
/*
 *  Display Key Tab
 */
  td_tab(key, name)
  long key;
  register unsigned char *name;
  {
    char work[9], text[16];
   
    if (key < 1) return;                  /* invalid key                     */
    if (key > 8) return;

    sprintf(work, "%-8.8s", name);
    if (ansi) 
    {
      strncpy(ta_tab_key[key - 1], work, 8);
      sprintf(text, "%c[25;%dH", ESC, (key - 1) * 10 + 2);
      td_put_string(text);
      fprintf(stdout, "%c[1;34;46m", ESC);
      td_put_string(work);
      fprintf(stdout, "%c[0;37;44m", ESC);
      fflush(stdout);
      return;
    }
    strncpy(&td_tab_key[12 * (key - 1) + 3], work, 8);
    td_put_string(td_tab_key);            /* display all keys                */

    return;
  }
/*
 *  Input And Decode Single Symbol
 */
  unsigned char td_get_byte(w)
  register td_window *w;
  {
    static unsigned char print_message[] =
    {ESC, 'F', 'W', 'A', 'I', 'T', ' ', '-', '-', '-', ' ',
      'P', 'r', 'i', 'n', 't', 'i', 'n', 'g', ' ', 
      'S', 'r', 'e', 'e', 'n', '\r'};
    register unsigned char c;             /* input byte                      */
    register unsigned char *p;
    register long k;
   
    while (1)
    {
      c = getchar();                      /* get any input byte              */
      if (ansi) c = c > 0x7f? 0 : ta_in_byte[c]; /* encode ansi byte         */
      else c = td_in_byte[c];             /* encode input byte               */

      if (!c)                             /* illegal if null                 */
      {
        putchar(BEEP);                    /* beep on illegal byte            */
        continue;                         /* get another byte                */
      }
      if (c != ESC) return c;             /* is single byte                  */

      c = (getchar() & 0x7f);             /* get second byte                 */
      if (ansi)
      {
        if (c != '[') {putchar(BEEP); continue;}
        c = getchar() & 0x7f;
        c = ta_in_escape[c];
      }
      else c = td_in_escape[c];           /* encode input byte               */

      if (!c)                             /* illegal if null                 */
      {
        putchar(BEEP);                    /* beep on illegal ESC             */
        continue;                         /* get another input               */
      }
      if (c == PRINT)                     /* print screen                    */
      {
        putchar(BEEP);
        td_put_string(print_message);
        td_lock_flag = 1;
        td_print_screen(w);
        td_lock_flag = 0;
        td_message(" ");
        continue;
      }
      else if (c == SEND)                 /* repaint screen                  */
      {
        putchar(BEEP);
        td_message("WAIT --- Painting Screen");
        td_cursor_off();
        td_scroll_off();

        td_lock_flag = 1;                 /* prevent refresh                 */
        td_send_screen();
        td_lock_flag = 0;                 /* enable refresh                  */

        if (!ansi) 
        {  
          td_put_string(td_func);         /* setup all function keys         */
          td_put_string(td_tab_key);      /* display key tabs                */
          td_tabs_on();                   /* tabs in reverse dim             */
          td_message(" ");                /* clear message line              */
          return SEND;
        }
        td_message(" ");
        for (k = 0; k < 8; k++)           /* ansi key tabs                   */
        {
          td_tab(k+1, ta_tab_key[k]);
        }
        return SEND;
      }
      return c;                           /* return ESC encoded              */
    }
  }
/*
 *  Encode And Output Single Symbol At Cursor To The Screen
 *              *** All Windows Must Be Aligned ***
 */
  td_put_byte(w, row, col, c)
  register td_window *w;
  register long row, col;
  register unsigned char c;
  {
    register unsigned char *p;

    td_lock_flag = 1;                     /* prevent refresh                 */

    if (c >= SPACE && c < DELETE)         /* is printable byte               */
    {
      putchar(c);                         /* output byte as is               */
    }
    if (c >= ULC && c <= REVDIM)
    {
      if (ansi) p = ta_attr[c - ULC];
      else p = td_attr[c - ULC];          /* is graphic/video                */
      while (*p) putchar(*p++);
    }
    td_cur_col++;                         /* output one byte only            */
   
    while (1)                             /* try to store above              */
    {
      *td_p(w, row, col) = c;             /* store byte in window            */

      if (!w->td_pwin) break;             /* no window above                 */

      row -= w->td_base;                  /* relative to pane                */
      if (row < 0 || row >= w->td_size) return;/* outside of pane            */

      row += w->td_row;                   /* in next window                  */
      col += w->td_col;                   /* in next window                  */
      w    = w->td_pwin;                  /* window above                    */
    }
    td_lock_flag = 0;                     /* allow refresh                   */
    return;
  }
/*
 *  Align Relative to Window And Determine Cursor Offsets
 */
  td_align(w, row, top)
  register td_window *w;
  register long row, top;
  {
    register long new, max, new_base;
   
    new_base = -1;                        /* no preferred yet                */
    td_row_offset = -row;                 /* save window row                 */
    td_col_offset = 0;

    while (w)                             /* not screen yet                  */
    {
      if (new_base >= 0 && w->td_base != new_base)
      {
        w->td_base = new_base;            /* preferred base here             */
        td_update(w);
      }
      new_base = w->td_pbase;             /* preferred above                 */

      if (row < w->td_base || row >= w->td_base + w->td_size)
      {
        max = w->td_length - w->td_size;  /* maximum row                     */

        if (w->td_size == 24 && top)
        {
          new = (row / 24) * 24;
        }
        else if (top)
        {
          new = row;                      /* top or pane                     */
          if (new > max) new = max;
        }
        else
        {
          new = row - (w->td_size >> 1);  /* middle of pane                  */
          if (new < 0) new = 0;           /* top of window                   */
          else if (new > max) new = max;  /* bottom of window                */
        }
        if (new != w->td_base)            /* base will move                  */
        {
          w->td_base = new;               /* update base                     */
          td_update(w);                   /* update image                    */
        }
      }
      td_col_offset += w->td_col;         /* in window above                 */
      row =  w->td_row + row - w->td_base;/* in window above                 */
      w = w->td_pwin;                     /* next window                     */
    }
    td_row_offset += row;

    return 0;
  }
/*
 *  Position Cursor Relative to Aligned Window
 */
  td_cursor(row, col)
  register long row, col;
  {
    register char *p;
    register long work;
    
    td_lock_flag = 1;                     /* prevent refresh                 */

    row += td_row_offset;                 /* relocate to screen              */
    col += td_col_offset;

    td_cur_row = row;                     /* save position                   */
    td_cur_col = col;
   
    if (ansi)
    {
      fprintf(stdout, "%c[%d;%dH", ESC, row + 1, col + 1);
    }
    else if (col < 80)                    /* 80 column screen                */
    {
      putchar(ESC);
      putchar('=');
      putchar(row + SPACE);               /* zero based                      */
      putchar(col + SPACE);
    }
    else                                  /* 132 column screen               */
    {
      row++; col++;                       /* one based                       */

      putchar(ESC);
      putchar('a');

      if (row > 19) {putchar('2'); row -= 20;}
      else if (row > 9) {putchar('1'); row -= 10;}
      putchar(row + '0');

      putchar('R');

      if (col > 100) {putchar('1'); col -= 100;}
      work = col / 10;
      putchar (work + '0');
      putchar (col - 10 * work + '0');
      putchar('C');
    }
    td_lock_flag = 0;
    fflush(stdout);
    return;
  }
/*
 *  Video Attribute of a Window In Screen Image
 */
  td_video(w, video)
  register td_window *w;
  register unsigned char video;
  {
    register td_window *x;
    register long k;
    register unsigned char *q;

    x = w->td_pwin;                       /* window above                    */

    if (w->td_col < 1) return;
    if (w->td_col + w->td_width >= x->td_width) return;

    q = td_p(x, w->td_row, w->td_col);    /* address in image                */

    for (k = 0; k < w->td_size; k++)
    {
      *(q + w->td_width) = NORMAL;
      *(q - 1) = video;
      q += x->td_width;
    }
    td_update(x);
    td_refresh_screen(0);                 /* update and display              */
    return;
  }
/*
 *  Clear A Window To A Fill Symbol
 */
  td_clear(w)
  register td_window *w;
  {
    register unsigned char form, fill;
    register unsigned char *p, *q;

    form = w->td_form;                    /* format of data                  */
   
    p = w->td_image;                      /* start of window                 */

    if (form == 'x')                      /* alpha format                    */
    {
      fill = SPACE;                       /* alpha format                    */
      q = p + w->td_length * w->td_width; /* end of window                   */
    }
    else                                  /* binary format                   */
    {
      fill = 0;                           /* numeric format                  */
      q = p + w->td_length * (form - '0');/* clear binary data               */
    }
    while (p < q) *p++ = fill;            /* copy fill bytes                 */

    w->td_base = 0;                       /* top of window                   */

    return;
  }
/*
 *   Refresh Sreen Window - Alpha Format
 */
  td_refresh_screen(flag)
  register long flag;
  {
    register td_window *w;
    register short *first, *last;

    register unsigned char *q, *r, *s;
    register long k;
    unsigned char x, y;
    long row, col , work;

    if (!td_need_refresh) return 0;       /* not needed                      */
    if (td_lock_flag) return 0;           /* something is active             */
    td_lock_flag = 1;                     /* prevent interrupt               */
   
    td_need_refresh = 0;                  /* clear refresh needed            */
   
    w = &td_crt;
    first = td_first;
    last  = td_last;

    for (k=0; k<24; k++)                  /* for all rows in screen          */
    {
      if (*last < 0)                      /* nothing is changed              */
      {
        first++; last++; continue;
      }
      q = w->td_image + k * w->td_width;
      r = q + *last;
      q += *first;

      if (ansi)
      {
        fprintf(stdout, "%c[%d;%dH", ESC, k + 1, *first + 1);
        fflush(stdout);
      }
      else if (*first < 80)               /* short move cursor               */
      {
        putchar(ESC);
        putchar('=');
        putchar(k + SPACE);
        putchar(*first + SPACE);
      }
      else                                /* long move cursor                */
      {
        row = k + 1; col = *first + 1;

        putchar(ESC);
        putchar('a');

        if (row > 19)     {putchar('2'); row -= 20;}
        else if (row > 9) {putchar('1'); row -= 10;}
        putchar('0' + row);
        putchar('R');

        if (col > 99) {putchar('1'); col -= 100;}
        work = col / 10;
        putchar(work + '0');
        putchar(col - 10 * work + '0');
        putchar('C');
      }
      while (q <= r)                      /* output data  line               */
      {
        if (*q >= SPACE && *q < DELETE) putchar(*q++);
        else
        {
          if (ansi) s = ta_attr[*q - ULC];
          else s = td_attr[*q - ULC];
          while(*s) putchar(*s++);
          q++;
        }
      }
      *first++ = 132;
      *last++  = -1;
    }
    if (td_crt.td_width == 80 && flag)
    {
      if (ansi) 
      {
        fprintf(stdout, "%c[%d;%dH", 
          ESC, td_cur_row + 1, td_cur_col + 1);
      }
      else
      {
        putchar(ESC); 
        putchar('=');                     /* restore cursor position         */
        putchar(td_cur_row + SPACE); 
        putchar(td_cur_col + SPACE);
      }
    }
    fflush(stdout);
    td_lock_flag = 0;
  }
/*
 *  Clear A Line In a Window - Relative Row
 */
  td_clear_line(w, row)
  register td_window *w;
  register long row;
  {
    register unsigned char *p, *q;

    p = td_p(w, row, 0);                  /* address in image                */
    q = p + w->td_width;

    while (p < q) *p++ = SPACE;
    return;
  }
/*
 *  Delete a Line In a Window - Relative Row
 */
  td_delete_line(w, row)
  register long row;
  register td_window *w;
  {
    register unsigned char *p, *q, *r;

    p = td_p(w, row, 0);                  /* row to delete                   */
    q = p + w->td_width;                  /* next row                        */
    r = td_p(w, w->td_length, 0);         /* end of image                    */

    while (q < r) *p++ = *q++;
    while (p < r) *p++ = SPACE;

    return;
  }
/*
 *  Open Line In Window - Relative Row
 */
  td_open_line(w, row)
  register td_window *w;
  register long row;
  {
    register unsigned char *p, *q, *r;

    p = td_p(w, row, 0);                  /* address in image                */
    r = td_p(w, w->td_length, -1);
    q = r - w->td_width;

    while (q >= p) *r-- = *q--;
    while (p <= r) *p++ = SPACE;

    return;
  }
/*
 *  Delete Byte In Window Line - Relative Row and Column
 */
  td_delete_byte(w, row, col)
  register td_window *w;
  register long row, col;
  {
    register unsigned char *p, *q, *r;

    p = td_p(w, row, 0);                  /* start of line                   */
    r = p + w->td_width;                  /* end of line                     */
    p += col;                             /* start of data                   */
    q = p + 1;                            /* start + 1 of data               */

    while (q < r) *p++ = *q++;            /* shift over by one               */
    *p = SPACE;                           /* space at end                    */
    return;
  }
/*
 *  Open Byte In a Window Line - Relative Row and Column
 */
  td_open_byte(w, row, col)
  register td_window *w;
  register long row, col;
  {
    register unsigned char *p, *q, *r;

    p = td_p(w, row, 0);                  /* start of line                   */
    q = p + w->td_width - 2;              /* origin of move                  */
    p += col;                             /* end of move                     */
    r = q + 1;                            /* destination of move             */

    while (q >= p) *r-- = *q--;           /* shift down                      */
    *p = SPACE;                           /* insert space                    */
    return;
  }
/*
 *  Check Decimal in Window Row
 */
  td_check_decimal(w, row)
  register td_window *w;
  long row;
  {
    register unsigned char *p, *q;

    p = td_p(w, row, 0);                  /* address in image                */
    q = p + w->td_width;

    while (p < q)
    {
      if (*p++ == '.') return 1;          /* decimal found                   */
    }
    return 0;
  }
/*
 *  Get Data Into A Window
 */
  unsigned char td_get_window(z, rx, cx)
  td_window *z;
  long rx, cx;
  {
    register td_window *w, *pane;
    register unsigned char c;
    register unsigned char *p, *q;
    register long row, col, mode;
    register  unsigned char type, form;
    td_window x;                          /* numeric window                  */
    unsigned char work[512];              /* numeric work area               */
    long length, width, size;
    short packed;
   
    form = z->td_form;                    /* data format                     */
    type = z->td_type;                    /* data type                       */

    if (form != 'x')                      /* numeric window                  */
    {
      x = *z;                             /* copy call window                */
      w = &x;                             /* now point to copy               */
      x.td_image = work;                  /* assign work area                */
      x.td_form = 'x';                    /* alpha format window             */
      td_putn(w, z->td_image, form);      /* to display form                 */
    }
    else w = z;                           /* use window in call              */

    length = w->td_length;                /* useful values                   */
    width  = w->td_width;
    size   = w->td_size;                  /* max pane size                   */
    pane   = w->td_pwin;                  /* window above                    */
      
    while (pane)                          /* find smallest pane              */
    {
      if (pane->td_size < size) size = pane->td_size;
      pane = pane->td_pwin;
    }

    if (rx >= length) rx = length - 1;    /* within window                   */
    if (cx >= width)  cx = width - 1;     /* within window                   */

    row = rx; col = cx; mode = 0;         /* initial values                  */
    td_change = 0;                        /* field not changed               */

    switch (type)                         /* right fill fields               */
    {
      case 'n':
      case 'N':
      case 'd':
      case 'D': mode = 2; col = width - 1;/* numeric right fill              */
        break;

        default:  break;
      }
    td_align(w, row, 1);                  /* window is displayable           */
    td_refresh_screen(0);                 /* refresh updated                 */
    td_cursor(row, col);                  /* position cursor                 */
    td_cursor_on();                       /* turn on cursor                  */

    while (1)                             /* until window done               */
    {
      c = td_get_byte(w);                 /* get one symbol                  */

      if (c >= SPACE && c < DELETE)       /* is a real data                  */
      {
        if (col >= width)                 /* end of window                   */
        {
          putchar(BEEP); continue;        /* no more space                   */
        }
        switch (type)                     /* check input on type             */
        {
          case 'y':  if (c == 'Y' || c == 'N') break;
            if (c == 'y') {c = 'Y'; break;}
            if (c == 'n') {c = 'N'; break;}
            c = 0;
            break;

          case 'T':  if (c >= 'a' && c <= 'z') c -= SPACE;
          case 't':  break;               /* any text allowed                */
            
          case 'A':  if (c >= 'a' && c <= 'z') {c -= SPACE; break;}
          case 'a':  if (c >= '0' && c <= '9') break;
            if (c == SPACE) c = 0;
            break;

          case 'M':  if (!col && (c == '+' || c == '-')) break;
            if ( c < '0' || c > '9') c = 0;
            break;
                       
          case 'N':  if (*td_p(w, row, col) == SPACE &&
            (c == '+' || c == '-')) break;

          case 'j':  if (c == '/') break;
          case 'z':
          case 'm':
          case 'n':  if (c < '0' || c > '9') c = 0;
            break;

          case 'D':  if (*td_p(w, row, col) == SPACE &&
            (c == '+' || c == '-')) break;

          case 'd':  if (c >= '0' && c <= '9') break;
            if (c != '.') {c = 0; break;}
            if (td_check_decimal(w, row)) c = 0;
            break;

            default:    c = 0;
            break;
          }
        if (!c)                           /* null is bad                     */
        {
          putchar(BEEP); continue;        /* bad input byte                  */
        }
        td_change = 1;                    /* window changed                  */

        if (mode == 2)                    /* numeric mode                    */
        {
          td_delete_byte(w, row, 0);      /* delete leftmost byte            */
        }
        if (mode)                         /* insert mode                     */
        {
          td_open_byte(w, row, col);      /* open space                      */
          td_update(w);
          td_refresh_screen(0);           /* echo window                     */
          td_cursor(row, col);
        }
        td_put_byte(w, row, col, c);      /* echo byte                       */
        col++;                            /* next column                     */

        if (col >= width)                 /* end of a line                   */
        {
          if (type != 't' && type != 'A' && type != 'm'
          && type != 'z' && type != 'M')
          {col--; putchar(BS); td_cur_col--; continue;}

          if (td_no_wrap) continue;       /* no automatic linefeed           */
            
          row++;
          if (row >= length)              /* end of window                   */
          {
            row--;
            continue;                     /* end of window                   */
          }
          col = 0;
          td_align(w, row, 0);
          td_refresh_screen(0);
          td_cursor(row, 0);              /* new line                        */
        }
        continue;
      }
      if (c == SEND)
      {
        td_cursor_on();
        continue;
      }
      else if (c == PRINT) continue;
      else if (c == INSERT)
      {
        if (mode != 2) mode = 1;          /* insert mode                     */
        continue;
      }
      else if (c == OVERSTRIKE)
      {
        if (mode != 2) mode = 0;          /* overstrike mode                 */
        continue;
      }
      if (type == 'j' && td_change)       /* check valid date                */
      {
        if (c != DELETE && c != DELINE && c != CLRLINE)
        {
          q = td_p(w, row, 0);
         
          if (Bbreak_date(q, &packed) == -1)
          {
            putchar(BEEP); continue;      /* invalid date form               */
          }
          Bexpand_date(packed, q);
            
          q = (unsigned char *)&packed;   /* store validated date            */
          *(z->td_image) = *q;
          *(z->td_image + 1) = *(q + 1);

          td_update(w);
          td_refresh_screen(0);
        }
      }
      switch (c)                          /* control code                    */
      {
        case DELETE:   td_change = 1;

          if (mode == 2)                  /* numeric field                   */
          {
            td_delete_byte(w, row, col);
            td_open_byte(w, row, 0);
            td_num_update(w, row, z);
            break;
          }
          if (col <= 0)
          {
            putchar(BEEP);
            break;
          }
          else col--;

          if (mode == 0)                  /* overstrike                      */
          {
            putchar(BS);
            td_put_byte(w, row, col, SPACE);
            putchar(BS); td_cur_col--;
          }
          else                            /* insert mode                     */
          {
            td_delete_byte(w, row, col);
            td_update(w);
          }
          break;

        case SHIFTAB:  col = width - 1;
          break;

        case F1:
        case F2:
        case F3:
        case F4:
        case F5:
        case F6:
        case F7:
        case F8:
        case HELP:     mode = 3;
          td_num_update(w, row, z);
          break;

        case LEFT:     if (col <= 0)
          {
            if (row <= 0)
            {
              putchar(BEEP);
              break;
            }
            td_num_update(w, row, z);
            row--;
            col = width - 1;
            if (mode != 2) mode = 0;
            td_align(w, row, 0);
            putchar(BEEP);
            break;
          }
          col--;
          break;

        case RIGHT:    col++;
          if (col >= width)
          {
            putchar(BEEP);
            td_num_update(w, row, z);
            row++;
            if(row >= length)
            {
              row = length - 1;
              col = width;
              break;
            }
            col = 0;
            if (mode != 2) mode = 0;
            td_align(w, row, 0);
          }
          break;

        case UP:       td_num_update(w, row, z);
          if (td_col_zero) col = 0;
                        
          if (row > 0)
          {
            if (col >= width) col = width - 1;
            row--;
            if (mode != 2) mode = 0;
            td_align(w, row, 0);
            break;
          }
          mode = 3;                       /* UP exit                         */
          break;

        case TAB:      col += 8;          /* tab over in field               */
          if (col < width) break;         /* otherwise fall through          */

        case DOWN:     td_num_update(w, row, z);
          if (td_col_zero) col = 0;
          row++;
          if (row < length)
          {
            if (mode != 2) mode = 0;
            td_align(w, row, 0);
            break;
          }
          mode = 3;
          break;

        case FORMDOWN:
        case FORMUP:
        case FORMHOME: mode = 3;          /* exit key                        */
          td_num_update(w, row, z);
          break;

        case ENTER:    mode = 3;          /* ENTER exit                      */
          td_num_update(w, row, z);
          break;

        case HOME:     mode = 3;          /* HOME exit                       */
          td_num_update(w, row, z);
          c = LEFT;
          break;

        case SHIFTHOME: td_num_update(w, row, z);
          row = col = 0;
          if (mode != 2) mode = 0;
          td_align(w, 0, 1);
          break;

        case OPENLINE: td_change = 1;
          if (length <= 1) break;
          td_open_line(w, row);
          td_num_update(w, row, z);
          col = 0;
          if (mode != 2) mode = 0;
          break;

        case DELINE:   td_change = 1;
          if (length <= 1) break;
          td_delete_line(w, row);
          td_num_update(w, row, z);
          col = 0;
          if (mode != 2) mode = 0;
          break;

        case CLRLINE:  td_change = 1;
          td_clear_line(w, row);
          td_num_update(w, row, z);
          col = 0;
          if (mode != 2) mode = 0;
          break;

        case PREVPAGE: td_num_update(w, row, z);
          row = w->td_base - size;
          if (row < 0) row = 0;
          td_align(w, row, 1);
          col = 0;
          break;

        case NEXTPAGE: td_num_update(w, row, z);
          row = w->td_base + size;
          if (row > length - size) row = length - size;
          td_align(w, row, 1);
          col = 0;
          break;

          default:       break;
        }
      td_refresh_screen(0);               /* update screen                   */

      if (mode > 2) break;

      if (type == 'A' || type == 'a' || type == 'z' ||
      type == 'm' || type == 'M')
      {
        p = td_p(w, row, col);
        while (col > 0)
        {
          if (*(p - 1) != SPACE) break;
          col--; p--;
        }
      }
      else if (mode == 2)                 /* check left of row               */
      {
        p = td_p(w, row, col);
        while (col < width - 1)
        {
          if (*p >= '0' && *p <= '9') break;
          if (*p == '.') break;
          col++; p++;
        }
      }
      td_cursor(row, col);
    }
    td_cursor_off();                      /* exit wrapup                     */
    td_rowx = row;
    td_colx = col;

/*  if (z->td_form == 'x') td_strip(z);  no longer stripped                  */

    return c;

  }
/*
 *  Strip Trailing Spaces
 */
  td_strip(w)
  register td_window *w;
  {
    register unsigned char *q;

    q = w->td_image + w->td_length * w->td_width - 1;

    while (q >= w->td_image)              /* strip any trailing spaces       */
    {
      if (*q != 0x20) break;              /* not a space                     */
      *q-- = 0;                           /* remove trailing spaces          */
    }
  }
/*
 *  Update Window And All Windows Above - Mark Screen Refresh Area
 */
  td_update(w)
  register td_window *w;
  {
    register td_window *x;
    register unsigned char *q, *r, *s;
    register long j, k;
    register short *first, *last;
    long dmin, dcol, smin, scol, width, length, max;
    unsigned char work[512];
    td_window z;

    if (w->td_form != 'x')                /* numeric format                  */
    {
      z = *w;                             /* copy window                     */
      z.td_form = 'x';
      z.td_image = work;
      td_putn(&z, w->td_image, w->td_form);/* display format                 */
      w = &z;
    }
    x = w->td_pwin;                       /* window above window             */

    dmin = w->td_row;                     /* destination corner              */
    dcol = w->td_col;

    smin = w->td_base;                    /* source corner                   */
    scol = 0;

    width  = w->td_width;                 /* pane size                       */
    length = w->td_size;
/*
 *  Update Below Screen
 */
    while (x->td_pwin)                    /* until screen                    */
    {
      for (k=0; k<length; k++)            /* for all rows                    */
      {
        q = td_p(w, smin + k, scol);      /* source data                     */
        r = td_p(x, dmin + k, dcol);      /* destination data                */
        s = r + width;
        while(r < s)
        {
          if (*q) *r++ = *q++;            /* copy data                       */
          else {*r++ = 0x20; q++;}        /* change null to space            */
        }
      }
      smin = dmin;                        /* new source row                  */
      scol = dcol;                        /* new source col                  */

      w = x;                              /* new source window               */
      x = w->td_pwin;                     /* new destination pane            */

      if (smin >= w->td_base + w->td_size) return 0;
      else if (smin + length <= w->td_base) return 0;
      else if (smin < w->td_base)         /* some above next pane            */
      {
        length = length + smin - w->td_base;/* new length                    */
        smin = w->td_base;
      }
      max = w->td_size + w->td_base - smin;/* max possible length            */
      if (max < length) length = max;

      dmin = w->td_row + smin - w->td_base;
      dcol = w->td_col + scol;
    }
/*
 *  Copy To Screen
 */
    first = td_first + dmin;
    last  = td_last  + dmin;

    for (k=0; k<length; k++)              /* all rows in pane                */
    {
      q = td_p(w, smin + k, scol);        /* source data                     */
      r = td_p(x, dmin + k, dcol);        /* destination data                */

      for (j = dcol; j < dcol + width; j++)/* all columns in data            */
      {
        if (*r != *q)                     /* test has changed                */
        {
          if (j < *first) *first = j;
          if (j > *last)  *last  = j;
          *r = *q;                        /* copy data                       */
          td_need_refresh |= 1;
        }
        r++; q++;
      }
      first++; last++;
    }
    return 0;
  }
/*
 *  Put Null Terminated Alpha Field in Window
 */
  td_put(w, r)
  register td_window *w;
  register unsigned char *r;
  {
    register unsigned char *p, *q;

    w->td_base = 0;

    p = w->td_image;
    q = p + w->td_length * w->td_width;

    while (p < q)
    {
      if (*r) *p++ = *r++;
      else *p++ = SPACE;
    }
    return;
  }
/*
 *  Put Numeric Fields in Window
 */
  td_putn(w, table, form)
  register td_window *w;
  register unsigned char *table;
  register unsigned char form;
  {
    register long j, k;
    register unsigned char *p, *q;
    union
    {
      long word;
      unsigned char bytes[4];
    } x;
    short packed;
   
    w->td_base = 0;

    for (k=0; k<w->td_length; k++)        /* for all fields                  */
    {
      j = form & 0x0f;                    /* number of bytes                 */
      x.word = 0;
#ifdef INTEL
      if ((*(table + j - 1) & 0x80) && 
            w->td_type >= 'A' && w->td_type <= 'Z')
      {
        x.word = -1;
      }
      else x.word = 0;
      q = x.bytes;
#else
      if ((*table & 0x80) && w->td_type >= 'A' && w->td_type <= 'Z')
      {
        x.word = -1;
      }
      else x.word = 0;
      q = x.bytes + 4 - j;
#endif
      for(; j > 0; j--) *q++ = *table++;
      
      switch (w->td_type)
      {
        case 'j': packed = x.word;
          Bexpand_date(packed, td_p(w, k, 0));
          break;
                   
        case 'n':
        case 'N': td_putfl(w, k, x.word);
          break;

        case 'M':
        case 'm': td_putfl(w, k, x.word);
          p = td_p(w, k, 0);
          while (*p == SPACE) td_delete_byte(w, k, 0);
          break;

        case 'z': td_putfl(w, k, x.word);
          p = td_p(w, k, 0);
          while (*p == SPACE) *p++ = '0';
          break;

          default:  td_putfd(w, k, x.word);
          break;
        }
    }
  }
/*
 *  Put Numeric Field To Screen - Right Justified
 */
  td_putfl(w, row, value)
  register td_window *w;
  register long row, value;
  {
    register long width;
    register long x;
    register unsigned char *p;
    unsigned char sign = SPACE;
   
    width = w->td_width;
    p = td_p(w, row, width - 1);

    if (value < 0) {sign = '-'; value = -value;}

    while (width > 0)
    {
      x = value / 10;
      *p-- = value - 10 * x + '0';
      width--;
      value = x;
      if (!value) break;
    }
    while (width > 0)
    {
      *p-- = sign;
      sign = SPACE;
      width--;
    }
    if (value > 0 || sign == '-') *++p = '*';
    return;
  }
/*
 *  Put Numeric Decimal Field To Screen - Right Justified
 */
  td_putfd(w, row, value)
  register td_window *w;
  long row;
  register long value;
  {
    register long width;
    register long x;
    register unsigned char *p;
    register long k;
    unsigned char sign  = SPACE;

    width = w->td_width;
    p = td_p(w, row, width - 1);

    if (value < 0) {sign = '-'; value = -value;}

    for (k=0; k<2; k++)
    {
      x = value / 10;
      *p-- = value - 10 * x + '0';
      width--;
      value = x;
    }
    *p-- = '.';
    width--;

    while (width > 0)
    {
      x = value / 10;
      *p-- = value - 10 * x + '0';
      width--;
      value = x;
      if (!value) break;
    }
    while (width > 0)
    {
      *p-- = sign;
      sign = SPACE;
      width--;
    }
    if (value > 0 || sign == '-') *++p = '*';
    return;
  }
/*
 *  Get Null Terminated Alpha Field From a Window
 */
  td_get(w, r)
  register td_window *w;
  register unsigned char *r;
  {
    register unsigned char *p, *q;
    register long m;

    m = td_window_size(w);                /* get true length                 */

    p = w->td_image;
    q = p + w->td_length * w->td_width;

    while (p < q)
    {
      if (m > 0) *r++ = *p;
      else *r++ = 0;
      p++;
    }
    return 0;
  }
/*
 *  Update One Numeric Field
 */
  td_num_update(w, row, z)
  register td_window *w;                  /* alpha window                    */
  register td_window *z;                  /* numeric window                  */
  register long row;                      /* row to update                   */
  {
    if (!td_change) return 0;

    if (z->td_form != 'x')
    {
      switch(w->td_type)
      {
        case 'n':
        case 'N':
        case 'd':
        case 'D':
        case 'm':
        case 'M':
        case 'z':   td_getn(w, z->td_image, z->td_form);
          td_putn(w, z->td_image, z->td_form);
          break;

          default:    break;
        }
    }
    return td_update(w);
  }
/*
 *  Get Numeric Window by Type
 */
  td_getn(w, table, form)
  register td_window *w;
  register unsigned char *table;
  register unsigned char form;
  {
    register unsigned char *q;
    register long j, k;
    union
    {
      long word;
      unsigned char bytes[4];
    } x;
    short packed;
   
    for (k=0; k<w->td_length; k++)
    {
      switch (w->td_type)
      {
        case 'j': packed = 0;
          Bbreak_date(td_p(w, k, 0), &packed);
          x.word = packed;
          break;

        case 'z':
        case 'n':
        case 'm':
        case 'M':
        case 'N': x.word = td_getfl(w, k);
          break;

        case 'd':
        case 'D': x.word = td_getfd(w, k);
          break;

          default:  return;
        }
      j = form & 0x0f;                    /* number of bytes                 */
#ifdef INTEL      
      q = x.bytes;
#else
      q = x.bytes + 4 - j;
#endif
      for (; j > 0; j--) *table++ = *q++;
    }
  }
/*
 *  Get Numeric Long Field
 */
  td_getfl(w, row)
  register td_window *w;
  long row;
  {
    register long width;
    register unsigned char *p, *q;
    register long x = 0;
    register unsigned char sign = '+';

    width = w->td_width;

    p = td_p(w, row, 0);                  /* field in screen                 */
    q = p + width;                        /* end of field                    */

    while (p < q && *p == SPACE) p++;     /* skip leading spaces             */

    if (*p == '+' || *p == '-') sign = *p++;/* save sign if any              */

    while (p < q)
    {
      if (*p < '0' || *p > '9') break;    /* not a digit                     */
      x = 10 * x + (*p++ & 0x0f);         /* convert to decimal              */
    }
    if (sign == '-') return -x;           /* negative value                  */
    return x;                             /* positive value                  */
  }
/*
 *  Get Numeric Long Decimal Field
 */
  td_getfd(w, row)
  register td_window *w;
  long row;
  {
    register long width;
    register unsigned char *p, *q;
    register long dp = 16;
    register long x = 0;
    register unsigned char sign = '+';

    width = w->td_width;

    p = td_p(w, row, 0);                  /* field in screen                 */
    q = p + width;                        /* end of field                    */

    while (p < q && *p == SPACE) p++;     /* skip leading spaces             */

    if (*p == '+' || *p == '-') sign = *p++;/* save sign if any              */

    while (p < q && dp > 0)
    {
      if (*p == '.') {dp = 2; p++; continue;}
      if (*p < '0' || *p > '9') break;    /* not a digit                     */
      x = 10 * x + (*p++ & 0x0f);         /* convert to decimal              */
      dp--;
    }
    if (dp >= 2) x = 100 * x;
    if (dp == 1) x = 10 * x;

    if (sign == '-') return -x;           /* negative value                  */
    return x;                             /* positive value                  */
  }
/*
 *  Get Length of Window
 */
  td_window_size(w)
  register td_window *w;
  {
    register unsigned char *p, *q;

    p = w->td_image;
    q = p + w->td_length * w->td_width -1;

    while (p <= q)
    {
      if (*q != SPACE) break;
      q--;
    }
    return (q + 1 - p);
  }
/*
 *  Refresh Actual Screen
 */
  td_send_screen()
  {
    register unsigned char *p;
    register long j, k;

    p = td_screen;

    for (k = 0; k < 24; k++)
    {
      if (ansi) fprintf(stdout, "%c[%d;1H", ESC, k + 1);
      else
      {
        putchar(ESC); 
        putchar('='); 
        putchar(SPACE + k); 
        putchar(SPACE);
      }
      for (j = 0; j < td_crt.td_width; j++)
      {
        if (*p >= SPACE && *p < DELETE) putchar(*p);

        else if (*p >= ULC && *p <= REVDIM)
        {
          if (ansi) td_put_string(ta_attr[*p - ULC]);
          else td_put_string(td_attr[*p - ULC]);
        }
        else putchar('?');
        p++;
      }
      fflush(stdout);
    }
    if (td_crt.td_width == 80)
    {
      if (ansi) 
      {
        fprintf(stdout, "%c[%d;%dH", 
          ESC, td_cur_row + 1, td_cur_col + 1);
      }
      else
      {
        putchar(ESC); 
        putchar('=');                     /* restore cursor position         */
        putchar(td_cur_row + SPACE); 
        putchar(td_cur_col + SPACE);
      }
      fflush(stdout);
    }
  }
/*
 *  Print Entire Screen
 */
  td_print_screen(w)
  td_window *w;
  {
    register long j, k;
    register unsigned char *q;
    td_window *x;
    unsigned char hof = 0x0c;
    unsigned char dot = '.';
    char fd_name[20];
    char command[80];
    long now;
    FILE *fd;

    x = w;

    while (x->td_pwin)                    /* find largest window             */
    {
      x = x->td_pwin;                     /* window above window             */
    }
#ifdef KW
    sprintf(fd_name, "tmp/BARD%d", time(0) & 0xffff);
#endif
#ifdef BARD
    sprintf(fd_name, "BARD%d", time(0) & 0xffff);
#endif
#ifdef TBB
    strcpy(fd_name, tmp_name());          /* get file name                   */
#endif
    fd = fopen(fd_name, "w");             /* open print file                 */

    if (fd == 0) return;                  /* bad file name                   */

    now = time(0);
   
#ifdef KW
    fprintf(fd, "\nPrint Screen  %24.24s\n", ctime(&now));
#endif
#ifdef BARD
    fprintf(fd, "\nPrint Screen  %24.24s\n", ctime(&now));
#endif
#ifdef TBB
    fprintf(fd, "\nPrint Screen For %3.3s\n", cuserid(0));
#endif

    fprintf(fd, "Last Message: %-46s\n\n", td_last_message + 2);
   
    for (k=0; k<x->td_width + 2; k++) fwrite(&dot, 1, 1, fd);
    fwrite("\n", 1, 1, fd);

    q = x->td_image;                      /* start of data                   */

    for (k=0; k<x->td_length; k++)        /* all rows                        */
    {
      fwrite(&dot, 1, 1, fd);

      for (j=0; j<x->td_width; j++)       /* all columns                     */
      {
        if (*q >= SPACE && *q < DELETE) fwrite(q, 1, 1, fd);
        else if (*q == HORT) fwrite("-", 1, 1, fd);
        else if (*q == VERT) fwrite("|", 1, 1, fd);
        else if (*q >= ULC && *q <= CROSS) fwrite("+", 1, 1, fd);
        else fwrite(" ", 1, 1, fd);
        q++;
      }
      fwrite(&dot, 1, 1, fd);
      fwrite("\n", 1, 1, fd);
    }
    for (k=0; k<x->td_width + 2; k++) fwrite(&dot, 1, 1, fd);
    fprintf(fd, "\n\n");

    fprintf(fd, " ");
    for (k = 0; k < 8; k++)
    {
      fprintf(fd, "|%8.8s|", &td_tab_key[12 * k + 3]);
    }
    fwrite("\n", 1, 1, fd);

    if (w->td_length <= w->td_size)
    {
      fwrite(&hof, 1, 1, fd);
      fclose(fd);

#ifdef KW

/* sprintf(command, "lpr -delete %s 1>/dev/null 2>/dev/null", fd_name); */

      sprintf(command, "%s %s", getenv("LPR"), fd_name);
      system(command);
#endif
#ifdef BARD
      sprintf(command, "lp -s -c %s 1>/dev/null 2>/dev/null", fd_name);
      system(command);
      unlink(fd_name);
#endif
#ifdef TBB
      queue_print(fd_name, "print screen", "1", "CPI=12,LENGTH=66", "plain",
      cuserid(0),"","","");
#endif
      return;
    }
    fwrite("\n\nPrint Form\n\n", 1, 14, fd);
    for (k=0; k<w->td_width + 2; k++) fwrite(&dot, 1, 1, fd);
    fwrite("\n", 1, 1, fd);

    q = w->td_image;                      /* start of data                   */

    for (k=0; k<w->td_length; k++)        /* all rows                        */
    {
      fwrite(&dot, 1, 1, fd);

      for (j=0; j<w->td_width; j++)       /* all columns                     */
      {
        if (*q >= SPACE && *q < DELETE) fwrite(q, 1, 1, fd);
        else if (*q == HORT) fwrite("-", 1, 1, fd);
        else if (*q == VERT) fwrite("|", 1, 1, fd);
        else if (*q >= ULC && *q <= CROSS) fwrite("+", 1, 1, fd);
        else fwrite(" ", 1, 1, fd);
        q++;
      }
      fwrite(&dot, 1, 1, fd);
      fwrite("\n", 1, 1, fd);
    }
    for (k=0; k<w->td_width + 2; k++) fwrite(&dot, 1, 1, fd);
    fwrite("\n", 1, 1, fd);
    fwrite(&hof, 1, 1, fd);
    fclose(fd);

#ifdef KW
/* sprintf(command, "lpr -delete %s 1>/dev/null 2>/dev/null", fd_name); */

    sprintf(command, "%s %s", getenv("LPR"), fd_name);
    system(command);
#endif
#ifdef BARD
    sprintf(command, "lp -s -c %s 1>/dev/null 2>/dev/null", fd_name);
    system(command);
    unlink(fd_name);
#endif
#ifdef TBB
    queue_print(fd_name, "print screen", "1", "CPI=12,LENGTH=66", "plain",
    cuserid(0),"","","");
#endif

    return;
  }

/* end of td.c */
