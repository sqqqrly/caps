connect /

CREATE OR REPLACE PACKAGE remarks_retrieve AS

   TYPE rem_cur_type IS REF CURSOR  RETURN remarks%ROWTYPE;


   PROCEDURE  open_cur1(curs IN OUT rem_cur_type,
                      p_rmks_pl IN   remarks.rmks_pl%TYPE,
                      p_rmks_on IN   remarks.rmks_on%TYPE);


END remarks_retrieve;
/
CREATE OR REPLACE PACKAGE BODY remarks_retrieve AS

  PROCEDURE  open_cur1(curs IN OUT rem_cur_type,
                       p_rmks_pl IN   remarks.rmks_pl%TYPE,
                       p_rmks_on IN   remarks.rmks_on%TYPE
                    ) IS
  BEGIN

    open curs FOR 
    SELECT *   FROM  remarks 
    WHERE rmks_pl     = p_rmks_pl 
    AND   rmks_on     = p_rmks_on;

  END open_cur1;

END remarks_retrieve;  

/
