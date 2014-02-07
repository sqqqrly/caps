connect /

CREATE OR REPLACE PACKAGE pending_retrieve AS

   TYPE pending_type IS REF CURSOR  RETURN pending%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT pending_type);

   PROCEDURE  open_cur1(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_lo     IN     pending.pnd_on%TYPE,
                       p_pnd_hi     IN     pending.pnd_on%TYPE);

   PROCEDURE  open_cur2(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_lo     IN     pending.pnd_group%TYPE,
                       p_pnd_hi     IN     pending.pnd_group%TYPE);

   PROCEDURE  open_cur3(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_con    IN     pending.pnd_con%TYPE);
END pending_retrieve;
/
CREATE OR REPLACE PACKAGE BODY pending_retrieve AS

/* All Records from pending Table */

  PROCEDURE open_cur0(curs IN OUT pending_type) IS
  BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM pending;

  END open_cur0;

/* Rows based on the combination of pickline,orderno */

   PROCEDURE  open_cur1(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_lo     IN     pending.pnd_on%TYPE,
                       p_pnd_hi     IN     pending.pnd_on%TYPE
                    ) IS
   BEGIN

     OPEN     curs    FOR 
     SELECT   *       FROM pending
     WHERE   pnd_pl = p_pnd_pl
     AND     pnd_on BETWEEN p_pnd_lo AND p_pnd_hi;

  END open_cur1;

/* Row based on pickline and group */ 

   PROCEDURE  open_cur2(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_lo     IN     pending.pnd_group%TYPE,
                       p_pnd_hi     IN     pending.pnd_group%TYPE
                    ) IS
   BEGIN

     OPEN     curs    FOR 
     SELECT   *       FROM pending
     WHERE   pnd_pl = p_pnd_pl
     AND     pnd_group BETWEEN p_pnd_lo AND p_pnd_hi;

  END open_cur2;
   
/* Row based on pickline, and customer orderno */ 

   PROCEDURE  open_cur3(curs        IN OUT pending_type,
                       p_pnd_pl     IN     pending.pnd_pl%TYPE,
                       p_pnd_con    IN     pending.pnd_con%TYPE
                       ) IS
   BEGIN

     OPEN     curs    FOR 
     SELECT   *       FROM pending
     WHERE   pnd_pl = p_pnd_pl
     AND     pnd_con= p_pnd_con;

  END open_cur3;

END pending_retrieve;

/
