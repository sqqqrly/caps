drop index i_orders;

drop table orders;

create table orders
(
  of_pl           SMALLINT ,
  of_on           INTEGER ,
  of_no_picks     SMALLINT       DEFAULT 0,
  of_no_units     SMALLINT       DEFAULT 0,
  of_datetime     INTEGER        DEFAULT 0,
  of_elapsed      SMALLINT       DEFAULT 0,
  of_picker       INTEGER        DEFAULT 0,
  of_pri          CHAR(1)        DEFAULT ' ',
  of_status       CHAR(1)        DEFAULT ' ',
  of_repick       CHAR(1)        DEFAULT ' ',
  of_grp          CHAR(6)        DEFAULT '      ',
  of_con          CHAR(15)       DEFAULT '  ',
  CONSTRAINT pk_plon PRIMARY KEY (of_pl,of_on) 
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index i_orders on orders (of_pl, of_con);

start $HOME/src/h/oracle/orders_pkg.sql;

exit;
