drop table ship_label;

create table ship_label
(
  sl_ref          INTEGER,
  sl_time         INTEGER,
  sl_copies       SMALLINT,
  sl_pl           SMALLINT,
  sl_on           INTEGER,
  sl_zone         SMALLINT
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);
 
create index sl_key1 on ship_label (sl_ref);
 
start $HOME/src/h/oracle/ship_label_pkg.sql;

start $HOME/src/h/oracle/ship_label_seq.sql;
exit;
