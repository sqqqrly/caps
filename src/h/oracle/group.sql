drop table groups;

create table groups
(
  g_group            CHAR(6) ,
  g_goodcarton       INTEGER ,
  g_badcarton        INTEGER ,
  g_status           SMALLINT
)
  STORAGE     (INITIAL      100K 
               NEXT         100K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index g_key1 on groups(g_group);

start  $HOME/src/h/oracle/groups_pkg.sql;
exit;
