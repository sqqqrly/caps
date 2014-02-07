drop table order_cust;

create table order_cust
(
  cust_order_nbr	NUMBER PRIMARY KEY,
  line_no		INTEGER        DEFAULT 0, 
  caps_order_no		INTEGER         DEFAULT 0,
  pickline_no		INTEGER        DEFAULT 0,
  store_no              CHAR(5)        DEFAULT '    ',
  dc_code               CHAR(3)        DEFAULT '   ',
  ord_constant          CHAR(4)        DEFAULT '    ',
  group_code		CHAR(3)        DEFAULT '   ',
  planned_pick_date	DATE,
  start_box             INTEGER        DEFAULT 0,
  end_box               INTEGER        DEFAULT 0,
  order_status          SMALLINT        DEFAULT 0,
  xmit_status           SMALLINT        DEFAULT 0
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start $HOME/src/h/oracle/order_cust_pkg.sql;

start $HOME/src/h/oracle/ocust_seq.sql;

exit;
