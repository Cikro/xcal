/********
*caltool.c --  A group of functions used to manipulate and gain information about ics files.
*
* Last updated: 10:34pm Apr. 7, 2016
*
* Nikolas Orkic 0854791
* E-mail: norkic@mail.uoguelph.ca
*
********/
#include "caltool.h"

/*findCalNumbers
*
* Purpose: To search through a populated CalComp, find the number of components, the number
*          of events, todos and 'other' components that make up the CalComp's components, and
*          and total number of subcomponents.
*
*
* Arguments:   - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
*
* Post-Conditions: - The information from the CalComp struct will be counted and the totals
*                    will be stored in the CalInfo Struct.
*
********************************************************************************************/
static void findCalNumbers(const CalComp *comp,CalInfo *info);

/*findEarlyAndLateTimes
*
* Purpose: To search through a populated CalComp, find the Earliest and latest dates
*          that occur, and populates the 'early' and 'late' fields of the CalComp struct.
*
*
* Arguments:   - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
*
* Post-Conditions: - The earliest and latest dates will be stored in the
*                    provided CalInfo struct
*                  - If no times are found, the contents of info will be unchanged. 
*
********************************************************************************************/
static void findEarlyAndLateTimes(const CalComp *comp,CalInfo *info);

/*findOrganizers
*
* Purpose: To search through a populated CalComp, find all of the Organizers,
*          and populates the 'orgs' field of a CalInfo struct with an array of
*          the organizer's common names and the 'norgs' field with the number
*          of common names stored.
*
*
* Arguments:   - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
*
* Post-Conditions: - The array of the organizer's common names will be stored in
*                    the 'orgs' field of the CalInfo struct and the 'norgs' field will
*                    store the number of common names in the 'orgs' array.
*                  - If no organizers are found, the contents of info will be unchanged. 
*
********************************************************************************************/
static void findOrganizers(const CalComp *comp,CalInfo *info);

/*findEvents
*
* Purpose: To search through a populated CalComp, find all of it's Events,
*          populates the 'events' field of a CalInfo struct with each Event
*          (in a CalEvent struct) and the 'nevents' field with the number of events stored.
*
*
* Arguments:   - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
*
* Post-Conditions: - An array of CalEvents will be stored in the 'events' field
*                    of the CalInfo struct and the 'nevents' field will store the number 
*                    of events in the 'events' array.
*                  - If no events are found, the contents of info will be unchanged. 
*
********************************************************************************************/
static void findEvents(const CalComp *comp,CalInfo *info);

/*findXprops
*
* Purpose: To search through a populated CalComp, find all of it's X-properties,
*          populates the 'xprops' field of a CalInfo struct and the 'nxprops'
*          field with the number of xprops stored.
*
*
* Arguments:   - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
*
* Post-Conditions: - The 'xprops' field of the CalInfo will be populated and the 'nxprops'
*                    field will hold the number of xprops found.
*                  - If no xprops are found, the contents of info will be unchanged. 
*
********************************************************************************************/
static void findXprops(const CalComp *comp,CalInfo *info);

/*expandStringArray
*
* Purpose: To add a string to an array of strings.
*
* Arguments: - An array of strings (char **).
*            - An allocated string to add to the array (char *).
*            - the address of an integer the size of the string array (int*).
*
*
* Returns: - The address of an expaned String array
*
* Post-Conditions: - A newly allocated string string will be added to the end of the array
*                    and the integer value will be incremented. 
*
*
********************************************************************************************/
static char** expandStringArray(char ** arr, char *toAdd, int *arrSize);

/*removeFromStringArray
*
* Purpose: To remove a string from an array of strings.
*
* Arguments: - An array of strings (char **).
*            - The position of the string to be removed (int).
*            - the address of an integer the size of the string array (int*).
*
* Returns: - The address of an expaned String array
*
* Post-Conditions: - A newly allocated string string will be added to the end of the array
*                    and the integer value will be decremented. 
*
*
********************************************************************************************/
static char** removeFromStringArray(char ** arr, int pos, int *arrSize);

/*expandCalEventArray
*
* Purpose: To add a string to an array of strings.
*
* Arguments: - An array of CalEvents (CalEvent **).
*            - An allocated CalEvent to add to the array (CalEvent *).
*            - the address of an integer the size of the string array (int*).
*
*
* Returns: - The address of an expaned CalEvent array
*
* Post-Conditions: - A newly allocated string string will be added to the end of the array
*
*
********************************************************************************************/
static CalEvent **expandCalEventArray(CalEvent ** arr, CalEvent *toAdd, int *arrSize);

/*writeInfo
*
* Purpose: To write the contents of a CalStatus Struct not associated to 
*          the 'xprops' and 'events' fields to the desired file.
*
* Pre-Conditions: - CalInfo's lines variable is greater than 2
*                 - All other integers in CalInfo are not negative
*
*
* Arguments:   - An open file stream (FILE *).
*              - A CalStatus struct (Calstatus).
*              - An initalized CalInfo Struct (CalInfo)
*
* Post-Conditions: - The information from the CalInfo struct will be written to file, status 
*                    will contain the code OK and status' lineto = linefrom = total number of
*                    lines successfully written.
*
*                  - If writing to the file fails, Status will conatin the code 'IOERR' and
*                    will contain the number of line successfully written.
*
*******************************************************************************************/
static CalStatus writeInfo(FILE *const file, CalStatus status, CalInfo info);

/*writeStringPlur
*
* Purpose: To write an integer with a descriptor string to an open file.
*          The string will be either plural or singular depending on the integer value.
*
* Pre-Conditions: - CalInfo's lines variable is greater than 2
*                 - All other integers in CalInfo are not negative
*
* Arguments:   - An open file stream (FILE *).
*              - An integer (int)
*              - A singular string (char *.
*              - A plural string (cahr *)
*
* Returns: - 1 on success
*          - 0 on failure
*
* Post-Conditions: -On success an integer will be written to the open file stream along with either a
*                   singular or plural string.
*
********************************************************************************************/
static int writeStringPlur(FILE *const file, int num, char *singular, char *plural);

/*compareString
*
* Purpose: To compare two strings, ignoring case alphabetically.
*
*
* Arguments:   - a pointer to string 1 (void*)
*              - a pointer to string 2 (void*)
*
* Returns: - -1 if string 1 is less than string 2
*          - 0 if string 1 equals sting 2
*          - 1 if string 1 is larger than string 2
*
*
********************************************************************************************/
static int compareString (const void* str1, const void* str2);

