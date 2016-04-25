/* calutil.c
*
* Last updated: 10:30pm Apr. 7, 2016
*
* Nikolas Orkic 0854791
* E-mail: norkic@mail.uoguelph.ca
*
*
*  These function are designed to read ics files, and store their data 
*  in CalComp, CalProp, and CalUtil data structures as defined in calutil.h.
*
********************************************************************************************/


#include "calutil.h"

/*FreeCalParams
*
* Purpose: to free any allocated memory stoerd in a CalParam.
*
* Arguments: A pointer to a CalParameter (CalParam*)
*
* PostCondtions: all memory stroed in the CalParam is freed
********************************************************************************************/
static void freeCalParams(CalParam *param)
{
    if (param == NULL)
    {
        return;
    }
    if (param->next != NULL)
    {
        freeCalParams(param->next);
    }
    free(param->name);
    param->name = NULL;

    for (int i = 0; i < param->nvalues; i++)
    {
        free(param->value[i]);
        param->value[i] = NULL;
    }
    free(param);
    param = NULL;
    
}

/*FreeCalProps
*
* Purpose: to free any allocated memory stoerd in a CalProp.
*
* Arguments: A pointer to a CalProperty (CalProp*)
*
* PostCondtions: all memory stroed in the CalProp is freed
********************************************************************************************/
static void freeCalProps(CalProp *prop)
{
    if(prop == NULL)
    {
        return;
    }
    if (prop->next != NULL)
    {
        freeCalProps(prop->next);
    }
    free(prop->name);
    free(prop->value);
    prop->name = NULL;
    prop->value = NULL;

    if (prop->nparams > 0)
    {
        freeCalParams(prop->param);
    }

    free(prop);
    prop = NULL;
}

/*FreeCalComps
*
* Purpose: to free any allocated memory stoerd in a CalComp.
*
* Arguments: A pointer to a CalComponent (CalComp*)
*
* PostCondtions: all memory stroed in the CalComp is freed
********************************************************************************************/
static void freeCalComps(CalComp *comp)
{
    free(comp->name);
    comp->name = NULL;
    if (comp->nprops > 0)
    {
        freeCalProps(comp->prop);
    }

    for (int i = 0; i < comp->ncomps; i++)
    {
        freeCalComps(comp->comp[i]);
        
    }
    free(comp);
    comp = NULL;

}

/*expandString
*
* Purpose: To dynamically increase the size of desired string by concatinating another string to it
*
* Preconditions: addition is a null terminated String and is not NULL.

* Arguments: A pointer to the string wished to be expaned (char **) and a pointer to the string 
*            that it will be concatenated with (char *)
*
* Postconditions: The string will be concatinated with the addition string and will be the size of the added string
*                  plus it's old size;
********************************************************************************************/
static void expandString(char **const string, char *addition)
{
    if (*string == NULL)
    {
        *string = malloc(sizeof(char)*strlen(addition) + 1);
        assert(*string != NULL);

        strcpy(*string, addition);
    }
    else
    {
        *string = realloc(*string,strlen(*string) + strlen(addition) + 2);
        assert(*string != NULL);

        strcat(*string,addition);
    }
}
/*isEmpty
*
* Purpose: to determine is a string is made up of only Blank characters
*
* Arguments: A pointer to a string (char*)
*
* Returns: 1: if the string was made up of only white space or tabs
*           0: if any other character was found
********************************************************************************************/
static int isEmpty(char *string)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] != ' ' && string[i] != '\t')
        {
            return(0);
        }    
    }
    return(1);
}


