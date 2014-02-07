connect /

CREATE OR REPLACE PACKAGE order_retrieve AS

   TYPE ord_cur_type IS REF CURSOR  RETURN orders%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT ord_cur_type);

   PROCEDURE  open_cur1(curs IN OUT ord_cur_type,
                       p_of_pl IN   orders.of_pl%TYPE,
                       p_of_on IN   orders.of_on%TYPE);

   PROCEDURE  open_cur2(curs IN OUT ord_cur_type,
                       p_of_pl IN   orders.of_pl%TYPE,
                       p_of_con IN  orders.of_con%TYPE);
END order_retrieve;

/

CREATE OR REPLACE PACKAGE BODY order_retrieve AS

  PROCEDURE open_cur0(curs IN OUT ord_cur_type) IS
  BEGIN
    open curs FOR 
    SELECT * FROM orders;
  END open_cur0;

  PROCEDURE open_cur1(curs IN OUT ord_cur_type,
                       p_of_pl IN   orders.of_pl%TYPE,
                       p_of_on IN  orders.of_on%TYPE
                    ) IS
  BEGIN
    open curs FOR 
    SELECT * FROM orders
    WHERE of_pl  = p_of_pl
    AND   of_on = p_of_on ;
  END open_cur1;

  PROCEDURE open_cur2(curs IN OUT ord_cur_type,
                       p_of_pl IN   orders.of_pl%TYPE,
                       p_of_con IN  orders.of_con%TYPE
                    ) IS
  BEGIN

    open curs FOR 
    SELECT * FROM orders
    WHERE of_pl  = p_of_pl
    AND   of_con  = p_of_con ;

  END open_cur2;

END order_retrieve;
/
