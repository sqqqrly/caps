drop table operator;

create table operator 
(
  op_name     CHAR(9)   PRIMARY KEY,
  op_desc     CHAR(33),
  op_printer  CHAR(9),
  op_pickline CHAR(3),
  op_level    CHAR(2),
  op_mm       CHAR(32),
  op_ops      CHAR(32),
  op_sys      CHAR(32),
  op_config   CHAR(32),
  op_prod     CHAR(32),
  op_sku      CHAR(32),
  op_label    CHAR(32)
)
 STORAGE     (INITIAL      500K 
              NEXT         500K  
              MINEXTENTS      1 
              MAXEXTENTS   1000 
              PCTINCREASE     5);

start operator_pkg;
exit;
