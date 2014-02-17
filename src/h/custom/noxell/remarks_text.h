/*-------------------------------------------------------------------------*
 *  Noxell Remarks Data Area - DAH 08/23/91
 *-------------------------------------------------------------------------*/
typedef struct
{
  char        address1[20];               /* Address line 1                  */
  char        address2[20];               /* Address line 2                  */
  char        address3[20];               /* Address line 3                  */
  char        address4[20];               /* Address line 4                  */
  char        zipcode[9];                 /* Full zip code                   */
  char        cust_number[9];             /* Customer Number                 */
  char        full_case[5];               /* Full Case Count                 */
  char        tote_labels[5];             /* Tote labels to print            */
  char        repack_cnt_sz1;             /* Repack Carton Size 1            */
  char        repack_cnt_cnt1[3];         /* Repack Carton Count 1           */
  char        repack_cnt_sz2;             /* Repack Carton Size 2            */
  char        repack_cnt_cnt2[3];         /* Repack Carton Count 2           */
  char        ship_date[8];               /* Ship Date                       */
  char        cust_po_number[18];         /* Customer order Number           */
  char        carrier_name[20];           /* Carrier name                    */
  char        tracer_number[9];           /* Tracer_Number                   */
  char        warehouse_desc[20];         /* Wharehouse Description          */
  char        freight_data[12][17];       /* Freight Data                    */
                                          /* Freight Cases  (0-4)            */
                                          /* Freight Class  (5-6)            */
                                          /* Freight Rate   (7-11)           */
                                          /* Freight Weight (12-16)          */
  char        cases[6];                   /* Total Cases Shipped             */
  char        weight[7];                  /* Total Weigth Shipped            */
  char        cube[8];                    /* Total Cube Shipped              */
  char        is_ups;                     /* 1 = print offline text 0 = don't*/
  char        print_cus_descr;            /* 1 = print cus descr    0 = don't*/
  char        offline_case[26][53];       /* Offline Text For Full Cases     */
                                          /* sku_shelf_pk (0-4)              */
                                          /* sku_number   (5-10)             */
                                          /* Full Cases   (11-15)            */
                                          /* Dozens       (16_20)            */
                                          /* Units        (21_22)            */
                                          /* Weight       (23_30)            */
                                          /* Class        (31_32)            */
                                          /* Description  (33_52)            */

} remarks_text_item;

/* length of remarks text is 1796 bytes */

/* end of remarks_text.h */
