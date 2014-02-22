/*
 *  Tote Labels
 */
int	boxid;
char	STX = 0x02;
char	ETX = 0x03;

before_request()	/* called once per order */
{
    copies = 1;

/*  copies = cvrt(rmks->tote_labels,5); copies used by tote_label.c */
    boxid = copies;			 /* print last tote_label first */
}
before_each_page()	/* called for each copy */
{
    aplace(rmks->address1,      20, 1,   1);
    aplace(rmks->address2,      20, 1,  21);
    aplace(rmks->address3,      20, 1,  41);
    aplace(rmks->address4,      20, 1,  61);
    aplace(rmks->zipcode,        9, 1,  81);
    aplace(udate,                8, 1,  90);
    aplace(rmks->carrier_name,  20, 1,  98);
    aplace(of_rec->of_con,    8, 1, 118);	
    aplace(rmks->tracer_number,  9, 1, 126);
    aplace(&rmks->repack_cnt_sz1, 1, 1, 135);
    aplace(rmks->repack_cnt_cnt1,3, 1, 136);
    aplace(&rmks->repack_cnt_sz2, 1, 1, 139);
    aplace(rmks->repack_cnt_cnt2,3, 1, 140);
    nplace(of_rec->of_on, "99999", 1, 143);
    nplace(boxid, "999",            1, 148);	
    boxid--;
}

/* end of tote_label.c */
