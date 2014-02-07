drop index i_ocitem;

drop table order_cust_item;

create table order_cust_item
(
  cust_item_nbr		NUMBER          PRIMARY KEY,
  cust_order_nbr	INTEGER 	DEFAULT 	0,
  sku_no		CHAR(6) 	DEFAULT 	'     ',
  pick_location		CHAR(7)		DEFAULT 	'      ',
  descr			CHAR(30)	DEFAULT 	'      ',
  ordered_qty		INTEGER 	DEFAULT 	0,
  ratio			INTEGER		DEFAULT 	0,
  assign_tote_id        INTEGER		DEFAULT 	0,
  work_code             CHAR(11)	DEFAULT 	'      ',
  merch_type		CHAR(1)		DEFAULT 	' ',
  picked_qty		INTEGER		DEFAULT 	0,
  actual_tote_id        INTEGER		DEFAULT 	0,
  picker_id             INTEGER		DEFAULT 	0,
  pick_datetime         DATE
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index i_ocitem on order_cust_item (cust_order_nbr);


start $HOME/src/h/oracle/order_custitem_pkg.sql;

start $HOME/src/h/oracle/ocustitem_seq.sql;

exit;
