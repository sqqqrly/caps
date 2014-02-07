connect /

CREATE OR REPLACE PACKAGE box_retrieve AS

   TYPE box_cur_type IS REF CURSOR  RETURN boxes%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT box_cur_type);

   PROCEDURE  open_cur1(curs IN OUT box_cur_type,
                      p_box_pl IN   boxes.box_pl%TYPE,
                      p_box_on IN   boxes.box_on%TYPE,
                      p_box_status IN boxes.box_status%TYPE);

   PROCEDURE  open_cur2(curs IN OUT box_cur_type,
                       p_box_pl    IN   boxes.box_pl%TYPE,
                       p_box_on    IN   boxes.box_on%TYPE,
                       p_box_no_lo IN   boxes.box_number%TYPE,
                       p_box_no_hi IN   boxes.box_number%TYPE);

   PROCEDURE  open_cur3(curs     IN OUT box_cur_type,
                        p_box_no IN   boxes.box_number%TYPE);

END box_retrieve;
/
CREATE OR REPLACE PACKAGE BODY box_retrieve AS
  PROCEDURE open_cur0(curs IN OUT box_cur_type) IS

  BEGIN

    open curs FOR 
    SELECT * FROM boxes ;

  END open_cur0;

  PROCEDURE  open_cur1(curs IN OUT box_cur_type,
                       p_box_pl IN   boxes.box_pl%TYPE,
                       p_box_on IN   boxes.box_on%TYPE,
                       p_box_status IN boxes.box_status%TYPE
                    ) IS
  BEGIN

    open curs FOR 
    SELECT *   FROM  boxes 
    WHERE box_pl     = p_box_pl 
    AND   box_on     = p_box_on 
    AND   box_status = p_box_status;

  END open_cur1;

  PROCEDURE  open_cur2(curs        IN OUT box_cur_type,
                       p_box_pl    IN   boxes.box_pl%TYPE,
                       p_box_on    IN   boxes.box_on%TYPE,
                       p_box_no_lo IN   boxes.box_number%TYPE,
                       p_box_no_hi IN   boxes.box_number%TYPE) IS

  BEGIN

  open curs FOR 
    SELECT *   FROM  boxes 
    WHERE box_pl     = p_box_pl 
    AND   box_on     = p_box_on 
    AND   box_number BETWEEN p_box_no_lo AND p_box_no_hi;

  END open_cur2;

  PROCEDURE  open_cur3(curs      IN OUT box_cur_type,
                       p_box_no IN   boxes.box_number%TYPE) IS
  BEGIN

    open curs FOR 
    SELECT *   FROM  boxes 
    WHERE box_number = p_box_no;
  
  END open_cur3; 

END box_retrieve;  

/
