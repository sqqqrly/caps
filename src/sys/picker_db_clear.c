/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear picker databases of active orders.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char picker_db_clear_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "bard/picker.h"
#include "bard/picker_order.h"

long errors = 0;

main( argc, argv )
long	argc;
char*	argv[];
{
	picker_item			picker;
	picker_order_item	picker_order;

   putenv("_=picker_db_clear");
   chdir(getenv("HOME"));
   
   database_open();
   
	picker_open( AUTOLOCK );
	begin_work();
	while( picker_next( &picker, LOCK ) == 0 )
	{
		picker.p_underway_orders = 0;
		picker.p_start_time	 = 0;
		picker.p_current_time	 = 0;
                picker.p_zone            = 0;
                picker.p_status          = 0;

		if (picker_update( &picker ))
		{
			printf("Error During Rewrite Of Picker %d\n", picker.p_picker_id);
			errors++;
		}
		commit_work();
		begin_work();
	}
	commit_work();
	picker_close( );

	picker_order_open( AUTOLOCK );

	while( picker_order_next( &picker_order, LOCK ) == 0 )
	{
		if( picker_order.p_completion_time == 0 )
		{
			if (picker_order_delete( ))
			{
				printf("Error During Delete Of Picker_Order %d\n", 
					picker_order.p_order_number);
				errors++;
			}
		}
	}
	picker_order_close( );
   database_close();
   
   if (errors)
   {
   	printf("\nPicker DB Clear Has Failed With %d Errors\n\n", errors);
   	exit(1);
	}
	printf("\nPicker DB Cleared\n\n\n");
	exit(0);
}

/*** End of picker_db_clear.c ***/