/*compareDate
*
* Purpose: To compare two CalDates based on their starting date.
*
* Arguments:   - a pointer to date 1 (void*)
*              - a pointer to date 2 (void*)
*
* Returns: - -1 if date1 comes before date2
*          - 0 if date1 and date2 are equal
*          - 1 if date1 comes after date2
********************************************************************************************/
static int compareDate (const void* date1, const void* date2);


/*filter
*
* Purpose: To filter a CalComp structure down to only values specified by CalOpt and between
*          datefrom and dateto.
*
* Arguments:   - a pointer to a CalComp (CalComp*)
*              - A CalOpt containg either 'OEVENT' or 'OTODO'. (CalOpt)
*              - The lower bound date. (datefrom)
*              - The upper bound date. (dateto)
*
********************************************************************************************/
static void filter(CalComp *comp, CalOpt opt,time_t datefrom,time_t dateto);

/*filterSubComp
*
* Purpose: To Determine wether or not a CalComponent needs to be filtered based on CalOpt, dateto and datefrom.
*
* Arguments:   - a pointer to a CalComp (CalComp*)
*              - A CalOpt containg either 'OEVENT' or 'OTODO'. (CalOpt)
*              - The lower bound date. (datefrom)
*              - The upper bound date. (dateto)
*
* Returns: - 1 if it requires filtering
*          - 0 if it does not require filtering
********************************************************************************************/
static int filterSubComp(CalComp *comp, CalOpt opt,time_t datefrom,time_t dateto);

/*writeExtractedKind
*
* Purpose: To write information about extracted CalEvents or X-properties
*          to a file.
*
* Pre-Conditions: - The File stream os opened.
*
* Arguments:   - An open file stream (FILE *).
*              - A populated CalInfo Structure (CalInfo)
*              - The kind of extracted information (CalOpt).
*
* Returns: - A CalStatus which stores the number of lines that
*            were written to the file and a code of OK.
*
*          - If an error occured writting to the file, the 
*            CalStatus's code will contain IOERROR
*
* Post-Conditions: - Information about the kind of information extracted
*                    will be outputted to the file.
*
********************************************************************************************/
static CalStatus writeExtractedKind(FILE *file,CalInfo info,CalOpt kind);

int main (int argc, char *argv[])
{
    char *flag, *fileName;
    FILE *openFile;
    CalComp *comp, *fileComp;
    CalStatus inStat, stat, fileStat;
    CalOpt opt;
    struct tm *tempTm;
    time_t datefrom, dateto, t;
    int dtErr;
    tempTm = NULL;
    comp = NULL;
    dateto = 0;
    datefrom = 0;
    if (argc <2)
    {
        fprintf(stderr,"ERROR: no arguments. See readme for list of valid arguments\n");
        return(EXIT_FAILURE);
    }

    flag = argv[1];
    //INFO
    if (strcmp("-info",flag) == 0)
    {
        if (argc > 2)
        {
            fprintf(stderr,"ERROR: invalid argument(s): ");
            for(int i = 2; i < argc; i++)
            {
                fprintf(stderr,"'%s'",argv[i]);
            }
            fprintf(stderr,"\n");
            return(EXIT_FAILURE);
        }
        inStat = readCalFile(stdin,&comp);

        if (inStat.code != OK)
        {
            printCalError(inStat);
            return(EXIT_FAILURE);
        }

        //Print Info 
        stat = calInfo(comp, inStat.lineto,stdout);

        if (stat.code != OK)
        {
            printCalError(stat);
            freeCalComp(comp);
            return(EXIT_FAILURE);
        }
        freeCalComp(comp);
    }
    //EXTRACT
    else if (strcmp("-extract",flag) == 0)
    {
        //Check OPT flag
        if (argc < 3)
        {
            fprintf(stderr,"ERROR: -filter requires more arguments.\n");
            return(EXIT_FAILURE);
        }

        if (strcmp("e",argv[2]) == 0)
        {
            opt = OEVENT;
        }
        else if (strcmp("x",argv[2]) == 0)
        {
            opt = OPROP;
        }
        else
        {
            fprintf(stderr,"ERROR: invalid argument for -filter. \n");
            return(EXIT_FAILURE); 
        }
        
        if (argc > 3)
        {
            fprintf(stderr,"ERROR: invalid argument(s) : ");
            for(int i = 2; i < argc; i++)
            {
                fprintf(stderr,"'%s'",argv[i]);
            }
            fprintf(stderr,"\n");
            return(EXIT_FAILURE);
        }

        //read file
        inStat = readCalFile(stdin,&comp);

        if (inStat.code != OK)
        {
            printCalError(inStat);
            return(EXIT_FAILURE);
        }

        stat = calExtract(comp,opt,stdout);

        if (stat.code != OK)
        {
            printCalError(stat);
            freeCalComp(comp);
            return(EXIT_FAILURE);
        }
        freeCalComp(comp);
    }
    //FILTER
    else if (strcmp("-filter",flag) == 0)
    {
        //check OPT flag
        if (argc < 3)
        {
            fprintf(stderr,"ERROR: -filter requires an additional argument. Try '-filter e from \"date\" to \"date\"' or '-filter t from \"date\" to \"date\"'\n");
            return(EXIT_FAILURE);
        }

        if (strcmp("e",argv[2]) == 0)
        {
            opt = OEVENT;
        }
        else if (strcmp("t",argv[2]) == 0)
        {
            opt = OTODO;
        }
        else
        {
            fprintf(stderr,"ERROR: '%s' invalid argument for -filter.\n",flag);
            return(EXIT_FAILURE); 
        }

        //Determin timeTo and From,
        tempTm = malloc(sizeof(struct tm));
        if (argc >= 4) 
        {
            //From must come first if it exists
            if (strcmp(argv[3],"from") == 0)
            {
                if (argc >= 5)
                {
                    //check date for today
                    if (strcmp(argv[4],"today") == 0)
                    {
                        t = time(NULL);
                        *tempTm = *localtime(&t);
                        
                    }
                    else
                    {
                        dtErr = getdate_r(argv[4],tempTm);
                        assert(dtErr != 6);
                        if (dtErr >= 1 && dtErr <= 5)
                        {
                            fprintf(stderr,"Problem with DATEMSK environment variable or template file. \n");
                            free(tempTm);
                            return(EXIT_FAILURE);
                        }
                        if (dtErr >= 7 && dtErr <= 8)
                        {
                            fprintf(stderr,"The 'from' date  could be be interpreted. \n");
                            free(tempTm);
                            return(EXIT_FAILURE);
                        }
                    }
                    tempTm->tm_sec = 0;
                    tempTm->tm_min = 0;
                    tempTm->tm_hour = 0;
                    tempTm->tm_isdst = -1;
                    datefrom = mktime(tempTm);
                }
                else
                {
                    fprintf(stderr,"ERROR: invalid argument. Syntax is 'from \"date\"'\n");
                    free(tempTm);
                    return(EXIT_FAILURE);                    
                }
            }
            //If its not 'From', error
            else
            {
                fprintf(stderr,"ERROR: 'from \"date\"' argument must occur first\n");
                free(tempTm);
                return(EXIT_FAILURE);
            }

        }
        if (argc >= 6) 
        {
            if(strcmp(argv[5],"to") == 0)
            {
                if (argc >= 7)
                {
                    if (strcmp(argv[6],"today") == 0)
                    {
                        t = time(NULL);
                        *tempTm = *localtime(&t);
                        
                    }
                    else
                    {
                        dtErr = getdate_r(argv[6],tempTm);
                        assert(dtErr != 6);    
                        if (dtErr >= 1 && dtErr <= 5)
                        {
                            fprintf(stderr,"Problem with DATEMSK environment variable or template file. \n");
                            free(tempTm);
                            return(EXIT_FAILURE);
                        }
                        if (dtErr >= 7 && dtErr <= 8)
                        {
                            fprintf(stderr,"The 'to' date could not be interpreted. \n");
                            free(tempTm);
                            return(EXIT_FAILURE);
                        
                        }
                    }

                    tempTm->tm_sec = 0;
                    tempTm->tm_min = 59;
                    tempTm->tm_hour = 23;
                    tempTm->tm_isdst = -1;
                    dateto = mktime(tempTm);   
                }
                else
                {
                    fprintf(stderr,"ERROR: invalid argument. Syntax is 'to \"date\"'\n");
                    free(tempTm);
                    return(EXIT_FAILURE);
                }
            }
            //To did not occur as expected
            else
            {
                fprintf(stderr,"ERROR: expected 'to \"date\"' argument.\n");
                free(tempTm);
                return(EXIT_FAILURE);
            }
        }
        free(tempTm);
        if (dateto != 0 && datefrom != 0)
        {
            //from is not eariler than to
            if (dateto < datefrom)
            {
                fprintf(stderr,"ERROR: 'from \"date\" must occur eariler than 'to \"date\"'.\n");
                return(EXIT_FAILURE);
            }
        }

        inStat = readCalFile(stdin,&comp);
        if (inStat.code != OK)
        {
            printCalError(inStat);
            return(EXIT_FAILURE);
        }

        stat = calFilter(comp,opt,datefrom,dateto,stdout);
        if (stat.code != OK)
        {
            printCalError(stat);
            freeCalComp(comp);
            return(EXIT_FAILURE);
        }
        freeCalComp(comp);
    }
    //COMBINE
    else if (strcmp("-combine",flag) == 0)
    {
        //Textfile
        if (argc < 3)
        {
            fprintf(stderr,"ERROR: Syntax is '-combine fileName'\n");
            return(EXIT_FAILURE);
        }
        fileName = argv[2];
        openFile = fopen(fileName,"r");

        if (openFile == NULL)
        {
            fprintf(stderr,"error opening file.\n");
            return(EXIT_FAILURE);
        }

        inStat = readCalFile(stdin,&comp);
        if (inStat.code != OK)
        {
            fclose(openFile);
            printCalError(inStat);
            return(EXIT_FAILURE);
        }

        fileStat = readCalFile(openFile,&fileComp);
        if (fileStat.code != OK)
        {
            fclose(openFile);
            printCalError(fileStat);
            return(EXIT_FAILURE);
        }

        stat = calCombine(fileComp,comp,stdout);
        freeCalComp(comp);
        freeCalComp(fileComp);
        fclose(openFile);
    }
    else
    {
        fprintf(stderr,"ERROR: invalid arguments. See readme for list of valid arguments\n");
        return(EXIT_FAILURE);

    }
    return(EXIT_SUCCESS);
}

