drop table restock_notice;

create table restock_notice
(
  rs_ref          INTEGER PRIMARY KEY,
  rs_time         INTEGER,
  rs_pl           SMALLINT,
  rs_mod          SMALLINT,
  rs_number       INTEGER,
  rs_quantity     SMALLINT
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start $HOME/src/h/oracle/restock_pkg.sql;

exit;
