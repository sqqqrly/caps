/*
 *  maint_log.h
 *
 *  Record Structure For maint_log
 */

typedef struct
{
   long           m_log_ref;
   long           m_log_time;
   unsigned char  m_log_text[64];

} maint_log_item;

#define maint_log_size 72

/* end of maint_log.h */
