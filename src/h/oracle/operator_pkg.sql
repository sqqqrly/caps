
connect /

CREATE OR REPLACE PACKAGE operator_retrieve AS

   TYPE opr_cur_type IS REF CURSOR  RETURN operator%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT opr_cur_type, 
                       p_op_name   IN operator.op_name%TYPE);

END operator_retrieve;

/

CREATE OR REPLACE PACKAGE BODY operator_retrieve AS

   PROCEDURE  open_cur(curs IN OUT opr_cur_type, 
                       p_op_name   IN operator.op_name%TYPE) IS
   BEGIN
 
     open curs FOR 
     SELECT * FROM operator
     WHERE   op_name = p_op_name;

   END open_cur;


END operator_retrieve;
/
