drop table picker_order;

create table picker_order
(
  order_number      INTEGER,
  picker_id         INTEGER,
  pickline          SMALLINT,
  order_status      SMALLINT,
  start_time        INTEGER,
  completion_time   INTEGER,
  picking_time      INTEGER,
  lines_picked      INTEGER,
  units_picked      INTEGER
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index po_key1 on picker_order (order_number, pickline, completion_time);
create index po_key2 on picker_order (picker_id, start_time);

start $HOME/src/h/oracle/picker_order_pkg.sql;

exit;
