connect /

CREATE OR REPLACE PACKAGE picks_retrieve AS

   TYPE pick_type IS REF CURSOR  RETURN picks%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT pick_type);

   PROCEDURE  open_cur1(curs        IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE);

   PROCEDURE  open_cur2(curs        IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE,
                       p_pk_lo     IN     picks.pi_box%TYPE,
                       p_pk_hi     IN     picks.pi_box%TYPE);

   PROCEDURE  open_cur3(curs      IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE,
                       p_pk_lo     IN     picks.pi_zone%TYPE,
                       p_pk_hi     IN     picks.pi_zone%TYPE);

   PROCEDURE  open_cur4(curs      IN OUT pick_type,
                       p_pi_sku    IN     picks.pi_sku%TYPE);

   PROCEDURE  open_cur5(curs      IN OUT pick_type,
                       p_pi_ref   IN     picks.pi_reference%TYPE);

   PROCEDURE  open_cur6(curs        IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE,
                       p_pi_module IN     picks.pi_module%TYPE);
END picks_retrieve;
/
CREATE OR REPLACE PACKAGE BODY picks_retrieve AS

/* All Records from Pick Table */

  PROCEDURE open_cur0(curs IN OUT pick_type) IS
  BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM picks;

  END open_cur0;

/* Rows based on the combnination of pickline,orderno and any moduleno */

  PROCEDURE open_cur1(curs IN OUT pick_type,
                       p_pi_pl IN     picks.pi_pl%TYPE,
                       p_pi_on IN     picks.pi_on%TYPE
                    ) IS
  BEGIN

    OPEN    curs      FOR 
    SELECT  *         FROM picks 
    WHERE   pi_pl     = p_pi_pl
    AND     pi_on     = p_pi_on;

  END open_cur1;

/* Row based on pickline,orderno and range of box no's */ 

   PROCEDURE  open_cur2(curs        IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE,
                       p_pk_lo     IN     picks.pi_box%TYPE,
                       p_pk_hi     IN     picks.pi_box%TYPE) IS

   BEGIN
    open   curs    FOR 
    SELECT *       FROM picks 
    WHERE  pi_pl     = p_pi_pl
    AND    pi_on     = p_pi_on 
    AND    pi_box BETWEEN  p_pk_lo AND p_pk_hi;

  END open_cur2;
   
/* Row based on pickline,orderno and range of Zone no's */ 

   PROCEDURE  open_cur3(curs        IN OUT pick_type,
                       p_pi_pl     IN     picks.pi_pl%TYPE,
                       p_pi_on     IN     picks.pi_on%TYPE,
                       p_pk_lo     IN     picks.pi_zone%TYPE,
                       p_pk_hi     IN     picks.pi_zone%TYPE) IS

   BEGIN

    open   curs    FOR 
    SELECT *       FROM picks 
    WHERE  pi_pl     = p_pi_pl
    AND    pi_on     = p_pi_on 
    AND    pi_zone BETWEEN  p_pk_lo AND p_pk_hi;

  END open_cur3;
   
/* Row based on SKU */ 

   PROCEDURE  open_cur4(curs        IN OUT pick_type,
                        p_pi_sku    IN     picks.pi_sku%TYPE) IS

   BEGIN

    open   curs    FOR 
    SELECT *       FROM picks 
    WHERE  pi_sku    = p_pi_sku;

   END open_cur4;

/* Row based on Reference No */

   PROCEDURE  open_cur5(curs        IN OUT pick_type,
                        p_pi_ref    IN     picks.pi_reference%TYPE) IS

   BEGIN

    open   curs    FOR 
    SELECT *       FROM picks 
    WHERE  pi_reference   = p_pi_ref;

   END open_cur5;

/* Rows based on the combnination of pickline,orderno and moduleno */

  PROCEDURE open_cur6(curs IN OUT pick_type,
                       p_pi_pl IN     picks.pi_pl%TYPE,
                       p_pi_on IN     picks.pi_on%TYPE,
                       p_pi_module IN picks.pi_module%TYPE
                    ) IS
  BEGIN

    OPEN    curs      FOR 
    SELECT  *         FROM picks 
    WHERE   pi_pl     = p_pi_pl
    AND     pi_on     = p_pi_on 
    AND     pi_module = p_pi_module;

  END open_cur6;

END picks_retrieve;

/
