
connect /

CREATE OR REPLACE PACKAGE box_list_retrieve AS

   TYPE boxlist_cur_type IS REF CURSOR  RETURN box_packing_list%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT boxlist_cur_type, 
                       p_bpl_ref box_packing_list.bpl_ref%TYPE);

END box_list_retrieve;

/

CREATE OR REPLACE PACKAGE BODY box_list_retrieve AS

  PROCEDURE open_cur(curs IN OUT boxlist_cur_type, 
                     p_bpl_ref   box_packing_list.bpl_ref%TYPE) IS
  BEGIN
 
    open curs FOR 
    SELECT * FROM box_packing_list
    WHERE  bpl_ref = p_bpl_ref;

  END open_cur;


END box_list_retrieve;
/
