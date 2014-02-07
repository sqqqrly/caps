drop table queue;

create table queue
(
  queue_ref     INTEGER    PRIMARY KEY,
  queue_time    INTEGER    DEFAULT 0,
  queue_text    CHAR(255)   DEFAULT ' '
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

start queue_pkg;

start queue_seq;

exit;
