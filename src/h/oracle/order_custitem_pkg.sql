connect /

CREATE OR REPLACE PACKAGE ordercusti_retrieve AS

   TYPE ord_cur_type IS REF CURSOR  RETURN order_cust_item%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT ord_cur_type);

   PROCEDURE  open_cur1(curs IN OUT ord_cur_type,
              p_cust_order_nbr IN   order_cust_item.cust_order_nbr%TYPE);

END ordercusti_retrieve;

/

CREATE OR REPLACE PACKAGE BODY ordercusti_retrieve AS

  PROCEDURE open_cur0(curs IN OUT ord_cur_type) IS
  BEGIN
    open curs FOR 
    SELECT * FROM order_cust_item;
  END open_cur0;

  PROCEDURE open_cur1(curs IN OUT ord_cur_type,
              p_cust_order_nbr IN   order_cust_item.cust_order_nbr%TYPE
                    ) IS
  BEGIN
    open curs FOR 
    SELECT * FROM order_cust_item
    WHERE  cust_order_nbr  = p_cust_order_nbr  ;
  END open_cur1;

END ordercusti_retrieve;
/
