/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Update picker database.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  11/04/95   |  tjt  ID from 5 to 9 digits.
 *  08/23/96   |  tjt  Modify begin and commit work.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *  06/05/01   |  aha  Added manual assignment of zone to picker.
 *  07/25/01   |  aha  Removed manual assignment of zone to picker.
 *-------------------------------------------------------------------------*/
static char picker_information_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"
#include "picker_information.t"
#include "Bard.h"
#include "bard/picker.h"

extern leave();

#define EMPLOYEE_NUM    0
#define LAST_NAME       1
#define FIRST_NAME      2
#define MIDDLE_NAME     3
#define ARE_YOU_SURE    4
#define NUM_PROMPTS     5

static short employee_num_size = 9;
static short name_size = 15;
static short ONE = 1;
#define MAX_BUF_SIZE 15

struct fld_parms fld[NUM_PROMPTS] = {
	{ 9,51,15,1,&employee_num_size,"Enter the picker's Employee No.",'n'},
	{11,51,15,1,&name_size,        "Enter picker's last name",'a'},
	{13,51,15,1,&name_size,        "Enter picker's first name",'a'},
	{15,51,15,1,&ONE,              "Enter picker's middle initial",'a'},
	{18,51,25,1,&ONE,              "Are you sure? (y/n)",'a'}
	};

static char buffers[NUM_PROMPTS][MAX_BUF_SIZE+1];  /* input buffer space */

static short rm = 0;  /* no one changes this, but we do need to be able to take
                         its address, so to avoid everyone declaring their own
                         useless copy, we make one global useless copy... */

#undef  DELETE
#define ADD    1
#define CHANGE 2
#define DELETE 3

/*------------------------------------------------------------------------ */

main(argc, argv)
int argc;
char *argv[];
{
  short i;
  unsigned char t;
  int   call_type;

  putenv("_=picker_information");
/*
 ** do setup and create the data on the screen
 */
  open_all();

  switch(argv[1][0]) 
  {
    case 'a': 

      call_type = ADD;    
      picker_information[76] = '1';
      memcpy(picker_information + 268, "Add", 3); 
      break;

    case 'c': 

      call_type = CHANGE; 
      picker_information[76] = '2';
      memcpy(picker_information + 265, "Change", 6);
      break;
       
    case 'd': 

      call_type = DELETE; 
      picker_information[76] = '3';
      memcpy(picker_information + 265, "Delete", 6); 
      break;
      
    default:  leave();
  }
  fix(picker_information);
  sd_screen_off();
  sd_clear_screen();
  sd_text(picker_information);
  sd_screen_on();
  
/*
 ** let a type specific function do all the real work
 */
  begin_work();
  
  switch (call_type) 
  {
    case ADD:    add_picker();    break;
    case CHANGE: change_picker(); break;
    case DELETE: delete_picker(); break;
  }
  commit_work();

  /*
  ** clean up and exit
  */
  leave();
}
/*end of main*/

/*
** process user interaction to add new picker information
*/
add_picker()
{
  char msg[32];
  picker_item  picker_record;

  while(1)
  {
    /* get user input */
    show_prompts();
    if (edit_data( EMPLOYEE_NUM, 1 ) == 0)
      return;

    /* fill in record */
    memset( (char *)&picker_record, 0, sizeof(picker_item) );
    strcpy(picker_record.p_first_name, buffers[FIRST_NAME]);
    strcpy(picker_record.p_last_name, buffers[LAST_NAME]);
    strcpy(picker_record.p_middle_initial, buffers[MIDDLE_NAME]);
    picker_record.p_picker_id = atol(buffers[EMPLOYEE_NUM]);
    picker_record.p_zone = 0;
    picker_record.p_status = 0;

    /* add record */
    if (picker_write( &picker_record ))
      sprintf(msg, "%s add failed!", buffers[EMPLOYEE_NUM]);
    else
      sprintf(msg, "%s added", buffers[EMPLOYEE_NUM]);
    eh_post(LOCAL_MSG, msg);
  }
}

