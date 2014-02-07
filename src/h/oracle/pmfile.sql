
DROP TABLE pmfile;

CREATE TABLE pmfile
(
  pmodno       INTEGER  PRIMARY KEY,
  pmsku        CHAR(15)  DEFAULT ' ',
  qty          INTEGER   DEFAULT 0,
  alloc        INTEGER   DEFAULT 0,
  restock      INTEGER   DEFAULT 0,
  rqty         INTEGER   DEFAULT 0,
  lcap         INTEGER   DEFAULT 0,
  stkloc       CHAR(6)   DEFAULT ' ',
  display      CHAR(4)   DEFAULT ' ',
  plidx        SMALLINT  DEFAULT 0,
  piflag       CHAR(1)   DEFAULT ' ',
  cuunits      INTEGER   DEFAULT 0,
  cmunits      INTEGER   DEFAULT 0,
  culines      INTEGER   DEFAULT 0,
  cmlines      INTEGER   DEFAULT 0,
  curecpt      INTEGER   DEFAULT 0,
  cmrecpt      INTEGER   DEFAULT 0,
  rsflag       CHAR(1)   DEFAULT ' ',
  acflag       CHAR(1)   DEFAULT ' '
)
 STORAGE     (INITIAL       500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

CREATE INDEX        pm_key2 ON pmfile (pmsku);

CREATE INDEX        pm_key3 ON pmfile (stkloc);

start $HOME/src/h/oracle/pmfile_pkg.sql;

exit;
