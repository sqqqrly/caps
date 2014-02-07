

connect /

CREATE OR REPLACE PACKAGE tote_label_retrieve AS

   TYPE tote_cur_type IS REF CURSOR  RETURN tote_label%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT tote_cur_type, 
                       p_tl_ref    IN tote_label.tl_ref%TYPE);

END tote_label_retrieve;

/

CREATE OR REPLACE PACKAGE BODY tote_label_retrieve AS

   PROCEDURE  open_cur(curs IN OUT tote_cur_type, 
                       p_tl_ref    IN tote_label.tl_ref%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM tote_label
     WHERE   tl_ref = p_tl_ref;

   END open_cur;


END tote_label_retrieve;
/
