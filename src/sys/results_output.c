/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction output.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/16/97   |  rvj  Revised for Eckerd to allow user entry of a transaction
 *                     start and stop date   
 *-------------------------------------------------------------------------*/
static char transac_output_c[] = "%Z% %M% %I% (%G% - %U%)";

#define DEBUG 

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include "file_names.h"
#include "iodefs.h"
#include "ss.h"
#include "xt.h"
#include "eh_nos.h"
#include "of.h"


#ifdef DEBUG
FILE *DF;
#define DF_SIZE 4000000
#endif



FILE *fd;

main(argc,argv)
int argc;
char **argv;
{
  register long k;
  unsigned char t, ans[2], datetime_start[15], datetime_stop[15];
  unsigned char starttime[20], endtime[20];
  long start_systime, stop_systime,now;
  long pid, status;
  long end_hour, end_min;
  char command[120];

  struct tm *xx;

  putenv("_=results_output");
  chdir(getenv("HOME"));
  end_hour=atol(argv[1]);
  end_min=atol(argv[2]);
 
#ifdef DEBUG
  DF = fopen("debug/results_output", "w");

  fprintf(DF, "results_output started:  \n");
  fprintf(DF, "end_out = %d    end_min = %d \n",end_hour,end_min);
#endif
  
  now = time(0);

  xx  = localtime(&now);
  
      memset(starttime, 0, 20);        /* starttime response             */
      memset(endtime, 0, 20);          /* endtime response               */ 

 	sprintf(starttime,"%02d/%02d/%04d 06:00:00",xx->tm_mon + 1,
		xx->tm_mday,xx->tm_year + 1900);
#ifdef DEBUG
  fprintf(DF, "starttime = %s\n",starttime);
#endif

 	sprintf(endtime,"%02d/%02d/%04d %02d:%02d:00",xx->tm_mon + 1,
		xx->tm_mday,xx->tm_year + 1900,end_hour, end_min);

#ifdef DEBUG
  fprintf(DF, "endtime = %s\n",endtime);
#endif


          if(!time_convert(starttime, &start_systime))
          {

 // fprintf(DF, "start_systime = %d\n",start_systime);
          }
        sprintf(datetime_start, "%d", start_systime);

#ifdef DEBUG
  fprintf(DF, "datetime_start = %s\n",datetime_start);
#endif
          if(!time_convert(endtime, &stop_systime))
           {
//  fprintf(DF, "stop_systime = %d\n",stop_systime);
           }
        sprintf(datetime_stop, "%d", stop_systime);
#ifdef DEBUG
  fprintf(DF, "datetime_stop = %s\n",datetime_stop);
#endif

	sprintf(command,"/u/mfc/bin/trans_out_new %s %s ",datetime_start,
		datetime_stop);
	system(command);
#ifdef DEBUG
	fprintf(DF,"%s\n",command);
	fclose(DF);
#endif
  

}   /* Main */
