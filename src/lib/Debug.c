#define DEBUG


#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdio.h>


Debug( Format, p1, p2, p3, p4 )
char	*Format, *p1, *p2, *p3, *p4;
{
	char	Message[80+1];
	struct timeb	Time;

	ftime( &Time );
	sprintf( Message, Format, p1, p2, p3, p4 );
	fprintf( stderr, "%.10d.%.3d: %s\n", Time.time, Time.millitm, Message );
}
