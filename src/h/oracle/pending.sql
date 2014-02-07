drop table pending;

create table pending
(
  CONSTRAINT pending_plon PRIMARY KEY (pnd_pl,pnd_on),
  pnd_pl          SMALLINT,
  pnd_on          INTEGER,
  pnd_group       CHAR(6)    DEFAULT ' ',
  pnd_con         CHAR(15)   DEFAULT ' ',
  pnd_flags       SMALLINT   DEFAULT 0
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index pnd_key2 on pending (pnd_pl, pnd_group);
create index pnd_key3 on pending (pnd_pl, pnd_con);

start $HOME/src/h/oracle/pending_pkg.sql;
exit;