/*EndOfFileHandle
*
* Purpose: To expand a dynamic buffer based off a static buffer 
*          or it will free and assign the dynamic buffer a status of NULL.
*          Called when the EOF character is found while reading in lines from readCalLine.
* Arguments: A pointer to the address of a dynamic string buffer (char **), a CalStatus, A pointer to 
*            the static buffer (char*) and the amount of the static buffer used (int);
* Returns: The CalStatus with potentially updated lines 
********************************************************************************************/
static CalStatus EndOfFileHandle(char **const pbuff, CalStatus stat, int *buffUsed, char buffer[READ_BUFF_SIZE])
{
    if (pbuff == NULL)
    {
        return(stat);
    }

    //If EOF and nothing is in the buffer
    else if (*buffUsed == 0)
    {
        free(*pbuff);
        *pbuff = NULL;
        updateLines(&stat);
    }

    // If something is in the buffer
    else if (!isEmpty(buffer))
    {
        buffer[*buffUsed] = '\0';
        expandString(pbuff,buffer);
        *buffUsed = 0;
    }
    return (stat);
}
/*VersionIdCheck
*
* Purpose: To look through a CalComp Scture and ensure it only has
*          One version number (that is the same as VCAL_VER ) and only one 
*          Prodid.
* Arguments: A pointer to the address of a CalComp(CalComp **) and a CalStatus.
*
* PostConditions: Status will contain the proper status as described in the specification.
********************************************************************************************/
static CalStatus VersionIDCheck(CalComp **pcomp, CalStatus stat)
{
    int verCount = 0;
    int idCount = 0;
    CalProp *propNode = NULL;


    propNode = (*pcomp)->prop;
    while (propNode != NULL)
    {
         if(strcmp(propNode->name,"VERSION") == 0)
        {
            //Version numbers don't match: ERROR
            if (strcmp(propNode->value,VCAL_VER) != 0)
            {
                stat.code = BADVER;
                return(stat);
            }
            else
            {
                verCount += 1;
            }

        }
        else if(strcmp(propNode->name,"PRODID") == 0)
        {
            idCount += 1;
        }
        propNode = propNode->next;
    }

    //i.e. no version or than one version
    if (verCount != 1)
    {
        stat.code = BADVER;
    }
    //i.e no ID or more than one ID
    else if(idCount != 1)
    {
        stat.code = NOPROD;
    }

    return(stat);
}

/*createParamLine
*
* Purpose: To create a string containg the information in a CalParam in the
*          format: ';name=value1,value2,value3'
* PreConditions: The ics files already open.
*
* Arguments: - The address to a CalParam containg information (calParam*).
*
* Returns:   - An allocated string containg the CalParam's information
*              in the RFC format.
*
*
********************************************************************************************/
static char *createParamLine(CalParam *param)
{

    /*NOTE: what if something is NULL/empty?*/
    char *paramLine, *value;

    paramLine = NULL;
    //FORMAT: ';name=value1,value2,value3'

    expandString(&paramLine,";\0");
    //Add name to paramLine
    expandString(&paramLine,param->name);

    expandString(&paramLine,"=\0");

    //paramLine is ';name='
    for (int i = 0; i < param->nvalues; i++)
    {
        value = param->value[i];
        expandString(&paramLine,value);

        if(i != param->nvalues - 1)
        {
            expandString(&paramLine,",\0");
        }      
    }
    return(paramLine);
}

/*createPropertyLine
*
* Purpose: To create a string containg the information in a CalProp variable to be written 
*          in the format: 'propName:propValue' or 
*          'propName;paramName=paramValue1,paramValue2;paramName=paramValue:PropValue'
*
* PreConditions: The ics files already open.
*
* Arguments: - The address to a CalProperty containg information (calProp*).
*
* Returns:   - An allocated string containg the CalProperty's information
*              in the RFC format.
*
********************************************************************************************/  