CalInfo InitializeCalInfo()
{
    CalInfo info;

    info.lines = 0;
    info.comps = 0;
    info.nCompEvents = 0;
    info.subcomps = 0;
    info.todos = 0;
    info.other = 0;
    info.early = NULL;
    info.late = NULL;
    info.props = 0;
    info.norgs = 0;
    info.orgs = NULL;
    info.nevents = 0;
    info.events = NULL;
    info.nxprops = 0;
    info.xprops = NULL;

    return (info);
}
CalEvent *InitializeCalEvent()
{
    CalEvent *event = malloc(sizeof(CalEvent));
    assert(event != NULL);

    event->dateStart = NULL;
    event->summary = NULL;
    event->org = NULL;
    event->location = NULL;
    return(event);
}

CalTodo *InitializeCalTodo()
{
    CalTodo *todo = malloc(sizeof(CalTodo));
    assert(todo != NULL);

    todo->summary = NULL;
    todo->priority = NULL;
    todo->org = NULL;

    return(todo);
}
CalOrganizer *InitializeCalOrganizer()
{
    CalOrganizer *org = malloc(sizeof(CalOrganizer));
    assert(org != NULL);

    org->name = NULL;
    org->contact = NULL;
    
    return(org);
}
void freeCalInfo(CalInfo *info)
{

    free(info->early);
    free(info->late);

    info->early = NULL;
    info->late = NULL;
    //Free Organizers
    for (int i = 0; i < info->norgs; i++)
    {
        free(info->orgs[i]);
        info->orgs[i] = NULL;
    }

    //free events
    for (int i = 0; i < info->nevents; i++)
    {
        freeCalEvent(info->events[i]);
        info->events[i] = NULL;
    }
    //X-Properties
    for (int i = 0; i < info->nxprops; i++)
    {
        free(info->xprops[i]);
        info->xprops[i] = NULL;
    }
    free(info->events);
    free(info->orgs);
    free(info->xprops);
}
void freeCalEvent(CalEvent *event)
{
    free(event->dateStart);
    free(event->summary);
    free(event->location);
    if (event->org != NULL)
    {
        freeCalOrganizer(event->org);
    }
    free(event);
}
void freeCalTodo(CalTodo *todo)
{
    free(todo->summary);
    free(todo->priority);
    if (todo->org != NULL)
    {
        freeCalOrganizer(todo->org);
    }
    free(todo);
}
void freeCalOrganizer(CalOrganizer *org)
{
    free(org->name);
    free(org->contact);
    free(org);
}
static void findCalNumbers(const CalComp *comp,CalInfo *info)
{
    CalInfo subInfo;
    char *name;

    subInfo = InitializeCalInfo();

    info->props = comp->nprops;
    info->comps = comp->ncomps;

    //Loop Through components
    for (int i = 0; i < comp->ncomps; i++)
    {
        name = comp->comp[i]->name;
        /*count events, todos and others. These will be 
          subcomponents in the recursive call*/
        if (strcmp(name, "VEVENT") == 0)
        {
            info->nCompEvents += 1;
        }
        else if (strcmp(name, "VTODO") == 0)
        {
            info->todos += 1;
        }
        else
        {
            info->other += 1;
        }
        findCalNumbers(comp->comp[i],&subInfo);
        info->subcomps += subInfo.comps;
        info->props += subInfo.props;
    }

    freeCalInfo(&subInfo);
    return;
}
static void findEarlyAndLateTimes(const CalComp *comp,CalInfo *info)
{
    CalInfo subInfo;
    CalProp *tempProp;
    char *propName;

    struct tm tempTm;
    time_t t, early, late;

    tempProp = NULL;
    subInfo = InitializeCalInfo();


    //Loop Through components
    for (int i = 0; i < comp->ncomps; i++)
    {
        findEarlyAndLateTimes(comp->comp[i],&subInfo);
    }

    //Go through properties
    tempProp = comp->prop;
    while(tempProp != NULL)
    {
        propName = tempProp->name;

        //Find Times
        if (strcmp(propName,"DTSTART") == 0 || strcmp(propName,"DTEND") == 0 ||
            strcmp(propName,"DUE") == 0 || strcmp(propName,"COMPLETED") == 0 ||
            strcmp(propName,"LAST-MODIFIED") == 0 || strcmp(propName,"CREATED") == 0 ||
            strcmp(propName,"DTSTAMP") == 0 )
        {
            strptime(tempProp->value,"%Y%m%dT%H%M%S",&tempTm);
            tempTm.tm_isdst = -1;

            t = mktime(&tempTm);

            if(info->early == NULL)
            {
                info->early = malloc(sizeof(struct tm));
                *(info->early) = tempTm;
            }
            else
            {
                early = mktime(info->early);
                if (t < early)
                {
                    *(info->early) = tempTm;
                }  
            }
            if(info->late == NULL)
            {
                info->late = malloc(sizeof(struct tm));
                *(info->late) = tempTm;
            }
            else
            {
                late = mktime(info->late);
                if (t > late)
                {
                    *(info->late) = tempTm;
                } 
            }
        }

        tempProp = tempProp->next;
    }

    //update time from recursive call
    if (subInfo.early != NULL)
    { 
        t = mktime(subInfo.early);
        if(info->early == NULL)
        {

            info->early = malloc(sizeof(struct tm));
            *(info->early) = *(subInfo.early);
        }
        else
        {
            early = mktime(info->early);
            if (t < early)
            {
                *(info->early) = *(subInfo.early);
            }  
        }
    }
    if (subInfo.late != NULL)
    {     
        t = mktime(subInfo.late);
        if(info->late == NULL)
        {
            info->late = malloc(sizeof(struct tm));
            *(info->late) = *(subInfo.late);
        }
        else
        {
            late = mktime(info->late);
            if (t > late)
            {
                *(info->late) = *(subInfo.late);
            } 
        }
    }

    freeCalInfo(&subInfo);
    return;
}

