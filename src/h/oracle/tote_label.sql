drop table tote_label;

create table tote_label
(
  tl_ref          INTEGER,
  tl_time         INTEGER,
  tl_copies       SMALLINT,
  tl_pl           SMALLINT,
  tl_on           INTEGER,
  tl_zone         SMALLINT
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS     1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index tl_key1 on tote_label (tl_ref);

start $HOME/src/h/oracle/tote_label_pkg.sql;

start $HOME/src/h/oracle/tote_label_seq.sql;
exit;