static char *createPropertyLine(CalProp *prop)
{
    CalParam *writeParam;
    char *propLine, *paramLine;

    propLine = NULL;
    paramLine = NULL;
    writeParam = NULL;

    //NAME:VALUE or
    //NAME; pName=pValue,pValue,Pvalue; pName=pValue:Value

    //name
    expandString(&propLine,prop->name);
    
    //Parameters
    writeParam = prop->param;
    while (writeParam != NULL)
    {
        paramLine = createParamLine(writeParam);

        //Concatinate parameter line to propline
        expandString(&propLine,paramLine);
        free(paramLine);
        paramLine = NULL;

        writeParam = writeParam->next;
    }
    //No parameters
    if (paramLine != NULL)
    {
        expandString(&propLine,paramLine);
        free(paramLine);
        paramLine = NULL;
    }
    expandString(&propLine,":\0");
    expandString(&propLine,prop->value);
    
    free(paramLine);
    return(propLine);
    
}
/*writeLine
*
* Purpose: To write a string to an ics file. Will write the string to the file
*          in multipal lines (if necessary) no more than FOLD_LEN characters.
*          All lines written after the frist line will begin with a space.
*
* PreConditions: The ics files already open.
*
* Arguments: - A pointer to and ics file (FILE*).
*            - A String (char *).
*
* Returns:   - A CalStatus variable that contains the number of lines written in its 
*              lineto and linefrom variables
*
* PostConditions: - The string will be written to the file and a CalStatus var
********************************************************************************************/
static void writeLine(FILE *const ics, char *string, CalStatus *stat)
{
    int writeCount;
    int len;

    if (string == NULL || ics == NULL || stat == NULL)
    {
        return;
    }

    writeCount = 0;
    len = strlen(string);

    for(int i = 0; i < len; i++)
    {
        //ERROR on EOF
        if(fputc(string[i],ics) == EOF)
        {
            updateLines(stat);
            stat->code = IOERR;
            return;
        }
        writeCount += 1;

        //Fold line
        if (writeCount == FOLD_LEN)
        {
            //Write EOL chars
            //ERROR on EOF
            if(fputc('\r',ics) == EOF || fputc('\n',ics) == EOF)
            {
                updateLines(stat);
                stat->code = IOERR;
                return;
            }

            stat->linefrom += 1;
            fflush(ics);
            writeCount = 0;
            
            //If there is at least 1 more character to write
            if (i != len-1)
            {
                //ERROR on EOF
                if (fputc(' ',ics) == EOF)
                {
                    updateLines(stat);
                    stat->code = IOERR;
                    return;
                }
                writeCount += 1;
            }
        }
    }
    if (writeCount != 0)
    {
        //Write EOL chars and ERROR on EOF
        if(fputc('\r',ics) == EOF || fputc('\n',ics) == EOF)
        {
            updateLines(stat);
            stat->code = IOERR;
            return;
        }

        stat->linefrom += 1;
        fflush(ics);
    }
    updateLines(stat);

}

CalStatus readCalFile( FILE *const ics, CalComp **const pcomp )
{

    CalStatus stat;
    int vComponent = 0;
    char *string;
    string = NULL;
    
    stat = readCalLine(NULL,NULL);

    *pcomp =InitializeCalComp();

    //Read in the CalFile
    stat = readCalComp(ics,pcomp);

    if (stat.code != OK)
    {
        freeCalComp(*pcomp);
        return(stat);
    }

    // Check for the letter V in component's name
    for (int i = 0; i < (*pcomp)->ncomps; i++)
    {

        if((*pcomp)->comp[i]->name[0] == 'V')
        {
            vComponent = 1;
            break;
        }

    }
    //No components with the letter V and/or no components exist
    if (vComponent != 1)
    {
        stat.code = NOCAL;
        freeCalComp(*pcomp);
        return (stat);
    }

    stat = VersionIDCheck(pcomp,stat);
    if (stat.code != OK)
    {
        freeCalComp(*pcomp);
        return(stat);
    }

    //Check if there is something past 'END: VCALENDAR'
    stat = readCalLine(ics,&string);
    if (string != NULL)
    {
        stat.code = AFTEND;
        free(string);
        freeCalComp(*pcomp);
        return(stat);
    }
    return(stat);
}

