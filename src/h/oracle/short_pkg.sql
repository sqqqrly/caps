

connect /

CREATE OR REPLACE PACKAGE short_retrieve AS

   TYPE short_cur_type IS REF CURSOR  RETURN short_notice%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT short_cur_type, 
                       p_sh_ref    short_notice.sh_ref%TYPE); 

END short_retrieve;

/

CREATE OR REPLACE PACKAGE BODY short_retrieve AS

   PROCEDURE  open_cur(curs IN OUT short_cur_type, 
                       p_sh_ref    short_notice.sh_ref%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM short_notice
     WHERE   sh_ref = p_sh_ref;

   END open_cur;


END short_retrieve;
/
