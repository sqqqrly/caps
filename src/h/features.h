/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Definition of Features.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/21/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#ifndef FEATURES_H
#define FEATURES_H

static char features_h[] = "%Z% %M% %I% (%G% - %U%)";

typedef struct features_item
{
  unsigned char fword[8][32];             /* eight feature words             */

} Tfeatures_item;

extern Tfeatures_item fl;

#define MainMenuFeatures                fl.fword[0]
#define OperationsMenuFeatures          fl.fword[1]
#define SystemMenuFeatures              fl.fword[2]
#define ConfigurationEntryFeatures      fl.fword[3]
#define ProductivityFeatures            fl.fword[4]
#define ProductFileFeatures             fl.fword[6]
#define LabelFeatures                   fl.fword[7]

#endif

/* end of features.h */
