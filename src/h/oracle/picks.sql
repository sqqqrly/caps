drop index pi_key1;
drop index pi_key2;
drop index pi_key3; 
drop index pi_key4;

drop table picks;

create table picks
(
  pi_reference    NUMBER PRIMARY KEY,
  pi_pl           SMALLINT      DEFAULT 0,
  pi_on           INTEGER       DEFAULT 0,
  pi_module       SMALLINT      DEFAULT 0,
  pi_zone         SMALLINT      DEFAULT 0,
  pi_ordered      SMALLINT      DEFAULT 0,
  pi_picked       SMALLINT      DEFAULT 0,
  pi_flags        SMALLINT      DEFAULT 0,
  pi_datetime     INTEGER       DEFAULT 0,
  pi_box          INTEGER       DEFAULT 0,
  pi_sku          CHAR(15)      DEFAULT '  ',
  pi_ptext        CHAR(32)      DEFAULT '  ',
  pi_lot          CHAR(15)      DEFAULT '  '
) 
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index pi_key1 on picks (pi_pl, pi_on, pi_module);
create index pi_key2 on picks (pi_pl, pi_on, pi_box);
create index pi_key3 on picks (pi_pl, pi_on, pi_zone);
create index pi_key4 on picks (pi_sku);

start $HOME/src/h/oracle/picks_pkg.sql;

start $HOME/src/h/oracle/picks_seq.sql;

exit;
