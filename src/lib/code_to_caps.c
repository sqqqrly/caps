/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Translate input to y, n, e, f, b.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/17/97   |  Added to mfc.
 *-------------------------------------------------------------------------*/
static char code_to_caps_c[] = "%Z% %M% %I% (%G% - %U%)";

extern char cYes;
extern char cNo;
extern char cExit;
extern char cFwd;
extern char cBwd;

/*-------------------------------------------------------------------------*
 *  Translate Language Codes To Yes, No, Exit, Forward, or Backwards
 *-------------------------------------------------------------------------*/

unsigned char code_to_caps(x)
unsigned char x;
{
  unsigned char c;

  c = tolower(x);

  if (c == cYes)       return 'y';
  else if (c == cNo)   return 'n';
  else if (c == cExit) return 'e';
  else if (c == cFwd)  return 'f';
  else if (c == cBwd)  return 'b';
  else                 return x;
}

/*-------------------------------------------------------------------------*
 *  Translate 'y' and 'n' To Language Code cYes and cNo.
 *-------------------------------------------------------------------------*/

unsigned char caps_to_code(x)
unsigned char x;
{
  unsigned char c;
  
  c = tolower(x);
  
  if (c == 'y')       return cYes;
  else if (c == 'n')  return cNo;
  else                return x;
}

/* end of code_to_caps.c and caps_to_code.c */
