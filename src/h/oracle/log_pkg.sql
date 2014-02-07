

connect /

CREATE OR REPLACE PACKAGE log_retrieve AS

   TYPE log_cur_type IS REF CURSOR  RETURN maint_log%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT log_cur_type, 
                       p_lg_ref    IN maint_log.log_ref%TYPE);

END log_retrieve;

/

CREATE OR REPLACE PACKAGE BODY log_retrieve AS

   PROCEDURE  open_cur(curs IN OUT log_cur_type, 
                       p_lg_ref    IN maint_log.log_ref%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM maint_log
     WHERE   log_ref =  p_lg_ref;

   END open_cur;


END log_retrieve;
/
