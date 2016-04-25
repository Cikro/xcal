/********
* calutil.h -- Public interface for iCalendar utility functions in calutil.c
* Last updated: 10:30pm Apr. 7, 2016
*
* Nikolas Orkic 0854791
* E-mail: norkic@mail.uoguelph.ca
*
* writeCalComp added for A2.
********/

#ifndef CALUTIL_H
#define CALUTIL_H A2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FOLD_LEN 75     // fold lines longer than this length (RFC 5545 3.1)
#define VCAL_VER "2.0"  // version of standard accepted
#define READ_BUFF_SIZE 75 //size of a buffer; used when reading lines of ics files
#define PARSE_BUFF_SIZE 75 //size of a buffer; used when parsing lines of ics files

/* data structures for ICS file in memory */

typedef struct CalParam CalParam;
typedef struct CalParam {    // property's parameter
    char *name;         // uppercase
    CalParam *next;     // linked list of parameters (ends with NULL)
    int nvalues;        // no. of values
    char *value[];      // uppercase or "..." (flexible array member)
} CalParam;

typedef struct CalProp CalProp;
typedef struct CalProp {    // (sub)component's property (=contentline)
    char *name;         // uppercase
    char *value;
    int nparams;        // no. of parameters
    CalParam *param;    // -> first parameter (or NULL)
    CalProp *next;      // linked list of properties (ends with NULL)
} CalProp;

typedef struct CalComp CalComp;
typedef struct CalComp {    // calendar's (sub)component
    char *name;         // uppercase
    int nprops;         // no. of properties
    CalProp *prop;      // -> first property (or NULL)
    int ncomps;         // no. of subcomponents
    CalComp *comp[];    // component pointers (flexible array member)
} CalComp;


/* General status return from functions */

typedef enum { OK=0,
    AFTEND,     // more text found after end of calendar 
    BADVER,     // version missing or wrong
    BEGEND,     // BEGIN...END not found as expected
    IOERR,      // I/O error
    NOCAL,      // outer block not VCALENDAR, or no V components found
    NOCRNL,     // CRNL missing at end of line
    NODATA,     // nothing between BEGIN...END
    NOPROD,     // PRODID missing
    SUBCOM,     // subcomponent not allowed
    SYNTAX,     // property not in valid form
} CalError;
    
typedef struct {
    CalError code;          // error code
    int linefrom, lineto;   // line numbers where error occurred
} CalStatus;    


/* File I/O functions */

CalStatus readCalFile( FILE *const ics, CalComp **const pcomp );
CalStatus readCalComp( FILE *const ics, CalComp **const pcomp );
CalStatus readCalLine( FILE *const ics, char **const pbuff );
CalError parseCalProp( char *const buff, CalProp *const prop );
CalStatus writeCalComp( FILE *const ics, const CalComp *comp );
void freeCalComp( CalComp *const comp );

/*Data Structure  Management functions*/

/*InitializeCalComp
*
* Purpose: To create an empty CalComp sctructre
*
* Post Conditions:A CalComp sctructure is returned with 
* malloced memory and null values
********************************************************************************************/
CalComp *InitializeCalComp();

/*InitializeCalProp
*
* Purpose: To create an empty Calprop sctructre
*
* Post Conditions:A CalProp sctructure is returned with 
* malloced memory and null values
********************************************************************************************/
CalProp *InitializeCalProp();

/*InitializeCalParam
*
* Purpose: To create an empty CalParam sctructre
*
* Post Conditions:A CalParam sctructure is returned with 
* malloced memory and null values
********************************************************************************************/
CalParam *InitializeCalParam();

/*InitializeCalStatus
*
* Purpose: To create an empty CalStatus sctructre
*
* Post Conditions:A CalStatus sctructure is returned with 
* malloced memory and null values
********************************************************************************************/
CalStatus InitializeCalStatus();


/*insertProperty
*
* Purpose: to insert a CalProperty into a Cal Component
*
* Arguments: A pointer to the address of an initialize pcomp (CalComp **),
*           and a pointer to the property desired to add (CalProp *).
*
* PostConditions: The CalComp's number of properties increases and and desired property is added to the CalComp. 
********************************************************************************************/
void insertProperty(CalComp **const pcomp, CalProp *toAdd);

/*insertParam
*
* Purpose: to insert a parameter into a CalProp's data scture.
*
* Arguments: A pointer to an allocated CalProp Scture (CalProp*), and
*            a pointer to an allocated CalParameter (CalParam*) to be inserted.
*
* Postconditions: The CalParameter inserted into the CalProperty's data sctructure
********************************************************************************************/
void insertParam(CalProp *prop,CalParam *toAdd);

/*expandCalComp
*
* Purpose: to reallocate memory for, and insert into a CalComp's felible array of CalComps
*
* Arguments:A Pointer to the address of an allocated CalComp (CalComp **) and a pointer to the 
*           CalComp wished to be inserted (CalComp*)
*
*PostConditions: The CalComp's flexible array size has been increased and the CalComp has been added to it
*                as desired
********************************************************************************************/
void expandCalComp(CalComp **const toExpand, CalComp *toAdd);

/*expandCalParam
*
* Purpose: to reallocate memory for, and insert into a CalParam's felible array of Values
*
* Arguments:A Pointer to the address of an allocated CalParam (CalParam **) and a pointer to the 
*           value wished to be inserted (char*)
*
*PostConditions: The CalParam's flexible array size has been increased and the value has been added to it
*                as desired
********************************************************************************************/
void expandCalParam(CalParam **const toExpand, char *toAdd);

/*updateLines
*
* Purpose: to make the lines of a CalStatus equal to eachother
*
* Arguments: A pointer to a CalStatus (CalStatus*)
*
* PostConditions: The linefrom and lineto variables from the CalStatus will be equal to the largest of the two values.
********************************************************************************************/
void updateLines(CalStatus *status);

/*toUpper
*
* Purpose: To create an uppercase version of a string
*
* Arguments: A string (char *)
*
* Returns: - A newly allocated string that is equal to the previous string, except
*            with all letters capitalized.
*          - NULL: if string is NULL
********************************************************************************************/
char* toUpper(char *string);

#endif