static void findOrganizers(const CalComp *comp,CalInfo *info)
{
    CalInfo subInfo;
    CalProp *tempProp;
    CalParam *tempParam;
    char  *propName, *paramName, *orgName;

    tempProp = NULL;
    tempParam = NULL;
    subInfo = InitializeCalInfo();

    //Loop Through components
    for (int i = 0; i < comp->ncomps; i++)
    {
        findOrganizers(comp->comp[i],&subInfo);
    }

    //Go through properties
    tempProp = comp->prop;
    while(tempProp != NULL)
    {
        propName = tempProp->name;

        //Find Organizers
        if (strcmp(propName,"ORGANIZER") == 0)
        {
            //Look through Parameters
            tempParam = tempProp->param;
            while(tempParam != NULL)
            {
                for (int j = 0; j < tempParam->nvalues; j++)
                {
                    paramName = tempParam->name;
                    //ORGANIZER's common name
                    if (strcmp(paramName,"CN") == 0)
                    {
                        //Store CN
                        orgName = malloc(sizeof(char)*strlen(tempParam->value[j]) +1);
                        assert(orgName != NULL);
                        strcpy(orgName,tempParam->value[j]);
                        info->orgs = expandStringArray(info->orgs,orgName,&(info->norgs));
                        free(orgName);
                    }
                }
             tempParam = tempParam->next;
            }

        }
        tempProp = tempProp->next;
    }

    //update organizers from recursive call
    for (int i = 0; i < subInfo.norgs; i++)
    {
       info->orgs = expandStringArray(info->orgs,subInfo.orgs[i],&(info->norgs));

    }

    freeCalInfo(&subInfo);
    return; 
}

