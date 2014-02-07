/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find All Embedded Quoted Strings.
 *
 *  Execution       find_quote
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/12/97   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char find_quote_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

#define MAX  10000

char *memchr();

FILE *id, *od, *hd;
char id_name[64];
char od_name[64];
char hd_name[64] = "new_src/lang_work.h";

char new_name[32];

char names[MAX][32];
char text[MAX][80];
char len[MAX];
long max;

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=find_quote");
  
  load_table();
  
  process_files();
  
}
/*-------------------------------------------------------------------------*
 *  Process One File
 *-------------------------------------------------------------------------*/
process_one_file()
{
  register char start, *p, *q, *r;
  char buf[256], ans[8];
  register long ret, n, iflag;
  
  iflag = 0;
  
  while (fgets(buf, 255, id))
  {
    n = strlen(buf) - 1;
    buf[n] = 0;
    r = buf;
    
    puts(buf);
    
    if (memcmp(buf, "#include", 8) == 0) 
    {
      if (!iflag) iflag = 1;
      fprintf(od, "%s\n", buf);
      
      if (memcmp(buf, "#include \"language.h\"", 21) == 0) iflag = 2;
      continue;
    }
    if (iflag == 1)
    {
      iflag = 2;
      fprintf(od, "#include \"language.h\"\n");
    }
    if (iflag != 2)
    {
      fprintf(od, "%s\n", buf);
      continue;
    }
    start = 0;
    
    while (*r)
    {
      if (!start && *r != 0x22) 
      {
        fprintf(od, "%c", *r);
        r++;
        continue;
      }
      start = *r;
      p = q = r + 1;
        
      while (*q)  
      {
        if (*q == '\\') {q += 2; continue;}
        if (*q == start) break;
        q++;
      }
      if (!*q) {fprintf(od, "%s", r); break;}

      start = 0;
      *q = 0;
      r = q + 1;

      ret = make_name(p);
  
      if (ret) 
      {
        fprintf(od, "%s", new_name);
        
        printf("Dup %s\n", new_name);
        gets(ans);     
        continue;
      }
      printf("Make %s ? (y/n)\n", new_name);
      gets(ans);  

      if (ans[0] == 'y')
      {
        fprintf(od, "%s", new_name);
        
        strcpy(names[max], new_name);
        strcpy(text[max], p);
        len[max] = strlen(text[max]);
        
        n = strlen(new_name) + len[max];
        if (n > 68)
        {
          fprintf(hd, "#define %s \\\n \"%s\"\n", new_name, p);
        }
        else
        {
          fprintf(hd, "#define %s \"%s\"\n", new_name, p);
        }
        fflush(hd);
        
        max++;
        continue;
      }
      else fprintf(od, "\"%s\"", p);
    }
    fprintf(od, "\n");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make Name From Text
 *-------------------------------------------------------------------------*/
make_name(p)
register char *p;
{
  register long k, n;
  register char *q;
  
  n = strlen(p);
  
  for (k = 0; k < max; k++)
  {
    if (len[k] != n) continue;
    if (memcmp(p, text[k], n) == 0)
    {
      strcpy(new_name, names[k]);
      return 1;
    }
  }
  strcpy(new_name, "psz");
  q = new_name + 3;
  k = 3;
  
  while (*p && k < 16)
  {
    if (*p >= '0' && *p <= '9')      {*q++ = *p; k++;}
    else if (*p >= 'a' && *p <= 'z') {*q++ = *p; k++;}
    else if (*p >= 'A' && *p <= 'Z') {*q++ = *p; k++;}
  
    p++;
  }
  *q = 0;
  
  for (k = 0; k < max; k++)
  {
    if (strcmp(new_name, names[k]) == 0) break;
  }
  if (k >= max) return 0;
  
  *q = '0';
  *(q + 1) = 0;
  
  while (1)
  {
    for (k = 0; k < max; k++)
    {
      if (strcmp(new_name, names[k]) == 0)
      {
        if (*q == '9') *q = 'a';
        else           *q += 1;
        break;
      }
    }
    if (k >= max) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process All Files
 *-------------------------------------------------------------------------*/
process_files()
{
  FILE *fd;
  long n;
  
  fd = fopen("LIST", "r");
  if (fd == 0)
  {
    printf("Can't Open LIST File\n\n");
    exit(1);
  }
  hd = fopen(hd_name, "w");
  if (hd == 0)
  {
    printf("Can't Open %s\n\n", hd_name);
    exit(1);
  }
  while (fgets(id_name, 60, fd)) 
  {
    n = strlen(id_name) - 1;
    
    id_name[n] = 0;
    
    id = fopen(id_name, "r");
    if (id == 0)
    {
      printf("Can't Open %s\n\n", id_name);
      exit(1);
    }
    printf("Processing %s\n", id_name);
    
    fprintf(hd, "/* -- %s -- */\n", id_name);
    
    sprintf(od_name, "new_src/%s", id_name);
    
    od = fopen(od_name, "w");
    if (od == 0)
    {
      printf("Can't Open %s\n\n", od_name);
      exit(1);
    }
    process_one_file();
    
    fclose(id);
    fclose(od);
  }
  fclose(fd);
  fclose(hd);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Load Current Table  = #define pszNAME "text"\n
 *-------------------------------------------------------------------------*/
load_table()
{
  FILE *td;
  char buf[256], *p, *q;
  long n;
  
  max = 0;
  
  td = fopen("/u/mfc/src/h/english.h", "r");
  if (td == 0) return 0;
  
  while (fgets(buf, 255, td))
  {
    if (memcmp(buf, "#define ", 8) != 0) continue;
    
    n = strlen(buf);
    
    if (buf[n - 2] == '\\') fgets(buf + n - 2, 255 - n, td);
    
    p = memchr(buf, '"', strlen(buf));
    if (!p) continue;
    
    q = memchr(p + 1, '"', strlen(p + 1));
    if (!q) continue;
  
    *(p - 1) = 0;
    p++;
    *q = 0;
  
    fprintf(stderr, "Loading: %s  \"%s\"\n", buf + 8, p);
  
    strcpy(names[max], buf + 8);
    strcpy(text[max], p);
    len[max] = strlen(p);
    
    fprintf(stderr, "Table:  %s %d [%s]\n", names[max], len[max], text[max]);
    
    max++;
  }
  fclose(td);
}
 
/* end of find_quote.c */
