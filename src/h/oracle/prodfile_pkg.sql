

connect /

CREATE OR REPLACE PACKAGE prodfile_retrieve AS

   TYPE prodfile_type IS REF CURSOR  RETURN prodfile%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT prodfile_type);

   PROCEDURE  open_cur1(curs      IN OUT prodfile_type,
                        p_pfsku IN  prodfile.pfsku%TYPE);

   PROCEDURE  open_cur2(curs      IN OUT prodfile_type,
                        p_fgroup  IN  prodfile.fgroup%TYPE);

END prodfile_retrieve;
/

CREATE OR REPLACE PACKAGE BODY prodfile_retrieve AS

/* All Records from Prodfile Table */

  PROCEDURE open_cur0(curs IN OUT prodfile_type) IS
  BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM prodfile;

  END open_cur0;

/* Rows based on pfsku */

   PROCEDURE  open_cur1(curs      IN OUT prodfile_type,
                        p_pfsku IN  prodfile.pfsku%TYPE) IS
   BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM prodfile
    WHERE pfsku    = p_pfsku;

   END open_cur1;

/* Row based on fgroup */

   PROCEDURE  open_cur2(curs      IN OUT prodfile_type,
                        p_fgroup  IN  prodfile.fgroup%TYPE) IS
   BEGIN

    OPEN     curs    FOR 
    SELECT   *       FROM prodfile
    WHERE fgroup   = p_fgroup;

   END open_cur2;
   

END prodfile_retrieve;

/
