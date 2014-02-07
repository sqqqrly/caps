connect /

CREATE OR REPLACE PACKAGE queue_retrieve AS

   TYPE queue_cur_type IS REF CURSOR  RETURN queue%ROWTYPE;

   PROCEDURE  open_cur(curs IN OUT queue_cur_type);

END queue_retrieve;

/

CREATE OR REPLACE PACKAGE BODY queue_retrieve AS

  PROCEDURE  open_cur(curs IN OUT queue_cur_type) IS 
  BEGIN

    open curs FOR 
    SELECT * FROM queue;

  END open_cur;

END queue_retrieve;
/
