drop index xt_key2;

drop table transaction;


create table transaction
(
  xt_ref          INTEGER PRIMARY KEY,
  xt_time         INTEGER,
  xt_group        CHAR(6),
  xt_con          CHAR(15),
  xt_on           CHAR(7),
  xt_pl           CHAR(2),
  xt_code         CHAR(1),
  xt_sku_mod1     CHAR(15),
  xt_stkloc       CHAR(6),
  xt_quan1        CHAR(4),
  xt_quan2        CHAR(4),
  xt_zone         CHAR(3),
  xt_lot          CHAR(15)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index xt_key2 on transaction (xt_pl, xt_on);

start $HOME/src/h/oracle/trans_pkg.sql;

start $HOME/src/h/oracle/trans_seq.sql;
exit;