static void findEvents(const CalComp *comp,CalInfo *info)
{
    CalInfo subInfo;
    char *compName;
    CalEvent *cEvent;


    cEvent = NULL;
    subInfo = InitializeCalInfo();

    for (int i = 0; i < comp->ncomps; i++)
    {
        compName = comp->comp[i]->name;
        //extract the comp, if it is an event
        if (strcmp(compName, "VEVENT") == 0)
        {
            cEvent = extractEvent(comp->comp[i]);
            if (cEvent != NULL)
            {
                info->events = expandCalEventArray(info->events,cEvent,&(info->nevents));
                freeCalEvent(cEvent);
                // free(cEvent->dateStart);
                // free(cEvent->summary);
                // free(cEvent);
            }
        }
        findEvents(comp->comp[i],&subInfo);
    }
    //update Events from recursive call
    for (int i = 0; i < subInfo.nevents; i++)
    {
       info->events = expandCalEventArray(info->events,subInfo.events[i],&(info->nevents));
    }
    freeCalInfo(&subInfo);
    return;
}
static void findXprops(const CalComp *comp,CalInfo *info)
{
    CalInfo subInfo;
    CalProp *tempProp;
    char *propName, *xpName;


    tempProp = NULL;
    subInfo = InitializeCalInfo();

    //Loop Through components
    for (int i = 0; i < comp->ncomps; i++)
    {
        findXprops(comp->comp[i],&subInfo);
    }

    //Go through properties
    tempProp = comp->prop;
    while(tempProp != NULL)
    {
        propName = tempProp->name;
        //if its an X-property
        if (strncmp(propName,"X-",2) == 0)
        {
            //Store Xprop's Name
            xpName = malloc(sizeof(char)*strlen(tempProp->name) +1);
            assert(xpName != NULL);
            strcpy(xpName,tempProp->name);
            info->xprops = expandStringArray(info->xprops,xpName,&(info->nxprops));
            free(xpName); 
        }
        tempProp = tempProp->next;
    }

    //update xprops from recursive call
    for (int i = 0; i < subInfo.nxprops; i++)
    {
       info->xprops = expandStringArray(info->xprops,subInfo.xprops[i],&(info->nxprops));
    }
    freeCalInfo(&subInfo);
    return;
}
static char** expandStringArray(char ** arr, char *toAdd, int *arrSize)
{
    char ** newArr;
    char *cpy;
    newArr = malloc(sizeof(char*)*((*arrSize)+1));
    assert(newArr != NULL);
    for (int i = 0; i < *arrSize; i++)
    {
        cpy = malloc(sizeof(char)*strlen(arr[i])+1);
        assert(cpy != NULL);

        strcpy(cpy,arr[i]);
        free(arr[i]);

        newArr[i] = cpy;
        cpy = NULL;
    }
    cpy = malloc(sizeof(char)*strlen(toAdd)+1);
    assert(cpy != NULL);

    strcpy(cpy,toAdd);
    newArr[*arrSize] = cpy;
    *arrSize += 1;

    free(arr);
    return(newArr);


}
static char** removeFromStringArray(char ** arr, int pos, int *arrSize)
{
    char ** newArr;
    char *cpy;
    int count;

    newArr = malloc(sizeof(char*)*((*arrSize)-1));
    assert(newArr != NULL);

    count = 0;
    for (int i = 0; i < *arrSize; i++)
    {
        if (i == pos)
        {
            free(arr[i]);
        }
        else
        {
            cpy = malloc(sizeof(char)*strlen(arr[i])+1);
            assert(cpy != NULL);

            strcpy(cpy,arr[i]);
            free(arr[i]);

            newArr[count] = cpy;
            cpy = NULL;
            count += 1;
        }
    }


    *arrSize -= 1;

    free(arr);
    return(newArr);
}

static CalEvent **expandCalEventArray(CalEvent ** arr, CalEvent *toAdd, int *arrSize)
{
    CalEvent ** newArr;
    CalEvent *cpy;

    newArr = malloc(sizeof(CalEvent*)*((*arrSize) +1));
    assert(newArr != NULL);

    for (int i = 0; i < *arrSize; i++)
    {

        cpy = InitializeCalEvent();

        if(arr[i]->dateStart != NULL)
        {
            cpy->dateStart = malloc(sizeof(struct tm));
            assert(cpy->dateStart != NULL);
            *(cpy->dateStart) = *(arr[i]->dateStart);
            free(arr[i]->dateStart);
        }
        if (arr[i]->summary != NULL)
        {
            cpy->summary = malloc(sizeof(char)*strlen(arr[i]->summary) + 1);
            assert(cpy->summary != NULL);
            strcpy(cpy->summary,arr[i]->summary);
            free(arr[i]->summary);
        }
        if (arr[i]->location != NULL)
        {
            cpy->location = malloc(sizeof(char)*strlen(arr[i]->location) + 1);
            assert(cpy->location != NULL);
            strcpy(cpy->location,arr[i]->location);
            free(arr[i]->location);
        }
        if (arr[i]->org != NULL)
        {
            cpy->org = InitializeCalOrganizer();
            if (arr[i]->org->name != NULL)
            {   
                cpy->org->name = malloc(sizeof(char)*strlen(arr[i]->org->name) + 1);
                strcpy(cpy->org->name,arr[i]->org->name);
            }
            if (arr[i]->org->contact != NULL)
            {
                cpy->org->contact = malloc(sizeof(char)*strlen(arr[i]->org->contact) + 1);
                strcpy(cpy->org->name,arr[i]->org->contact);
            }
            free(arr[i]->summary);
        }
        
        newArr[i] = cpy;
        free(arr[i]);
        cpy = NULL;
    }

    cpy = InitializeCalEvent();
    if(toAdd->dateStart != NULL)
    {
        cpy->dateStart = malloc(sizeof(struct tm));
        assert(cpy != NULL);
        *(cpy->dateStart) =*(toAdd->dateStart);
    }

    if (toAdd->summary != NULL)
    {
        cpy->summary = malloc(sizeof(char)*strlen(toAdd->summary) + 1);
        assert(cpy != NULL);
        strcpy(cpy->summary,toAdd->summary);
    }
    newArr[(*arrSize)] = cpy;

    *arrSize += 1;
    free(arr);

    return(newArr);
}
static CalStatus writeInfo(FILE *const file, CalStatus stat, CalInfo info)
{
    char t_from[100];
    char t_to[100];

    /* x  lines
       x component(s): y event(s), z todo(s), q other(s)
       x subcomponent(s)
       x properties
       from yyyy-Mmm-dd to yyyy-Mmm-dd STILL NEED
       Organizer(s) STILL NEED
       Comman Name
       Comman Name
    */

    //Lines
    if(writeStringPlur(file,info.lines,"line\n\0","lines\n\0") == 0)

    {
        stat.code = IOERR;
        return(stat);
    }
    stat.linefrom += 1;
    updateLines(&stat);

    // x components: y events, z todos, q others:
    if(writeStringPlur(file,info.comps,"component: \0","components: \0") == 0 ||
       writeStringPlur(file,info.nCompEvents,"event, \0","events, \0") == 0 ||
       writeStringPlur(file,info.todos,"todo, \0","todos, \0") == 0 ||
       writeStringPlur(file,info.other,"other\n\0","others\n\0") == 0)
    {
        stat.code = IOERR;
        return(stat);
    }

    stat.linefrom += 1;
    updateLines(&stat);

    //x subcomponents
    if(writeStringPlur(file,info.subcomps,"subcomponent\n\0","subcomponents\n\0") == 0)

    {
        stat.code = IOERR;
        return(stat);
    }
    stat.linefrom += 1;
    updateLines(&stat);

    //x properties
    if(writeStringPlur(file,info.props,"property\n\0","properties\n\0") == 0)
    {
        stat.code = IOERR;
        return(stat);
    }
    stat.linefrom += 1;
    updateLines(&stat);

    //time
    if(info.early == NULL || info.late == NULL)
    {
        if(fprintf(file, "No dates\n") < 0)
        {
            stat.code = IOERR;
            return(stat);
        }
    }
    else
    {
        strftime(t_from,100,"%Y-%b-%d",info.early);
        strftime(t_to,100,"%Y-%b-%d",info.late);
        if(fprintf(file, "From %s to %s\n",t_from,t_to) < 0)
        {
            stat.code = IOERR;
            return(stat);
        }   
    }

    stat.linefrom += 1;
    updateLines(&stat);

    //Organizers
    if(info.norgs <= 0)
    {
        if(fprintf(file, "No organizers\n") < 0)
        {
            stat.code = IOERR;
            return(stat);
        }

        stat.linefrom += 1;
        updateLines(&stat);
    }
    else 
    {
        //num Organizers
        if(fprintf(file, "Organizers:\n") < 0)
        {
            stat.code = IOERR;
            return(stat);
        }

        stat.linefrom += 1;
        updateLines(&stat);

        //The organizers themselves
        for (int i = 0; i < info.norgs; i++)
        {
            if(fprintf(file, "%s\n",info.orgs[i] ) < 0)
            {
                stat.code = IOERR;
                return(stat);
            }
            stat.linefrom += 1;
            updateLines(&stat);
        }

    }



    return(stat);
}
static int compareString (const void* str1, const void* str2)
{
    int cmp, loopLen, len1, len2;
    char *s1,*s2;
    char c1,c2;


    s1 = toUpper(*(char**)str1);
    s2 = toUpper(*(char**)str2);
    cmp = 0;

    len1 = strlen(s1);
    len2 = strlen(s2);
 
    //loop through the smaller length times
    loopLen = len1;
    if (loopLen > len2)
    {
        loopLen = len2;   
    }

    //compare each character
    for (int i = 0; i < loopLen; i++)
    {
        c1 = s1[i];
        c2 = s2[i];

        if (c1 < c2)
        {
            cmp = -1;
            break;
        }
        else if(c1 > c2)
        {
            cmp = 1;
            break;
        }
    }
    free(s1);
    free(s2);
    //if all the letters are the same so far, the smaller string comes first
    if (cmp == 0)
    {
        if (len1 < len2)
        {
            cmp = -1;
        }
        else if(len1 > len2)
        {
            cmp = 1;
        } 
    }
    return(cmp);


}
static int compareDate (const void* date1, const void* date2)
{
    //Cal Event **
    CalEvent *event1,*event2;
    time_t t1,t2;
    event1 = *(CalEvent**)date1;
    event2 = *(CalEvent**)date2;

    if (event1 == NULL)
    {
        t1 = 0;
    }
    else
    {
        t1 = mktime(event1->dateStart);
    }
    if (event2 == NULL)
    {
        t1 = 0;
    }
    else
    {
        t2 = mktime(event2->dateStart);  
    }

    if (t1 < t2)
    {
        return(-1);
    }
    else if (t1 == t2)
    {
        return(0);
    }
    else
    {
        return(1);
    }



}

