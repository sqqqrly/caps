
connect /

CREATE OR REPLACE PACKAGE group_retrieve AS

   TYPE grp_cur_type IS REF CURSOR  RETURN groups%ROWTYPE;

   PROCEDURE  open_cur0(curs IN OUT grp_cur_type);

   PROCEDURE  open_cur1(curs IN OUT grp_cur_type,
                       p_group IN   groups.g_group%TYPE);

END group_retrieve;

/

CREATE OR REPLACE PACKAGE BODY group_retrieve AS

  PROCEDURE open_cur0(curs IN OUT grp_cur_type) IS
  BEGIN
    open curs FOR 
    SELECT * FROM groups;
  END open_cur0;

  PROCEDURE open_cur1(curs IN OUT grp_cur_type,
                       p_group IN   groups.g_group%TYPE
                    ) IS
  BEGIN
    open curs FOR 
    SELECT * FROM groups
    WHERE g_group = p_group;
  END open_cur1;

END group_retrieve;
/
