
connect /

CREATE OR REPLACE PACKAGE shiplabel_retrieve AS

   TYPE shiplabel_curtype IS REF CURSOR  RETURN ship_label%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT shiplabel_curtype, 
                       p_sl_ref    IN ship_label.sl_ref%TYPE); 

END shiplabel_retrieve;

/

CREATE OR REPLACE PACKAGE BODY shiplabel_retrieve AS

   PROCEDURE  open_cur(curs IN OUT shiplabel_curtype, 
                       p_sl_ref    IN ship_label.sl_ref%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM ship_label
     WHERE   sl_ref = p_sl_ref;

   END open_cur;


END shiplabel_retrieve;
/
