drop table picker;

create table picker
(
  picker_id         INTEGER PRIMARY KEY,
  last_name         CHAR(16),
  first_name        CHAR(16),
  middle_initial    CHAR(2),  
  underway_orders   SMALLINT,
  start_time        INTEGER,
  current_time      INTEGER,
  cur_order_count   INTEGER,
  cur_lines         INTEGER,
  cur_units         INTEGER,
  cur_time          INTEGER,
  cum_order_count   INTEGER,
  cum_lines         INTEGER,
  cum_units         INTEGER,
  cum_time          INTEGER
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start $HOME/src/h/oracle/picker_pkg.sql;

exit;
