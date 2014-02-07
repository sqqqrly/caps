drop table prodfile;

create table prodfile
(
  pfsku    CHAR(15)  PRIMARY KEY,
  descr    CHAR(25),
  fgroup   CHAR(5),
  um       CHAR(3),
  ipqty    SMALLINT,       
  cpack    SMALLINT,
  bsloc    CHAR(6),
  absloc   CHAR(6),
  altid    CHAR(25)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index pf_key2 on prodfile (fgroup);

start $HOME/src/h/oracle/prodfile_pkg.sql;

exit;