static int writeStringPlur(FILE *const file, int num, char *singular, char *plural)
{

    if (num == 1)
    {
        if(fprintf(file, "%d %s",num,singular) < 0)
        {
            return(0);
        }
    }
    else
    {
        if(fprintf(file, "%d %s",num,plural) < 0)
        {
            return(0);
        } 
    }

    return(1);
}


static int filterSubComp(CalComp *comp, CalOpt opt,time_t datefrom,time_t dateto)
{
    CalProp *tempProp;
    char *name, *propName;
    struct tm tempTm;
    time_t t;

    int toRemove = 1;
    //recursivly call subcomponents
    for (int i = 0; i < comp->ncomps; i++)
    {
        name = comp->comp[i]->name;
        if(strcmp(name,"VEVENT") == 0 && opt == OEVENT)
        {
            toRemove = filterSubComp(comp->comp[i],opt,datefrom,dateto);
        }
        else if (strcmp(name,"VTODO") == 0 && opt == OTODO)
        {
            toRemove = filterSubComp(comp->comp[i],opt,datefrom,dateto);
        }
        //if you dont need to filter, send value back up the recursive calls
        if (toRemove == 0)
        {
            return(toRemove);
        }
    }
           
    tempProp = comp->prop;
    while (tempProp != NULL)
    {
        propName = tempProp->name;
        //Find Times. If they fall in range of datefrom to dateto, do not filter
        if (strcmp(propName,"DTSTART") == 0 || strcmp(propName,"DTEND") == 0 ||
            strcmp(propName,"DUE") == 0 || strcmp(propName,"COMPLETED") == 0)
        {
            strptime(tempProp->value,"%Y%m%dT%H%M%S",&tempTm);
            tempTm.tm_isdst = -1;

            t = mktime(&tempTm);
            
            if (datefrom == 0 && dateto == 0)
            {
                toRemove = 0;
                return(toRemove);
            }
            else
            {
                //Don't check dateto if not set
                if (dateto == 0 && t >= datefrom)
                {
                    toRemove = 0;
                    return(toRemove);   
                }
                //check between both dates
                else if( t >= datefrom && t <= dateto)
                {
                    toRemove = 0;
                    return(toRemove);
                }
            }
        }
        tempProp = tempProp->next;
    }
    return(toRemove);
}
static CalStatus writeExtractedKind(FILE *file,CalInfo info,CalOpt kind)
{
    char t[100];
    CalStatus stat;

    stat = InitializeCalStatus();

    //Prind Events 
    if (kind == OEVENT)
    {
        //time
        if(info.events != NULL)
        {
            for (int i = 0; i < info.nevents; i++)
            {

                strftime(t,100,"%Y-%b-%d %l:%M %p",info.events[i]->dateStart);
                if (info.events[i]->summary == NULL)
                {
                    if(fprintf(file, "%s: (na)\n",t) < 0)
                    {
                        stat.code = IOERR;
                        return(stat);
                    }  
                }
                else
                {
                    if(fprintf(file, "%s: %s\n",t,info.events[i]->summary) < 0)
                    {
                        stat.code = IOERR;
                        return(stat);
                    }  
                }
                stat.linefrom += 1;
                updateLines(&stat);
            }
        }
    }

    //Print X-Props
    else if(kind == OPROP)
    {
        for (int i = 0; i < info.nxprops; i++)
        {
            if(fprintf(file, "%s\n",info.xprops[i]) < 0)
            {
                stat.code = IOERR;
                return(stat);
            }  
            stat.linefrom += 1;
            updateLines(&stat);
        }

    }
    return(stat);
}
CalStatus calInfo( const CalComp *comp, int lines, FILE *const txtfile )
{
    CalStatus stat;
    CalInfo info;
    int curStrIndex, *remPos;
    int remCount;
    info = InitializeCalInfo();
    stat = InitializeCalStatus();

    findCalNumbers(comp,&info);
    findEarlyAndLateTimes(comp,&info);
    findOrganizers(comp,&info);
    info.lines = lines;

    // Sort and Remove Duplicate organizers
    if(info.norgs > 0)
    {
        qsort(info.orgs,info.norgs,sizeof(char*),&compareString);
        curStrIndex = 0;
        remPos = malloc(sizeof(int)*info.norgs);
        assert(remPos != NULL);
        remCount = 0;
        for(int i = 1; i < info.norgs; i++)
        {
            if (strcmp(info.orgs[curStrIndex],info.orgs[i]) != 0)
            {
                curStrIndex = i;
            }
            else
            {
                remPos[remCount] = i;
                remCount += 1;
            }
        }
        for(int i = remCount-1; i >= 0; i--)
        {
            info.orgs = removeFromStringArray(info.orgs,remPos[i],&(info.norgs));
        }
        free(remPos);
    }


    stat = writeInfo(txtfile,stat,info);

    freeCalInfo(&info);

    return(stat);
}

