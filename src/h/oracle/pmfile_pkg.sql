connect /

CREATE OR REPLACE PACKAGE pmfile_retrieve AS

   TYPE pmfile_cur_type IS REF CURSOR  RETURN pmfile%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT pmfile_cur_type);

   PROCEDURE  open_cur1(curs    IN OUT pmfile_cur_type,
                       p_pmodno IN     pmfile.pmodno%TYPE);

   PROCEDURE  open_cur2(curs   IN OUT pmfile_cur_type,
                       p_pmsku IN   pmfile.pmsku%TYPE);

   PROCEDURE  open_cur3(curs   IN OUT pmfile_cur_type,
                       p_pmstkloc IN   pmfile.stkloc%TYPE);

END pmfile_retrieve;

/

CREATE OR REPLACE PACKAGE BODY pmfile_retrieve AS

  PROCEDURE open_cur0(curs IN OUT pmfile_cur_type) IS
  BEGIN

    open curs FOR 
    SELECT * FROM pmfile;

  END open_cur0;

  PROCEDURE  open_cur1(curs     IN OUT pmfile_cur_type,
                       p_pmodno IN     pmfile.pmodno%TYPE
                    ) IS
   BEGIN

    open curs FOR 
    SELECT * FROM pmfile
    WHERE pmodno  = p_pmodno;

   END open_cur1;

    PROCEDURE  open_cur2(curs    IN OUT pmfile_cur_type,
                        p_pmsku IN     pmfile.pmsku%TYPE
                       ) IS
   BEGIN

    open curs FOR 
    SELECT * FROM pmfile
    WHERE  pmsku   = p_pmsku;

   END open_cur2; 

   PROCEDURE  open_cur3(curs   IN OUT pmfile_cur_type,
                       p_pmstkloc IN   pmfile.stkloc%TYPE
                       ) IS
   BEGIN

         open curs FOR
         SELECT * FROM pmfile
         WHERE  stkloc  = p_pmstkloc;
   
   END open_cur3; 

END pmfile_retrieve;

/
