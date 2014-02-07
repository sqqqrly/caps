select object_name,object_type,status
from all_objects
where owner='OPS$ADMIN' and object_type like 'PA%'
/