CalStatus readCalComp( FILE *const ics, CalComp **const pcomp )
{

    static int depth = 0; //depth become 0 at run time
    CalStatus stat;
    char *propLine, *upperName, *upperValue;
    CalProp *property;
    CalComp *newCalComp;

    propLine = NULL;
    property = NULL;
    
    //Read Line
    stat = readCalLine(ics,&propLine);

    if (stat.code != OK)
    {
        free(propLine);
        return(stat);
    }
    while(propLine != NULL)
    {
        
        property = InitializeCalProp();

        //Parse Line

        stat.code = parseCalProp(propLine,property);

        free(propLine);
        propLine = NULL;
        if (stat.code != OK)
        {
            freeCalProps(property);
            depth = 0;
            return(stat);
        }

        //Malloc is inside toUpper()
        upperName = toUpper(property->name);
        upperValue = toUpper(property->value);

        if (strcmp(upperName,"BEGIN") == 0)
        {

            if(depth != 0)
            {
                //Exceeds Nesting
                if (depth == 3)
                {
                    stat.code = SUBCOM;
                    free(upperName);
                    free(upperValue);
                    freeCalProps(property);
                    depth = 0;
                    return(stat);
                }
                depth += 1;

                //Create a new comp and give it an UPPERCASE value
                newCalComp = InitializeCalComp();

                //memory is malloced in toUpper
                newCalComp->name = upperValue;
                upperValue = NULL;

                free(upperName);
                free(upperValue);
                freeCalProps(property);
             
                stat = readCalComp(ics,&newCalComp);

                expandCalComp(pcomp,newCalComp);
                if (stat.code != OK)
                {
                    depth = 0;
                    return(stat);
                }

            }
            //If the first card is being parsed
            else if ((*pcomp)->name == NULL && strcmp(upperValue,"VCALENDAR") == 0)
            {
                (*pcomp)->name = upperValue;
                upperValue = NULL;
                depth = 1;
                free(upperName);
                free(upperValue);
                freeCalProps(property);
            }
            else
            {
                stat.code = NOCAL;
                free(upperName);
                free(upperValue);
                freeCalProps(property);
                depth = 0;
                return(stat);
            }
        }
        else if(strcmp(upperName,"END") == 0)
        {

            //Error if NoData
            if ((*pcomp)->nprops == 0 && (*pcomp)->ncomps == 0)
            {
                stat.code = NODATA;
            }
            //Error if BEGIN:x Does not match with END:x
            else if (strcmp(upperValue,(*pcomp)->name) != 0)
            {
                stat.code = BEGEND;
            }

            free(upperName);
            free(upperValue);
            freeCalProps(property);
            
            if (stat.code == OK)
            {
                depth -= 1;             
            }

            return(stat);
        }
        //Regular property
        else
        {
            free(upperName);
            free(upperValue);
            //Name should only be NULL if BEGIN:VACALENDAR is expected
            if ((*pcomp)->name == NULL)
            {
                stat.code = NOCAL;
                freeCalProps(property);
                depth = 0;
                return(stat);
            }
            insertProperty(pcomp,property);
        }
        
        //Read Line
        stat = readCalLine(ics,&propLine);
        if (stat.code != OK)
        {
            free(propLine);
            depth = 0;
            return(stat);
        }
    }
    if (depth != 0)
    {
        stat.code = BEGEND;
    }
    return(stat);
}

