#include <stdio.h>

typedef struct
{
	char *pat;
	char *val;
} pair;

pair tab[] = {{"*",         "stuff"},
              {"[DE]*",     "stuff"},
              {"[DE]*",     "Drop"},
              {"[DE]*",     "Each"},
              {"abc",       "abcd"},
              {"abc*",      "abcdef"},
              {"a?c",       "abc"},
              {"a?c*",      "abcde"},
              {"[!0-9]*",   "12345"},
              {"[!0-9]*",   "abcde"},
              {"[!a-zA-Z]", "1"},
              {"[!a-zA-Z]", "M"},
              {"[!a-zA-Z]", "b"},
              {0, 0}};
 

main()
{
  register long k;
  
  
  for (k = 0; tab[k].pat; k++)
  {
    printf("Blike(%s, %s) = %d\n", 
      tab[k].pat, tab[k].val, Blike(tab[k].pat, tab[k].val));
  
  
  }
  
}
  
  
