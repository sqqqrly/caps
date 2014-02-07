
connect /

CREATE OR REPLACE PACKAGE lot_retrieve AS

   TYPE lot_cur_type IS REF CURSOR  RETURN lot%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT lot_cur_type, 
                       p_lot_pl    IN lot.lot_pl%TYPE, 
                       p_lot_sku   IN lot.lot_sku%TYPE); 

END lot_retrieve;

/

CREATE OR REPLACE PACKAGE BODY lot_retrieve AS

   PROCEDURE  open_cur(curs IN OUT lot_cur_type, 
                       p_lot_pl    IN lot.lot_pl%TYPE, 
                       p_lot_sku   IN lot.lot_sku%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM lot
     WHERE   lot_pl = p_lot_pl
     AND     lot_sku = p_lot_sku;

   END open_cur;


END lot_retrieve;
/
