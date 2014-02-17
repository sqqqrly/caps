/*-------------------------------------------------------------------------*
 *  Noxell Pick Text _ DAH 08/23/91
 *-------------------------------------------------------------------------*/

typedef struct
{
  char sku_shelf_pk[5];                   /* SKU Shelf Packs                 */
  char sku_full_cases[5];                 /* SKU Full Cases                  */
  char sku_doz[5];                        /* SKU Dozens                      */
  char sku_units[2];                      /* SKU Units                       */
  char sku_weight[8];                     /* SKU Weight                      */
  char sku_class[2];                      /* SKU Class                       */
  char cust_sku_descr[20];                /* Customer specific description   */

} pick_text_item;

/* length of pick text is 47 bytes */

/* end of pick_text.h */