/*
** process user interaction for changing picker information in the file
*/
change_picker()
{
  char msg[32];
  picker_item picker_record;
  
  while(1)
  {
    /*
    ** show and get information
    */
    show_prompts();
    if (get_one_picker_info() == 0) return 0;
    
    picker_record.p_picker_id = atol(buffers[EMPLOYEE_NUM]);
    if (edit_data( LAST_NAME, 0 ) != 0)
    {
      /* write change to the file */

      if (picker_read( &picker_record, LOCK) == 0)
      {
        strcpy(picker_record.p_first_name, buffers[FIRST_NAME]);
        strcpy(picker_record.p_last_name, buffers[LAST_NAME]);
        strcpy(picker_record.p_middle_initial, buffers[MIDDLE_NAME]);

        picker_record.p_picker_id = atol(buffers[EMPLOYEE_NUM]);
        picker_record.p_zone = 0;
        picker_record.p_status = 0;
        if (picker_update( &picker_record ) )
          sprintf(msg, "%s change failed!", buffers[EMPLOYEE_NUM]);
        else
          sprintf(msg, "%s changed", buffers[EMPLOYEE_NUM]);
      }
      else
        sprintf(msg, "%s not changed!", buffers[EMPLOYEE_NUM]);
      eh_post(LOCAL_MSG, msg);
    }
  }
}

/*
** process user interaction for deleting pickers from the file
*/
delete_picker()
{
  char msg[32];
  picker_item picker_record;
  
  while(1)
  {
    /*
    ** show and get information
    */
    show_prompts();
    if (get_one_picker_info() == 0) return 0;
    
    if (are_you_sure() == 1)
    {
      /* delete picker from the file */
      picker_record.p_picker_id = atol(buffers[EMPLOYEE_NUM]);
      if ( picker_read( &picker_record, LOCK) == 0)
      {
        if ( picker_delete( ) == 0)
          sprintf(msg, "%s deleted", buffers[EMPLOYEE_NUM]);
        else
          sprintf(msg, "%s delete failed!", buffers[EMPLOYEE_NUM]);
      }
      else
        sprintf(msg, "%s not deleted!", buffers[EMPLOYEE_NUM]);
      eh_post(LOCAL_MSG, msg);
    }
  }
}

/*
** allow user to edit the data and ask "are you sure?" when done.
** returns 0 if user hit EXIT, 1 otherwise
*/
int edit_data( first_index, fail_if_exists )
int first_index;
int fail_if_exists;
{
  unsigned char t;
  int index = first_index;

  while(1)
  {
    /*
    ** get user input until user presses RETURN on last field
    ** (or presses EXIT at any time)
    */
    t = sd_input(&fld[index],rm,&rm,buffers[index],0);

    switch (t) {
      case EXIT:
        return 0;
      case UP_CURSOR:
        if (index > first_index) index--;
        continue;
      case DOWN_CURSOR:
      case TAB:
        if (index < MIDDLE_NAME)
          index++;
        else
          index = first_index;
        continue;
      case RETURN:
        if (index < MIDDLE_NAME)
        {
          index++;
          continue;
        }
    }

    /*
    ** now check out all the given information and verify it as ok 
    */
    for (index = first_index; index <= FIRST_NAME; index++)
    {
      if (buffers[index][0] == '\0')
      {
        eh_post(LOCAL_MSG, "Missing Data Field(s)");
        break;
      }
    }
    if (index <= FIRST_NAME)
      continue;
    if (fail_if_exists && get_employee_info(buffers[EMPLOYEE_NUM], 0))
    {
      char msg[32];
      sprintf(msg, "%s already exists", buffers[EMPLOYEE_NUM]);
      eh_post(LOCAL_MSG, msg);
      index = first_index;
      continue;
    }

    /*
    ** all ok, so make sure user says go
    */
    switch (are_you_sure()) {
      case 1:  return 1;
      case -1: return 0;
      default: break;  /* fall through */
    }
  }
}

