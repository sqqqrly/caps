
drop index pie_key1;

drop table picker_error;

create table picker_error
(
  pie_id            INTEGER   NOT NULL,
  pie_desc          CHAR(15)  DEFAULT '  ',
  pie_num           INTEGER   DEFAULT 0,
  pie_sample        INTEGER   DEFAULT 0
) 
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index pie_key1 on picker_error (pie_id, pie_desc);

start $HOME/src/h/oracle/picker_error_pkg.sql;

exit;
