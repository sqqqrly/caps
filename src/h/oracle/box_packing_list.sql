drop table box_packing_list;

create table box_packing_list 
(
  bpl_ref          INTEGER PRIMARY KEY,
  bpl_time         INTEGER, 
  bpl_copies       SMALLINT,
  bpl_pl           SMALLINT,
  bpl_on           INTEGER,
  bpl_box          INTEGER,
  bpl_printer      CHAR(8)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start $HOME/src/h/oracle/box_list_pkg.sql;

start $HOME/src/h/oracle/box_list_seq.sql;

exit;
