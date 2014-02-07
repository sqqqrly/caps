connect /

CREATE OR REPLACE PACKAGE restock_retrieve AS

   TYPE restock_cur_type IS REF CURSOR  RETURN restock_notice%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT restock_cur_type,
                       p_rs_ref IN restock_notice.rs_ref%TYPE);

END restock_retrieve;

/

CREATE OR REPLACE PACKAGE BODY restock_retrieve AS

   PROCEDURE  open_cur(curs IN OUT restock_cur_type,
                       p_rs_ref IN restock_notice.rs_ref%TYPE) IS
   BEGIN

    open curs FOR 
    SELECT * FROM restock_notice
    WHERE  rs_ref= p_rs_ref;

   END open_cur;

END restock_retrieve;
/