CalStatus readCalLine( FILE *const ics, char **const pbuff )
{
    static char buffer[READ_BUFF_SIZE];
    static int endR = 0;
    static int buffUsed = 0;
    
    static CalStatus stat;
    
    int lineEnd = 0;
    int folded = 0;
    int incremented = 0;
    char charIn;

    if (ics == NULL)
    {
        endR = 0;
        buffUsed = 0;
        stat = InitializeCalStatus();
        return(stat);
    }

    *pbuff = NULL;

    updateLines(&stat);
    while (lineEnd == 0)
    {
         
        charIn = fgetc(ics);
        //End of File
        if (charIn == EOF)
        {
            stat = EndOfFileHandle(pbuff,stat,&buffUsed,buffer);
            return(stat);
        }

        //Increment lines at the start when we know the file is not empty
        if (incremented == 0)
        {
            stat.linefrom += 1;
            stat.lineto += 1;
            incremented = 1;
        }

        //if not an ending character, add it to the buffer
        if (charIn != '\r' && charIn != '\n')
        {
            buffer[buffUsed] = charIn;
            buffUsed += 1;

            //copy buffer to pbuff if it is full
            if (buffUsed == READ_BUFF_SIZE-1)
            {
                buffer[buffUsed] = '\0';
                expandString(pbuff,buffer);
                buffUsed = 0;
            }
            
        }
        //Set flag to mark that '\r' has occured once
        else if(charIn == '\r' && endR == 0)
        {
            endR = 1;
        }
        else
        {
            //Line is over, copy buffer into pbuff
            if (charIn == '\n' && endR == 1)
            {
                buffer[buffUsed] = '\0';
                expandString(pbuff,buffer);
                lineEnd = 1;

                buffUsed = 0;
                endR = 0;

                //Read in next char to check for folding
                charIn = fgetc(ics);

                //don't want to store EOF in buffer
                //EOF
                if (charIn == EOF)
                {
                    stat = EndOfFileHandle(NULL,stat,&buffUsed,buffer);
                    return(stat);
                }
                //Folding required
                if (charIn == ' ' || charIn == '\t')
                {
                    stat.lineto += 1;
                    lineEnd = 0;
                    folded = 1;
                }
                //Flag the CR if it appears
                else if (charIn == '\r')
                {
                    endR = 1;
                }

                //New line would be occuring before a CR, Violates the spec
                else if (charIn == '\n')
                {
                    free(*pbuff);
                    *pbuff = NULL;
                    stat.linefrom += 1;
                    stat.lineto  += 1;
                    stat.code = NOCRNL;
                    return(stat);
                }
                else
                {
                    buffer[buffUsed] = charIn;
                    buffUsed += 1;
                }
                if (isEmpty(*pbuff))
                {
                    if(!folded)
                    {
                        stat.linefrom += 1;
                        stat.lineto += 1;
                    }
                    else
                    {
                        stat.lineto += 1;

                    }
                    lineEnd = 0;
                }
            } 
            //Occurs if 2 CR occur in a row or a NL occure before a CR
            else
            {
                free(*pbuff);
                *pbuff = NULL;
                stat.code = NOCRNL;
                return(stat);
            }
        }
    }
    return (stat);
}

