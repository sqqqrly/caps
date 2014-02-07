connect /

CREATE OR REPLACE PACKAGE ordercust_retrieve AS

   TYPE ord_cur_type IS REF CURSOR  RETURN order_cust%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT ord_cur_type);

   PROCEDURE  open_cur1(curs IN OUT ord_cur_type,
              p_cust_order_nbr IN   order_cust.cust_order_nbr%TYPE);

   PROCEDURE  open_cur2(curs IN OUT ord_cur_type,
                       p_order_status IN   order_cust.order_status%TYPE,
                       p_xmit_status IN  order_cust.xmit_status%TYPE);
END ordercust_retrieve;

/

CREATE OR REPLACE PACKAGE BODY ordercust_retrieve AS

  PROCEDURE open_cur0(curs IN OUT ord_cur_type) IS
  BEGIN
    open curs FOR 
    SELECT * FROM order_cust;
  END open_cur0;

  PROCEDURE open_cur1(curs IN OUT ord_cur_type,
              p_cust_order_nbr IN   order_cust.cust_order_nbr%TYPE
                    ) IS
  BEGIN
    open curs FOR 
    SELECT * FROM order_cust
    WHERE  cust_order_nbr  = p_cust_order_nbr  ;
  END open_cur1;

  PROCEDURE open_cur2(curs IN OUT ord_cur_type,
           p_order_status IN   order_cust.order_status%TYPE,
           p_xmit_status IN  order_cust.xmit_status%TYPE
                    ) IS
  BEGIN

    open curs FOR 
    SELECT * FROM order_cust
    WHERE order_status  = p_order_status
    AND   xmit_status  = p_xmit_status ;

  END open_cur2;

END ordercust_retrieve;
/
