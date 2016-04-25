/********
* calModule.c --  Wrapper functions for calutil library python to c
*
* Last updated: 10:29pm Apr. 7, 2016
*
* Nikolas Orkic 0854791
* E-mail: norkic@mail.uoguelph.ca
*
********/
#include <string.h>
#include <stdio.h>
#include "calutil.h"
#include "caltool.h"
#include <Python.h>


/*Cal_readFile
*
* Purpose: A wrapper function for Calutil's readCalFile function. Reads an ICS file into a CalComp,
*          and store it, and information about each of it's sub-components in a python list. 
*
* Arguments: - The name of an ics file (string)
*            - The address of a python list (list) 
*
* Returns:   - "OK" (as a python string) on read success
*            - A string indicating the type of error and where it occured on read Fail
*
********************************************************************************************/
static PyObject *Cal_readFile(PyObject *self, PyObject *args);

/*Cal_freeFile
*
* Purpose: A wrapper function for Calutil's freeCalComp function. Frees a provided CalComp structure
*
* Arguments: - The address of a open CalComp structure (python int)
*
* Retuns:    - 1 on success
*            - 0 on fail
*
********************************************************************************************/
static PyObject *Cal_freeFile(PyObject *self, PyObject *args);

/*Cal_writeFile
*
* Purpose: A wrapper function for Calutil's writeCalComp function. Writes specified open CalComp Structure's subcompenents
*          to a given file
*
* Arguments:   - The name of a file to write to (python string)
*              - The address of a open CalComp structure (python int)
*              - A list the length of the CalComp's number of sub-components which specifies which
*                subcomponents to write to the file by having a '1' stored at the index of the requested
*                subcomponent number e.x. [0,1,1,0] specifies to write subcompents 1 and 2 to the file. (python list)
*
* Returns:     - The string "OK'num' lines written" on success where num is the number of lines written
*              - A description of an error with the number of lines written on fail
*
********************************************************************************************/
static PyObject *Cal_writeFile(PyObject *self, PyObject *args);

/*createShallow
*
* Purpose: To create a shallow copy of a component with specified subcomponents that are a subset
*          of it's orignal components
*
* Arguments:   - The address of the initial subcomponent (CalComp*)
*              - The address of array of integers that specify which subcomponents to copy (int *)
*              - The length of the above integer array (int)
*
* Returns:     - A shallow copy of the orignal CalComp with a subset of its orignal subcompents
*
********************************************************************************************/
static CalComp *createShallow(CalComp *pCal, int *indexes, int nIndexes);

/*getCalError
*
* Purpose:  To obtain an error string describing a calError.
*
* Arguments:   - A CalStatus indicating the error (CalStatus)
*              - The address of a string to store the error string (char*)
*
********************************************************************************************/
static void getCalError(CalStatus stat, char *errorStore);

/*** method list to export to python ***/
static PyMethodDef CalMethods[] = {
    // {"Python_func_name", c function, Argument style}
    {"readFile", Cal_readFile, METH_VARARGS, "Reads iCalendar 2.0 File"},
    {"writeFile", Cal_writeFile, METH_VARARGS, "Writes the current file"},
    {"freeFile", Cal_freeFile, METH_VARARGS, "Frees memory from a CalComp populated to result of readFile"},
    {NULL, NULL, 0, NULL}, //denotes end of list
};

static struct PyModuleDef calModuleDef = {
    PyModuleDef_HEAD_INIT,
    "cal", //enable "import Cal"
    NULL, //omit module documentation
    -1, //don't reinitialize the module
    CalMethods /*link module name "Cal" to methods table*/ 
};

PyMODINIT_FUNC PyInit_cal(void)
{
    return PyModule_Create(&calModuleDef);
}