CalError parseCalProp( char *const buff, CalProp *const prop )
{
    CalParam *newParam = NULL; // new Paramerter to allocate
    char *upperStr;    //To store an uppercase string
    char buffer[PARSE_BUFF_SIZE];   //static buffer
    int buffUsed = 0;  //amount of static buffer used
    char *info = NULL; //dynamic buffer
    char *pValue = NULL; //used to store Parameter Value
    char ch;             //used to store a character from a buffer
    int ignore = 0;      //Boolean flag to ignore special charaters that occur betwwen quataion marks when parsing
    int semiPos = -1;  //Stores the location of the first semicolen that appears in "buff". -1 if non occurs
    int colPos = -1;   //Stores the location of the first colen that appears in  "buff". -1 if non occurs
    int namePos = 0;   //Stores the smaller of the semiPos or colPos variables. used to find the name of the prop
    int paramName = 0; //Boolean flag signaling if a parameter has recieved its name.

    int len;
    len = strlen(buff);

    //Find first semicolen and colen
    for (int i = 0; i < len; i++)
    {
        ch = buff[i];
        //ignore between the quotes
        if (ch == '"')
        {
            if (ignore == 1)
            {
                ignore = 0;    
            }
            else
            {
                ignore = 1;
            }
        }
        
        if (ignore == 0)
        {
            //Find the first colen
            if (ch == ':' && colPos == -1)
            {
                colPos = i;
            }
            //find the first semicolen
            else if (ch == ';' && semiPos == -1)
            {
                semiPos = i;
            }
        }
    }
    //Each property must have a value and therefore a colen. error if not
    if (colPos == -1)
    {
        return(SYNTAX);
    }
    //Namepos become the small of semiPos or colenPos
    if (colPos < semiPos || semiPos == -1)
    {
        namePos = colPos;
    }
    else
    {
        namePos = semiPos;
    }
    
    //store the property name as UPPERCASE
    prop->name = malloc(sizeof(char)*namePos+1);
    assert(prop->name != NULL);

    strncpy(prop->name,buff,namePos);
    prop->name[namePos] = '\0';
    
    upperStr = toUpper(prop->name);
    free(prop->name);
    prop->name = upperStr;
    upperStr = NULL;

    //Empty name violates spec
    if (strlen(prop->name) == 0)
    {
        return(SYNTAX);
    }
    
    //Find the length of the property's value and store it

    prop->value = malloc(sizeof(char)*(len-colPos)+1);
    assert(prop->value != NULL);
    strncpy(prop->value, buff+colPos+1,len-colPos);
 
    //If there was a semicolen, look for parameters
    if (semiPos != -1)
    {
        ignore = 0;
        for (int i = semiPos+1; i < colPos+1; i++)
        {
            pValue = NULL;
            ch = buff[i];
            if(ch == '"')
            {
                if (ignore == 1)
                {
                    ignore = 0;
                }
                else
                {
                    ignore = 1;
                }
            }

            if(ignore == 0)
            {
                if (ch == ';' || ch == ',' || ch == '=' || ch == ':')
                {
                    //Flush buffer
                    buffer[buffUsed] = '\0';
                    expandString(&info,buffer);
                    buffUsed = 0;

                    //propName
                    if(ch == '=')
                    {
                        //There is already a name for the parameter or it doesn't have a name
                        if (paramName == 1 || isEmpty(info))
                        {
                            return(SYNTAX);
                        }

                        newParam = InitializeCalParam();

                        //param Name needs to be uppercase
                        //Memory is malloced in toUpper;
                        newParam->name = toUpper(info);

                        paramName = 1;
                        free(info);
                        info = NULL;
                        continue;
                    }
                    //Parameter Value: needs to be  regular
                    if (ch == ',' || ch == ';' || ch == ':')
                    {
                        if (isEmpty(info))
                        {
                            expandString(&info,"\0");
                        }
                        pValue = malloc(sizeof(char)*strlen(info)+1);
                        assert(pValue != NULL);

                        strcpy(pValue,info);
                        expandCalParam(&newParam,pValue);
                        free(info);
                        info = NULL;
                        //New Parameter on semicolen
                        if (ch == ';' || ch == ':')
                        {
                            insertParam(prop,newParam);
                            paramName = 0;
                        }
                        continue;
                    }
                }
            }
            //Add to buffer
            buffer[buffUsed] = ch;
            buffUsed += 1;

            //flush buffer
            if (buffUsed == PARSE_BUFF_SIZE - 1)
            {
                buffer[buffUsed] = '\0';
                expandString(&info,buffer);
                buffUsed = 0;
            }
        }
    }
    return(OK);

}

CalStatus writeCalComp( FILE *const ics, const CalComp *comp )
{
    CalStatus stat, subStat;
    CalProp *writeProp;

    char *toWrite;

    toWrite = NULL;
    writeProp = NULL;
    stat = InitializeCalStatus();

    // 1. OutPut BEGIN
    expandString(&toWrite,"BEGIN:\0");
    expandString(&toWrite,comp->name);
    
    writeLine(ics,toWrite,&stat);

    free(toWrite);
    toWrite = NULL;

    if (stat.code != OK)
    {
        return(stat);
    }

    // 2. Add Properties
    writeProp = comp->prop;
    while(writeProp != NULL)
    {
        toWrite = createPropertyLine(writeProp);
        writeLine(ics,toWrite,&stat);

        free(toWrite);
        toWrite = NULL;

        if (stat.code != OK)
        {
            return(stat);
        }

        writeProp = writeProp->next;
    }

    // 3. add subComponents
    
    for (int i = 0; i < comp->ncomps; i++)
    {
    
        //recursive call
        subStat = writeCalComp(ics,comp->comp[i]);

        //update stat with number of successful lines written
        stat.code = subStat.code;
        stat.linefrom += subStat.linefrom;

        updateLines(&stat);
        if (stat.code != OK)
        {
            return(stat);
        }
    }

    // 4. Add END
    expandString(&toWrite,"END:\0");
    expandString(&toWrite,comp->name);

    writeLine(ics,toWrite,&stat);

    free(toWrite);
    toWrite = NULL;
    // NOTE return IOERROR
    return(stat);

}

