drop table lot;

create table lot
(
  lot_time        INTEGER,
  lot_pl          SMALLINT,
  lot_sku         CHAR(16),
  lot_number      CHAR(16)
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index lot_key1 on lot(lot_pl, lot_sku, lot_time DESC);

start lot_pkg; 

exit;
