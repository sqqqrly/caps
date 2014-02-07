
connect /

CREATE OR REPLACE PACKAGE picker_order_retrieve AS

   TYPE picker_type IS REF CURSOR  RETURN picker_order%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT picker_type);

   PROCEDURE  open_cur1(curs          IN OUT picker_type,
                   p_order_number IN  picker_order.order_number%TYPE,
                   p_pickline     IN  picker_order.pickline%TYPE,
                   p_comp_time    IN  picker_order.completion_time%TYPE);

   PROCEDURE  open_cur2(curs    IN OUT picker_type,
                   p_picker_id  IN  picker_order.picker_id%TYPE,
                   p_start_time IN  picker_order.start_time%TYPE);

END picker_order_retrieve;
/
CREATE OR REPLACE PACKAGE BODY picker_order_retrieve AS

/* All Records from Pick Table */

  PROCEDURE open_cur0(curs IN OUT picker_type) IS
  BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM picker_order;

  END open_cur0;

/* Rows based on the combination of pickline,orderno and complete time*/

   PROCEDURE  open_cur1(curs      IN OUT picker_type,
                   p_order_number IN  picker_order.order_number%TYPE,
                   p_pickline     IN  picker_order.pickline%TYPE,
                   p_comp_time    IN  picker_order.completion_time%TYPE) IS
   BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM picker_order
    WHERE order_number    = p_order_number
    AND   pickline        = p_pickline
    AND   completion_time = p_comp_time;

   END open_cur1;

/* Row based on picker_id  and start_time */

   PROCEDURE  open_cur2(curs    IN OUT picker_type,
                   p_picker_id  IN  picker_order.picker_id%TYPE,
                   p_start_time IN  picker_order.start_time%TYPE) IS
   BEGIN
    open   curs    FOR 
    SELECT *       FROM picker_order 
    WHERE  picker_id  = p_picker_id
    AND    start_time = p_start_time;

   END open_cur2;
   

END picker_order_retrieve;

/