void freeCalComp( CalComp *const comp )
{
    freeCalComps(comp);
}

CalComp *InitializeCalComp()
{
    CalComp *newCalComp;

    newCalComp = malloc(sizeof(CalComp));
    assert(newCalComp != NULL);

    newCalComp->name = NULL;
    newCalComp->nprops = 0;
    newCalComp->prop = NULL;
    newCalComp->ncomps = 0;

    return(newCalComp);
}

CalProp *InitializeCalProp()
{
    CalProp *newCalProp;

    newCalProp = malloc(sizeof(CalProp));
    assert(newCalProp != NULL);

    newCalProp->name = NULL;
    newCalProp->value = NULL;
    newCalProp->nparams = 0;
    newCalProp->param = NULL;
    newCalProp->next = NULL;

    return(newCalProp);
}

CalParam *InitializeCalParam()
{
    CalParam *newCalParam;

    newCalParam = malloc(sizeof(CalParam));
    assert(newCalParam != NULL);

    newCalParam->name = NULL;
    newCalParam->next = NULL;
    newCalParam->nvalues = 0;
    return(newCalParam);
}

CalStatus InitializeCalStatus()
{
    CalStatus status;

    status.code = OK;
    status.lineto = 0;
    status.linefrom = 0;

    return(status);
}

void insertProperty(CalComp **const pcomp, CalProp *toAdd)
{
    CalProp *tmpPtr;
    tmpPtr = (*pcomp)->prop;
    if ((*pcomp)->nprops == 0)
    {
        (*pcomp)->prop = toAdd;
    }
    else
    {
        while(tmpPtr->next != NULL)
        {
            tmpPtr = tmpPtr->next;
        }
        tmpPtr->next = toAdd;
    }
    (*pcomp)->nprops += 1;

}

void insertParam(CalProp *prop,CalParam *toAdd)
{
    CalParam *tmpPtr;

    tmpPtr = prop->param;
    if (prop->nparams == 0)
    {
        prop->param = toAdd;
    }
    else
    {
        while (tmpPtr->next != NULL)
        {
            tmpPtr = tmpPtr->next;
        }
        tmpPtr->next = toAdd;
    }
    prop->nparams += 1;

}

void expandCalComp(CalComp **const toExpand, CalComp *toAdd)
{

    *toExpand = realloc(*toExpand, sizeof(CalComp)+(sizeof(CalComp*)*(1+(*toExpand)->ncomps)));
    assert(*toExpand != NULL);

    (*toExpand)->comp[(*toExpand)->ncomps] = toAdd;
    (*toExpand)->ncomps += 1;
}

void expandCalParam(CalParam **const toExpand, char *toAdd)
{
    *toExpand = realloc(*toExpand, sizeof(CalParam)+ (sizeof(char*)*(1+(*toExpand)->nvalues)));
    assert(*toExpand != NULL);

    (*toExpand)->value[(*toExpand)->nvalues] = toAdd;
    (*toExpand)->nvalues += 1;
}

void updateLines(CalStatus *status)
{
    if (status->lineto > status->linefrom)
    {
        status->linefrom = status->lineto;
    }
    else
    {
        status->lineto = status->linefrom;
    }
}

char* toUpper(char *string)
{
    char *upperStr, ch;

    if (string == NULL)
    {
        return(NULL);
    }

    upperStr = malloc(sizeof(char)*strlen(string)+1);
    assert(upperStr != NULL);

    strcpy(upperStr,string);

    for (int i = 0; i < strlen(upperStr); i++)
    {
        ch = upperStr[i];

        if (ch >= 'a' && ch <= 'z')
        {
            ch -= 32;
            upperStr[i] = ch;
        }
    }
    return(upperStr);
}