/*
** prompt for the employee number and fill in buffers with data,
** returns 0 if user hit EXIT else 1
*/
int get_one_picker_info()
{
  unsigned char t;
  char msg[32];

  while(1)
  {
    /*
    ** get employee number
    */
    t = sd_input(&fld[EMPLOYEE_NUM],rm,&rm,buffers[EMPLOYEE_NUM],0);
    if (t == EXIT) return 0;
    if (t != RETURN) continue;

    if (buffers[EMPLOYEE_NUM][0] == '\0')
    {
      eh_post(LOCAL_MSG, "Number Is Required");
      continue;
    }

    /*
    ** get data related to employee number
    */
    if (get_employee_info( buffers[EMPLOYEE_NUM], 1 ))
      return 1;   /* success */
    sprintf(msg, "%s unknown", buffers[EMPLOYEE_NUM]);
    eh_post(LOCAL_MSG, msg);
  }
}

/*
** zero buffers and display screen prompts
*/
show_prompts()
{
  int i;

  memset( buffers[0], 0, sizeof(buffers) );
  for (i = 0; i <= MIDDLE_NAME; i++)
    sd_prompt(&fld[i], rm);
}

/*
** get information about an employee then fill and show buffers if so indicated
** returns 0 if no such employee, 1 otherwise
*/
int get_employee_info( employee_num, fill_and_show )
char *employee_num;
int   fill_and_show;
{
  picker_item picker_record;

  /* search dB for record and fill local record structure */
  picker_record.p_picker_id = atol(employee_num);

  if (picker_read( &picker_record, NOLOCK )) return 0;

  /* record exists -- return now if caller doesn't want fill and show */
  if (!fill_and_show) return 1;

  /* fill global buffers */

  strcpy(buffers[FIRST_NAME], picker_record.p_first_name);
  strcpy(buffers[LAST_NAME],  picker_record.p_last_name);
  strcpy(buffers[MIDDLE_NAME],picker_record.p_middle_initial);

  strip_space(buffers[FIRST_NAME], MAX_BUF_SIZE + 1);
  strip_space(buffers[LAST_NAME],  MAX_BUF_SIZE + 1);
  strip_space(buffers[MIDDLE_NAME], 2);
  
  /* show information on screen and return */ 

  sd_cursor(0, fld[FIRST_NAME].irow, fld[FIRST_NAME].icol);
  sd_text(buffers[FIRST_NAME]);
  sd_cursor(0, fld[LAST_NAME].irow, fld[LAST_NAME].icol);
  sd_text(buffers[LAST_NAME]);
  sd_cursor(0, fld[MIDDLE_NAME].irow, fld[MIDDLE_NAME].icol);
  sd_text(buffers[MIDDLE_NAME]);

  return 1;
}

/*
** make sure the user wants this action to happen, returns
** 1 on yes, 0 on no, and -1 on EXIT
*/
int are_you_sure()
{
  unsigned char t, yn[2];

  while(1)
  {
    *yn = 0;

    sd_prompt(&fld[ARE_YOU_SURE], rm);
    t = sd_input(&fld[ARE_YOU_SURE],rm,&rm,yn,0);
    if (t == EXIT) break;
    *buffers[ARE_YOU_SURE] = code_to_caps(*yn);
    if ((buffers[ARE_YOU_SURE][0] == 'n') || (buffers[ARE_YOU_SURE][0] == 'y'))
      break;
    eh_post(ERR_YN, 0);
  }

  sd_cursor(0, fld[ARE_YOU_SURE].irow + rm, 1);
  sd_clear_line();
  if (buffers[ARE_YOU_SURE][0] == 'n')
    return 0;
  else if (buffers[ARE_YOU_SURE][0] == 'y')
    return 1;
  else
    return -1;
}

/*
 *open all files
 */
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  picker_open(AUTOLOCK); 
  picker_setkey(1);
  return 0;
}
/*
 *close all files
 */
close_all()
{
  sd_close();
  ss_close();
  picker_close( );
  database_close();
}
/*
 *transfer control back to calling program
 */
leave()
{
  close_all();
  execlp("picker_acctability", "picker_acctability",0);
  krash("leave", "picker_acctability load", 1);
  exit(1);
}

/* end of picker_information.c */
