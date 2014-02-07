drop table remarks;

create table remarks
(
  rmks_pl        SMALLINT DEFAULT 0,
  rmks_on        INTEGER  DEFAULT 0,  CONSTRAINT pk_remarks 
                           PRIMARY KEY (rmks_pl, rmks_on),
  rmks_text      CHAR(64)  DEFAULT ' '
) 
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start $HOME/src/h/oracle/remarks_pkg.sql;
exit;
