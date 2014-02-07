
connect /

CREATE OR REPLACE PACKAGE packing_list_retrieve AS

   TYPE pklist_cur_type IS REF CURSOR  RETURN packing_list%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT pklist_cur_type,
                       p_pl_ref    IN packing_list.pl_ref%TYPE); 

END packing_list_retrieve;

/

CREATE OR REPLACE PACKAGE BODY packing_list_retrieve AS

   PROCEDURE  open_cur(curs IN OUT pklist_cur_type,
                       p_pl_ref    IN packing_list.pl_ref%TYPE) IS

   BEGIN
 
     open curs FOR 
     SELECT * FROM packing_list
     WHERE   pl_ref  = p_pl_ref;

   END open_cur;


END packing_list_retrieve;
/
