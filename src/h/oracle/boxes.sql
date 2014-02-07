drop index box_key1;

drop index box_key3;

drop table boxes;

create table boxes
(
  CONSTRAINT pk_boxes PRIMARY KEY(box_pl,box_on,box_number),
  box_pl          SMALLINT NOT NULL,
  box_on          INTEGER  NOT NULL,
  box_number      INTEGER  NOT NULL,
--  status: '0'=unused, '1'=open, '2'=closed, '3'=queued, '4'=printed 
  box_status      CHAR(1) DEFAULT ' ',
  box_last        CHAR(1) DEFAULT ' ',
  box_lines       SMALLINT DEFAULT 0,
  box_units       SMALLINT DEFAULT 0
)
  STORAGE     (INITIAL      500K 
               NEXT         500K  
               MINEXTENTS      1 
               MAXEXTENTS   1000 
               PCTINCREASE     5);

create index box_key1 on boxes (box_pl, box_on, box_status);

--create index box_key2 on boxes (box_pl, box_on, box_number);

create index box_key3 on boxes (box_number);

start $HOME/src/h/oracle/boxes_pkg.sql;

exit;
