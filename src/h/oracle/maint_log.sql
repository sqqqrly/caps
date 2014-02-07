drop table maint_log;

create table maint_log
(
   log_ref       INTEGER  PRIMARY KEY,
   log_time      INTEGER,
   log_text      CHAR(64)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);
start log_pkg;
start log_seq;

exit;