CalStatus calExtract( const CalComp *comp, CalOpt kind, FILE *const txtfile )
{
    CalStatus stat;
    CalInfo info;
    int *remPos;
    int remCount, curStrIndex;

    curStrIndex = 0;
    info = InitializeCalInfo();
    stat = InitializeCalStatus();

    if (kind == OEVENT)
    {
        findEvents(comp,&info);
        qsort(info.events,info.nevents,sizeof(CalEvent*),&compareDate);
    }

    else if (kind == OPROP)
    {
        findXprops(comp,&info);
        //Sort and Remove Duplicate xprops
        if(info.nxprops > 0)
        {
            qsort(info.xprops,info.nxprops,sizeof(char*),&compareString);
            curStrIndex = 0;
            remPos = malloc(sizeof(int)*info.nxprops);
            assert(remPos != NULL);
            remCount = 0;
            for(int i = 1; i < info.nxprops; i++)
            {
                if (strcmp(info.xprops[curStrIndex],info.xprops[i]) != 0)
                {
                    curStrIndex = i;
                }
                else
                {
                    remPos[remCount] = i;
                    remCount += 1;
                }
            }
            for(int i = remCount-1; i >= 0; i--)
            {
                info.xprops = removeFromStringArray(info.xprops,remPos[i],&(info.nxprops));
            }
            free(remPos);
        }
    }
    stat = writeExtractedKind(txtfile,info,kind);


    freeCalInfo(&info);

    return(stat);
}
CalStatus calFilter( const CalComp *comp, CalOpt content, time_t datefrom, time_t dateto, FILE *const icsfile )
{
    CalStatus stat;
    CalComp *filComp;

    stat = InitializeCalStatus();

    filComp = malloc(sizeof(CalComp)+ sizeof(CalComp)*(comp->ncomps));
    assert(filComp != NULL);

    //Make a shallow Copy of comp
    filComp->name = comp->name;
    filComp->nprops = comp->nprops;
    filComp->prop = comp->prop;
    filComp->ncomps = comp->ncomps;

    for (int i = 0; i < comp->ncomps; i++)
    {
        filComp->comp[i] = comp->comp[i];
    }
    

    filter(filComp,content,datefrom,dateto);

    filComp = realloc(filComp, sizeof(CalComp)+ sizeof(CalComp)*(comp->ncomps));
    assert(filComp != NULL);

    if(filComp->ncomps == 0)
    {
        stat.code = NOCAL;
        free(filComp); 
        return(stat);
    }

    stat = writeCalComp(icsfile,filComp);
    free(filComp);

    return(stat);
}
CalStatus calCombine( const CalComp *comp1, const CalComp *comp2, FILE *const icsfile )
{
    CalStatus stat;
    CalComp *newComp;
    CalProp *c2Prodid, *c2Version, *propEnd,*curProp, *prevProp;
    int pos, prodIdPos, verPos;

    pos = 0;
    prodIdPos = 0;
    verPos = 0;

    stat = InitializeCalStatus();

    newComp = malloc(sizeof(CalComp)+ sizeof(CalComp)*((comp1->ncomps)+(comp2->ncomps)));
    assert(newComp != NULL);

    //Make a shallow Copy of comp
    newComp->name = comp1->name;
    newComp->nprops = comp1->nprops;
    newComp->prop = comp1->prop;
    newComp->ncomps = comp1->ncomps + comp2->ncomps;

    for (int i = 0; i < comp1->ncomps; i++)
    {
        newComp->comp[i] = comp1->comp[i];
    }
    for (int i = 0; i < comp2->ncomps; i++)
    {
        newComp->comp[i+comp1->ncomps] = comp2->comp[i]; 
    }

    //Find the end of newComps Property list
    curProp = newComp->prop;
    while (curProp != NULL && curProp->next != NULL)
    {
        curProp = curProp->next;
    }
    propEnd = curProp;

    //Unlink comp2s ProdID and Version from new comp's properties    
    propEnd->next = comp2->prop;
    prevProp = propEnd;
    curProp = comp2->prop;
    while (curProp != NULL)
    {
        pos += 1;
        if (strcmp(curProp->name,"PRODID") == 0)
        {
            prodIdPos = pos - 1;
            c2Prodid = curProp;
            prevProp->next = curProp->next;
            curProp = curProp->next;
            c2Prodid->next = NULL;
            continue;
        }
        else if (strcmp(curProp->name,"VERSION") == 0)
        {
            verPos = pos - 1;
            c2Version = curProp;
            prevProp->next = curProp->next;
            curProp = curProp->next;
            c2Version->next = NULL;
            continue;
        }
        prevProp = curProp;
        curProp = curProp->next;
    }

    //Add properties 



    stat = writeCalComp(icsfile,newComp);

    propEnd->next = NULL;
    //reLink Comp2
    pos = 1;
    curProp = comp2->prop;
    while (curProp != NULL)
    {
        if (pos == verPos)
        {
            c2Version->next = curProp->next;
            curProp->next = c2Version;
        }
        else if (pos == prodIdPos)
        {
            c2Prodid->next = curProp->next;
            curProp->next = c2Prodid;
        }
        pos += 1;
        curProp = curProp->next;
    }

    free(newComp);
    return(stat);
}

