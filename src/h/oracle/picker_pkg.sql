
connect /

CREATE OR REPLACE PACKAGE picker_retrieve AS

   TYPE picker_cur_type IS REF CURSOR  RETURN picker%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT picker_cur_type); 

   PROCEDURE  open_cur1(curs       IN OUT picker_cur_type, 
                       p_picker_id IN picker.picker_id%TYPE); 

END picker_retrieve;

/

CREATE OR REPLACE PACKAGE BODY picker_retrieve AS

   PROCEDURE  open_cur0(curs IN OUT picker_cur_type) IS 
   BEGIN
 
     open curs FOR 
     SELECT * FROM picker;

   END open_cur0;

   PROCEDURE  open_cur1(curs       IN OUT picker_cur_type, 
                       p_picker_id IN picker.picker_id%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM picker
     where picker_id = p_picker_id;

   END open_cur1;

END picker_retrieve;
/
