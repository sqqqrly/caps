/*
 *  K-Mart Remarks Data Area
 */
#define BOX 6
#define BNUM 99

typedef struct
{
  char r_schedule_no[6];
  char r_load_no[4];
  char r_batch_no[3];
  char r_box_number[BNUM][BOX];

} remarks_text_item;

/* length of remarks text is 607 bytes */

/* end of remarks.h */