static PyObject *Cal_readFile(PyObject *self, PyObject *args)
{
    //args: 
    //char *filename,
    //PyObject *result,
    //PyArg_ParseTuple(args,"sO",&filename, &result)  

    FILE *fh;

    CalComp *pCal;     //Calcomp to populate
    CalProp *tempProp; //temp Pointer to look through property lsit
    CalEvent *cEvent; //To store Event information
    CalTodo *cTodo; //to Store Todo information
    CalStatus stat;    //To store the status of readCalfile

    char *fileName;    //fileName argument from function call
    PyObject *result;   // python list that will be populated

    //Objects used to construct final result/return value
    PyObject *listOfPrimTuples, *listOfSecTuples,*tempTuple, *noFileObj;

    //message for when file cannot open
    char *noFileMsg;
    char calErrBuff[50];
    char errMsgBuff[100];

    // information to place in a pyObject tuple
    char *name, *summary, *priority,*dtStart,*location, *orgName, *orgContact;
    char dateStart[100];
    int props,comps;

    pCal = NULL;
    tempProp = NULL;

    name = NULL;
    summary = NULL;
    dtStart = NULL;
    location = NULL;
    priority = NULL;
    orgName = NULL;
    orgContact = NULL;
    cEvent = NULL;
    cTodo = NULL;

    //parse function call args
    if (PyArg_ParseTuple(args, "sO", &fileName, &result))
    {
        fh = fopen(fileName,"r");
        //File Open failed: return error string
        if (fh == NULL)
        {
            noFileMsg = malloc(sizeof(char)*(strlen(fileName)+strlen(strerror(errno))+3));
            assert(noFileMsg != NULL);
            strcpy(noFileMsg,fileName);
            strcat(noFileMsg,": ");
            strcat(noFileMsg,strerror(errno));
            noFileObj = Py_BuildValue("s",noFileMsg);
            free(noFileMsg);
            return(noFileObj);
        }

        stat = readCalFile(fh,&pCal);
        fclose(fh);

        if (stat.code == OK)
        {

            listOfPrimTuples = Py_BuildValue("[]");
            listOfSecTuples = Py_BuildValue("[]");
            //loop Through top level components and creat tuples from
            //their infomation
            for (int i = 0; i < pCal->ncomps; i++)
            {
                name = NULL;
                summary = NULL;
                dtStart = NULL;
                priority = NULL;
                location = NULL;
                orgName = NULL;
                orgContact = NULL;
                name  = pCal->comp[i]->name;
                props = pCal->comp[i]->nprops;
                comps = pCal->comp[i]->ncomps;
                tempProp = pCal->comp[i]->prop;

                if (strcmp(name,"VEVENT") == 0)
                {
                    cEvent = InitializeCalEvent();
                    cEvent = extractEvent(pCal->comp[i]);
                    strftime(dateStart,100,"%F %H:%M:%S",cEvent->dateStart);
                    dtStart = dateStart;
                    location = cEvent->location;
                    summary = cEvent->summary;

                    if (summary == NULL)
                    {
                        summary = "";
                    }
                    if (cEvent->org != NULL)
                    {   
                        orgName = cEvent->org->name;
                        orgContact = cEvent->org->contact;
                    }
                }

                else if (strcmp(name,"VTODO") == 0)
                {
                    cTodo = InitializeCalTodo();
                    cTodo = extractTodo(pCal->comp[i]);
                    dtStart = NULL;
                    location = NULL;
                    summary = cTodo->summary;
                    priority = cTodo->priority;
                    if (summary == NULL)
                    {
                        summary = "";
                    }
                    if (cTodo->org != NULL)
                    {   
                        orgName = cTodo->org->name;
                        orgContact = cTodo->org->contact;
                    }
                }
                else
                {
                    //Looks for 'Summary' property and stores a pointer to it
                    for (int j = 0; j < pCal->comp[i]->nprops; j++)
                    {
                        if (strcmp(tempProp->name,"SUMMARY") == 0)
                        {
                            summary = tempProp->value;
                            break;
                        }
                        tempProp = tempProp->next;
                    }
                    if (summary == NULL)
                    {
                        summary = "";
                    }
                }
                //build a tuple of the comp's information and append it to the list of tuples
                tempTuple = Py_BuildValue("(siis)",name,props,comps,summary);
                PyList_Append(listOfPrimTuples, tempTuple);
                tempTuple = Py_BuildValue("(sssss)",dtStart,priority,location,orgName,orgContact);
                PyList_Append(listOfSecTuples, tempTuple);

            }

            //add CalComp pointer and  the list of informaion tuples the result 
            PyList_Append(result,Py_BuildValue("k",pCal));
            PyList_Append(result,listOfPrimTuples);
            PyList_Append(result,listOfSecTuples);
            if (cEvent != NULL)
            {
                freeCalEvent(cEvent);
                cEvent = NULL;
            }
            if (cTodo != NULL)
            {
                freeCalTodo(cTodo);
                cTodo = NULL;
            }
        }
        //Failed to read Cal file, return error
        else
        {
            getCalError(stat,calErrBuff);
            if (stat.lineto == stat.linefrom)
            {
                sprintf(errMsgBuff,"Error on line: %d\n%s",stat.lineto,calErrBuff);
            }
            else
            {
                sprintf(errMsgBuff,"Error from line %d to line %d\n%s",stat.linefrom,stat.lineto,calErrBuff);
            }
            return(Py_BuildValue("s",errMsgBuff));
        }
        //Success
        return (Py_BuildValue("s","OK\0"));
    }
    else
    {
        return(Py_BuildValue("s","Bad Args"));
    }

}

static PyObject *Cal_freeFile(PyObject *self, PyObject *args)
{
    CalComp *pCal;
    pCal = NULL;
    //parse function call args
    if (PyArg_ParseTuple(args, "k",(unsigned long*)&pCal))
    {
        freeCalComp(pCal);
        return(Py_BuildValue("i",1));
    }
    else
    {
        return(Py_BuildValue("i",0));
    }
}

