
connect /

CREATE OR REPLACE PACKAGE trans_retrieve AS

   TYPE trans_cur_type IS REF CURSOR  RETURN transaction%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT trans_cur_type);

   PROCEDURE  open_cur1(curs     IN OUT trans_cur_type,
                        p_xt_ref IN   transaction.xt_ref%TYPE);

   PROCEDURE  open_cur2(curs IN OUT trans_cur_type,
                       p_xt_pl IN   transaction.xt_pl%TYPE,
                       p_xt_on IN  transaction.xt_on%TYPE);
END trans_retrieve;

/

CREATE OR REPLACE PACKAGE BODY trans_retrieve AS

  PROCEDURE open_cur0(curs IN OUT trans_cur_type) IS
  BEGIN

    open curs FOR 
    SELECT * FROM transaction;

  END open_cur0;

  PROCEDURE  open_cur1(curs     IN OUT trans_cur_type,
                       p_xt_ref IN   transaction.xt_ref%TYPE
                      )IS
  BEGIN

    open curs FOR 
    SELECT * FROM transaction
    WHERE   xt_ref = p_xt_ref;

  END open_cur1;

  PROCEDURE open_cur2(curs IN OUT trans_cur_type,
                       p_xt_pl IN   transaction.xt_pl%TYPE,
                       p_xt_on IN   transaction.xt_on%TYPE
                    ) IS
  BEGIN

    open curs FOR 
    SELECT * FROM transaction
    WHERE xt_pl  = p_xt_pl
    AND   xt_on  = p_xt_on ;

  END open_cur2;

END trans_retrieve;
/
