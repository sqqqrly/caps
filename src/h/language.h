/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Language Header.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/17/97   |  tjt  Original Implementation.
 *-------------------------------------------------------------------------*/
static char language_h[] = "%Z% %M% %I% (%G% - %U%)";

unsigned char code_to_caps();
unsigned char caps_to_code();

/*-------------------------------------------------------------------------*
 *  Codes Below Are Defined For The Current Langauge
 *
 *  For example, cYes = 'y' for English.
 *               cYes = 's' for Spanish (si) or Portuguese (sim).
 *               cYes = 'j' for German (ja).
 *               cYes = 'o' for French (oui).
 *-------------------------------------------------------------------------*/

unsigned char cYes  = 'y';
unsigned char cNo   = 'n';
unsigned char cExit = 'e';
unsigned char cFwd  = 'f';
unsigned char cBwd  = 'b';

/* #include "english.h"                   not use for english !!!            */

/* end of language.h */