static PyObject *Cal_writeFile(PyObject *self, PyObject *args)
{

    FILE *fh;

    CalComp *pCal, *toWrite;     //Calcomp to populate
    CalStatus stat;    //To store the status of readCalfile

    char *fileName;    //fileName argument from function call
    int *cPosToWrite, size; //int array to store indexes of which subcomponents to write and its size
    int tempInt;            //int to move python to c
    //Objects used to construct final result/return value
    PyObject *pyPosToWrite, *index, *noFileObj;

    //message for when file cannot open or error occurs
    char *noFileMsg;
    char buffer[200];
    char buff2[200];

    cPosToWrite = NULL;
    
    tempInt = 0;
    pCal = NULL;
    pyPosToWrite = NULL;
    stat = InitializeCalStatus();
    //parse function call args
    if (PyArg_ParseTuple(args, "skO", &fileName, (unsigned long*)&pCal, &pyPosToWrite))
    {
        size = PyList_Size(pyPosToWrite);

        cPosToWrite = malloc(sizeof(int)*size);
        assert(cPosToWrite != NULL);

        //convert int list from py to c
        for (int i = 0; i < size; i++)
        {
            index = PyList_GetItem(pyPosToWrite,i);
            PyArg_Parse(index,"i",&tempInt);

            cPosToWrite[i] = tempInt;
        }

        //build a shallow copy
        toWrite = createShallow(pCal,cPosToWrite,size);

        if (toWrite->ncomps == 0)
        {
            stat.code = NOCAL;
            getCalError(stat,buffer);
            sprintf(buff2,"\n%s%d Lines written.",buffer, stat.lineto);
            return(Py_BuildValue("s",&buff2));
        }

        fh = fopen(fileName,"w" );
        //File Open failed: return error string
        if (fh == NULL)
        {
            noFileMsg = malloc(sizeof(char)*(strlen(fileName)+strlen(strerror(errno))+3));
            assert(noFileMsg != NULL);
            strcpy(noFileMsg,fileName); 
            strcat(noFileMsg,": ");
            strcat(noFileMsg,strerror(errno));
            noFileObj = Py_BuildValue("s",noFileMsg);
            free(noFileMsg);
            free(cPosToWrite);
            return(noFileObj);
        }


        stat = writeCalComp(fh,toWrite);

        if (stat.code != OK)
        {
            getCalError(stat,buffer);
            sprintf(buffer,"\n%s%d Lines written.",buffer, stat.lineto);
            return(Py_BuildValue("s",&buffer));
        }

        free(toWrite);
        free(cPosToWrite);
        fclose(fh);
        
        if (stat.code != OK)
        {
            getCalError(stat,buffer);
            sprintf(buffer,"%s%d Lines written",buffer, stat.lineto);
            return(Py_BuildValue("s",&buffer));
        }
    }
    else
    {
        return(Py_BuildValue("s","Bad Args"));
    }
    sprintf(buffer,"OK%d Lines written",stat.lineto);
    return(Py_BuildValue("s",buffer));
}

static CalComp *createShallow(CalComp *Cal, int *indexes, int nIndexes)
{

    CalComp *shalCal;
    int sizeShallow;
    int countIn;
    sizeShallow = 0;
    countIn = 0;


   shalCal = NULL;

    for (int i = 0; i < nIndexes; i++)
    {
        if (indexes[i] == 1)
        {
            sizeShallow += 1;
        }
    }

    shalCal = malloc(sizeof(CalComp)+(sizeof(CalComp)*sizeShallow));
    assert(shalCal != NULL);

    //assign values to shallow copy
    shalCal->name = Cal->name;
    shalCal->nprops = Cal->nprops;
    shalCal->prop = Cal->prop;
    shalCal->ncomps = sizeShallow;

    //on '1' in indexes, assign value to shalCal
    for (int i = 0; i < nIndexes; i++)
    {
        //Not write pos on 1   
        if (indexes[i] == 1)
        {
            shalCal->comp[countIn] = Cal->comp[i];
            countIn += 1;
        }
    }

    return(shalCal);
}
static void getCalError(CalStatus stat, char *errorStore)
{
    char buffer[150];

    //error message
    switch (stat.code)
    {
        case OK:
            sprintf(buffer,"No error occured.");
            break;
        case AFTEND:
            sprintf(buffer,"Text Found after the end of calendar.");
            break;
        case BADVER:
            sprintf(buffer,"Version of ics file is not '%s' or is missing.",VCAL_VER);
            break;
        case BEGEND:
            sprintf(buffer,"'BEGIN' or 'END' block not found as expected.");
            break;
        case IOERR:
            sprintf(buffer,"Error writing to file.");
            break;
        case NOCAL:
            sprintf(buffer,"No 'VCALENDAR' component or no V components found.");
            break;
        case NOCRNL:
            sprintf(buffer,"End of line characters ('\\r''\\n') missing at end of line.");
            break;
        case NODATA:
            sprintf(buffer,"No data found between component.");
            break;
        case NOPROD:
            sprintf(buffer,"PRODID is missing.");
            break;
        case SUBCOM:
            sprintf(buffer,"Subcomponent is not allowed.");
            break;
        case SYNTAX:
            sprintf(buffer,"Syntax error");
            break;
        default:
            sprintf(buffer,"Unknown Error Occured.");
            break;
    }
    strcpy(errorStore,buffer);
}