void printCalError(CalStatus stat)
{

    //Error on one line
    if (stat.linefrom == stat.lineto)
    {
        fprintf(stderr,"Error on line %d.\n",stat.lineto);
    }
    //Error spanning multipal lines
    else
    {
        fprintf(stderr,"Error from lines %d-%d.\n",stat.linefrom,stat.lineto);
    }
    //error message
    switch (stat.code)
    {
        case OK:
            fprintf(stderr,"No error occured.\n");
            break;
        case AFTEND:
            fprintf(stderr,"Text Found after the end of calendar.\n");
            break;
        case BADVER:
            fprintf(stderr,"Version of ics file is not '%s' or is missing.\n",VCAL_VER);
            break;
        case BEGEND:
            fprintf(stderr,"'BEGIN' or 'END' block not found as expected.\n");
            break;
        case IOERR:
            fprintf(stderr,"Error writing to file.\n");
            break;
        case NOCAL:
            fprintf(stderr,"No 'VCALENDAR' component or no V components found.\n");
            break;
        case NOCRNL:
            fprintf(stderr,"End of line characters ('\\r''\\n') missing at end of line.\n");
            break;
        case NODATA:
            fprintf(stderr,"No data found between component.\n");
            break;
        case NOPROD:
            fprintf(stderr,"PRODID is missing.\n");
            break;
        case SUBCOM:
            fprintf(stderr,"Subcomponent is not allowed.\n");
            break;
        case SYNTAX:
            fprintf(stderr,"Syntax error\n");
            break;
        default:
            fprintf(stderr,"Unknown Error Occured.\n");
    }
}
CalEvent *extractEvent(CalComp const *comp)
{
    CalProp *tempProp;
    CalEvent *cEvent;
    char *propName;

    cEvent = NULL;

    cEvent = InitializeCalEvent();
    //Loop Through props`
    tempProp = comp->prop;
    while (tempProp != NULL)
    {
        propName = tempProp->name;
        //Store startdate in CalEvent
        if (strcmp(propName,"DTSTART") == 0 && cEvent->dateStart == NULL)
        {
            cEvent->dateStart = malloc(sizeof(struct tm));
            assert(cEvent->dateStart!=NULL);
            strptime(tempProp->value,"%Y%m%dT%H%M%S",cEvent->dateStart);
            cEvent->dateStart->tm_isdst = -1; 
        } 

        //Store Summary in CalEvent
        else if (strcmp(propName,"SUMMARY") == 0 && cEvent->summary == NULL)
        {
            cEvent->summary = malloc(sizeof(char)*strlen(tempProp->value)+1);
            assert(cEvent->summary!=NULL);
            strcpy(cEvent->summary,tempProp->value);
        }
        else if (strcmp(propName,"LOCATION") == 0 && cEvent->location == NULL)
        {
            cEvent->location = malloc(sizeof(char)*strlen(tempProp->value)+1);
            assert(cEvent->location!=NULL);
            strcpy(cEvent->location,tempProp->value);
        }
        else if (strcmp(propName,"ORGANIZER") == 0 && cEvent->org == NULL)
        {
            cEvent->org = InitializeCalOrganizer();
            populateOrganizer(tempProp,cEvent->org);
        }

    tempProp = tempProp->next;
    }
    if (cEvent->dateStart != NULL)
    {
        return(cEvent);
    }
    else
    {
        freeCalEvent(cEvent);
        return(NULL);
    }

}
CalTodo *extractTodo(CalComp const *comp)
{
    CalProp *tempProp;
    CalTodo *cTodo;
    char *propName;

    cTodo = NULL;

    cTodo = InitializeCalTodo();

    //Loop Through props`
    tempProp = comp->prop;
    while (tempProp != NULL)
    {
        propName = tempProp->name;
        if(strcmp(propName,"SUMMARY") == 0 && cTodo->summary == NULL)
        {
            cTodo->summary = malloc(sizeof(char)*strlen(tempProp->value)+1);
            assert(cTodo->summary!=NULL);
            strcpy(cTodo->summary,tempProp->value);
        }
        else if(strcmp(propName,"PRIORITY") == 0 && cTodo->priority == 0)
        {
            cTodo->priority = malloc(sizeof(char)*strlen(tempProp->value)+1);
            assert(cTodo->priority!=NULL);
            strcpy(cTodo->priority,tempProp->value);
        }
        else if (strcmp(propName,"ORGANIZER") == 0 && cTodo->org == NULL)
        {
            cTodo->org = InitializeCalOrganizer();
            populateOrganizer(tempProp,cTodo->org);
        }
        tempProp = tempProp->next;
    }

    return(cTodo);
    
}

static void filter(CalComp *comp, CalOpt opt,time_t datefrom,time_t dateto)
{
    CalComp *swap;
    char* name;
    int *remPos;
    int remCount;
    int toRemove;
    remCount = 0;

    remPos = malloc(sizeof(int)*comp->ncomps);
    assert(remPos != NULL);

    for (int i = 0; i < comp->ncomps; i++)
    {
        toRemove = 0;
        name = comp->comp[i]->name;
        if(strcmp(name,"VEVENT") == 0 && opt == OEVENT)
        {
            toRemove = filterSubComp(comp->comp[i],opt,datefrom,dateto);
        }
        else if (strcmp(name,"VTODO") == 0 && opt == OTODO)
        {
            toRemove = filterSubComp(comp->comp[i],opt,datefrom,dateto);
        }
        else
        {
            toRemove = 1;
        }
        if (toRemove == 1)
        {
            remPos[remCount] = i;
            remCount += 1;
        }
    }
    //Set remove positions to NULL
    for (int i = 0; i < remCount; i++)
    {
        comp->comp[remPos[i]] = NULL;
    }
    //Shift elements to far left positions .
    for (int i = 0; i < comp->ncomps; i++)
    {
        if (comp->comp[i] == NULL)
        {
            for (int j = i; j < comp->ncomps; j++)
            {
                if (comp->comp[j] != NULL)
                {
                    swap = comp->comp[j];
                    comp->comp[j] = NULL;
                    comp->comp[i] = swap;
                    break;
                }
            }
        }
    }
    comp->ncomps -= remCount;
    free(remPos);

}

void populateOrganizer (CalProp *orgProp, CalOrganizer *org)
{
    CalParam *tempParam;
    char *paramName;
    tempParam = NULL;
    
    for (int i = 0; i < orgProp->nparams; i++)
    {
        tempParam = orgProp->param;
        paramName = tempParam->name;

        for (int j = 0; j < tempParam->nvalues; j++)
        {
            // Store common name
            if (strcmp(paramName,"CN") == 0)
            {
                org->name = malloc(sizeof(char)* strlen(tempParam->value[j]) + 1);
                assert(org->name != NULL);
                strcpy(org->name,tempParam->value[j]);
            }
        }
    }
    org->contact = malloc(sizeof(char)* strlen(orgProp->value) + 1);
    assert(org->contact != NULL);
    strcpy(org->contact,orgProp->value);

}