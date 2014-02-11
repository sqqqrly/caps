/*
 *  queue.h
 *
 *  Record Structure For queue
 */

typedef struct
{
   long           q_queue_ref;
   long           q_queue_time;
   unsigned char  q_queue_text[64];

} queue_item;

#define queue_size 72

/* end of queue.h */
