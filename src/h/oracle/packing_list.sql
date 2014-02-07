drop table packing_list;

create table packing_list
(
  pl_ref          INTEGER,
  pl_time         INTEGER,
  pl_copies       SMALLINT,
  pl_pl           SMALLINT,
  pl_on           INTEGER,
  pl_zone         SMALLINT
) STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index pl_key1 on packing_list (pl_ref);

start $HOME/src/h/oracle/packing_list_pkg.sql;

start $HOME/src/h/oracle/packing_list_seq.sql;
exit;
