/********
* caltool.h -- Public interface for iCalendar tools in caltool.c
* Last updated: 10:31pm Apr. 7, 2016
*
* Nikolas Orkic 0854791
* E-mail: norkic@mail.uoguelph.ca
*
********/

#ifndef CALTOOL_H
#define CALTOOL_H A2_RevA

#define _GNU_SOURCE     // for getdate_r
#define _XOPEN_SOURCE   // for strptime
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "calutil.h"

typedef struct CalOrganizer
{
    char *name;
    char *contact;
}CalOrganizer;

typedef struct CalEvent
{
    char *summary;
    char *location;
    struct tm *dateStart;
    CalOrganizer *org;
}CalEvent;

typedef struct CalTodo
{
    char *summary;
    char *priority;
    CalOrganizer *org;
}CalTodo;

/*struct used to store information about a CalComp*/
typedef struct CalInfo
{
    int lines;
    int comps;
    int nCompEvents;
    int subcomps;
    int todos;
    int other;
    struct tm *early;
    struct tm *late;
    int props;
    int norgs;
    char **orgs;
    int nevents;
    CalEvent **events;
    int nxprops;
    char **xprops;
}CalInfo;

/* Symbols used to send options to command execution modules */
typedef enum {
    OEVENT,     // events
    OPROP,      // properties
    OTODO,      // to-do items
} CalOpt;

/* iCalendar tool functions */

CalStatus calInfo( const CalComp *comp, int lines, FILE *const txtfile );
CalStatus calExtract( const CalComp *comp, CalOpt kind, FILE *const txtfile );
CalStatus calFilter( const CalComp *comp, CalOpt content, time_t datefrom, time_t dateto, FILE *const icsfile );
CalStatus calCombine( const CalComp *comp1, const CalComp *comp2, FILE *const icsfile );

/*Helper Functions*/

/*printCalError
*
* Purpose: To output an error message on stderr based on an error code stored in a 
*          CalStatus.
*
*
* Arguments: - A CalStatus struct (Calstatus).
*
********************************************************************************************/
void printCalError(CalStatus stat);

/*InitializeCalInfo
*
* Purpose: To set all of a CalInfo structs variables to 0/NULL.
*
* Returns: an initialized CalInfo sctruct 
*
*
********************************************************************************************/
CalInfo InitializeCalInfo();

/*InitializeCalEvent
*
* Purpose: To set all of a CalEvent structs variables to 0/NULL.
*
* Returns: an initialized CalEvent sctruct 
*
*
********************************************************************************************/
CalEvent *InitializeCalEvent();
CalTodo *InitializeCalTodo();
CalOrganizer *InitializeCalOrganizer();

/*freeCalInfo
*
* Purpose: to Free memory allocated in a CalInfo structure.
*
*
********************************************************************************************/
void freeCalInfo(CalInfo *info);

/*freeCalEvent
*
* Purpose: to Free memory allocated in a CalEvent structure.
*
*
********************************************************************************************/
void freeCalEvent(CalEvent *Event);
void freeCalTodo(CalTodo *todo);
void freeCalOrganizer(CalOrganizer *org);

/*extractEvent
*
* Purpose: To allocate memory for and populate a new CalEvent structure from a CalComp
*
* Arguments:   - a pointer a CalComp (CalComp*)
*              - a pointer to date 2 (void*)
*
* Returns: - The address of an allocated and populated CalEvent structure.
*          - If no Event was found, NULL is returned.
********************************************************************************************/
CalEvent *extractEvent(CalComp const *comp);
CalTodo *extractTodo(CalComp const *comp);
void populateOrganizer (CalProp *orgProp, CalOrganizer *org);
#endif
