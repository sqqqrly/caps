drop table short_notice;

create table short_notice
(
  sh_ref          INTEGER,
  sh_time         INTEGER,
  sh_pl           SMALLINT,
  sh_on           INTEGER,
  sh_mod          SMALLINT,
  sh_ordered      SMALLINT,
  sh_picked       SMALLINT,
  sh_remaining    SMALLINT,
  sh_picker       INTEGER,
  sh_split        CHAR(1),
  sh_con          CHAR(15),
  sh_grp          CHAR(6)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index sh_key1 on short_notice (sh_ref);

start $HOME/src/h/oracle/short_pkg.sql;

start $HOME/src/h/oracle/short_seq.sql;

exit;
