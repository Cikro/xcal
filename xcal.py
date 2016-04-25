#!/usr/bin/python3

#####################################################################
# xcal.py --  A graphical user interface for the calutil library
# 
# Last updated: 4:27am Apr. 4, 2016
#
# Nikolas Orkic 
# E-mail: norkic@mail.uoguelph.ca
##########################

from tkinter import *
from tkinter import messagebox
from tkinter import filedialog
from tkinter import ttk

import subprocess
import os
import sys
import getpass
import mysql.connector
from mysql.connector import errorcode

import cal
from calTree import calTree
from calFile import calFile

#'file' Menu functions
#------------------------------------------------------------------------------------------------------------------------------------------------

#####################################################################
# fileOpen
#
# Purpose:    Prompts the use to open an ics file, loads the ics file into a CalComp, displays its info
#             in the log panel, and displays its components on the file view panel tree
#
# Arguments:  The fileViewPanel tree (calTree)
#
##########################
def fileOpen(fvpTree):

    global curFilePath, curFileName,fileOptions, unsavedChanges
    newFilePath =()
    newCal = []
    visibleComps = []
    #1. Unsaved Changes
    if (unsavedChanges == 1):
        yN = messagebox.askyesnocancel(title = "Unsaved Changes",message = "You have Unsaved Changes. Would you like to save?")
        if(yN == None):
            return
        elif (yN == True):
            fileSave()

    
    #2. Open file
    fileOptions['title'] = 'Open'
    newFilePath = filedialog.askopenfilename(**fileOptions)
    if (len(newFilePath) != 0):

        newFileName = newFilePath[newFilePath.rfind('/')+1:]
        
        
        #3. Run Wrapper for readCalFile
        status = readAndDisplayComp(str(newFilePath),fvpTree)
        if (status == "OK"):
   
            callCalInfo(fvpTree.cal.pointer,fvpTree.cal.visibleComps)

            #Make menu options and fvp buttons clickable
            fileMenu.entryconfigure('Save',state = NORMAL)
            fileMenu.entryconfigure('Save as...',state = NORMAL)
            fileMenu.entryconfigure('Combine...',state = NORMAL)
            fileMenu.entryconfigure('Filter...',state = NORMAL)
            toDoMenu.entryconfigure('To-do List...',state = NORMAL)
            dbMenu.entryconfigure('Store All',state = NORMAL)

            exEvBut.configure(state = NORMAL)
            exXBut.configure(state = NORMAL)           
            showSelBut.configure(stat = DISABLED)

            #Change title bar
            curFilePath = newFilePath
            curFileName = newFileName
            updateTitlebar("xcal - "+curFileName)
        else:
            status = "Failed to open \""+newFileName+"\"\n" + status
            writeToTextLog(textLog,"&SEP&")
            writeToTextLog(textLog,status)

#####################################################################
# readAndDisplayComp
#
# Purpose: To read an ics file and store its information in a fvpTree object and display its
#          information on the file view panel tree. 
#
# Arguments: fileName - The path to the file to read from. (string) 
#            fvpTree  - The file View Panel tree (calTree)
#
##########################
def readAndDisplayComp(fileName,fvpTree):
    newCalData = []
    status = "OK"
    status = cal.readFile(fileName,newCalData)

    if (status == "OK"):

        newCal = calFile(newCalData[0],newCalData[1],newCalData[2])

        fvpTree.clear()
        fvpTree.addCal(newCal)
    return(status)

#####################################################################
# fileSavefvpTree
#
# Purpose: Saves the Calendar currently open on the tree to an ics file
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def fileSave(fvpTree):

    global curFilePath, curFileName, fileOptions,unsavedChanges

    result = cal.writeFile(str(curFilePath),fvpTree.cal.pointer, fvpTree.cal.visibleComps)
    if(result[0:2] == "OK"):
        result = "\""+ curFileName +"\""+ " Saved Successfully:\n\n" +result[2:]+"\n"
        unsavedChanges = 0

        fvpTree.cal.hideSubs()
        fvpTree.clear()
        fvpTree.addCal(fvpTree.cal)

        toDoMenu.entryconfigure('Undo...',state = DISABLED)
        updateTitlebar("xcal - "+str(curFileName))
    else:
        result = "\""+ curFileName +"\""+ " Save FAILED:\n" +result+"\n"


    writeToTextLog(textLog,"&SEP&")
    writeToTextLog(textLog,result);

#####################################################################
# fileSaveAs
#
# Purpose: prompts the user to choose a directory of where to save the currently
#          open Calendar and saves it in an ics file of that name
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def fileSaveAs(fvpTree):

    global curFilePath,curFileName,fileOptions

    newFilePath = ""

    #Choose file Name
    fileOptions['title'] = 'Save As'
    newFilePath = filedialog.asksaveasfilename(**fileOptions)

    #On file selected
    if (len(newFilePath) != 0):

        newFileName = newFilePath[newFilePath.rfind('/')+1:]
        result = cal.writeFile(str(newFilePath),fvpTree.cal.pointer, fvpTree.cal.visibleComps)

        #On success
        if(result[0:2] == "OK"):
            result = "\""+newFileName+"\"" + " Saved Successfully:\n" +result[2:]+"\n"
            unsavedChanges = 0

            #Clear the current tree
            fvpTree.cal.hideSubs()
            fvpTree.clear()

            #add the cal back to the tree
            fvpTree.addCal(fvpTree.cal)

            toDoMenu.entryconfigure('Undo...',state = DISABLED)
            #update title bar
            curFileName = newFileName
            curFilePath = newFilePath
            updateTitlebar("xcal - "+str(curFileName))
        else:
            result = "\""+ newFileName +"\""+ " Save FAILED:\n\n" +result+"\n"

        writeToTextLog(textLog,"&SEP&")
        writeToTextLog(textLog,result)

#####################################################################
# fileCombine
#
# Purpose: prompts the user to enter a file to combine with the current fvp, combines the
#          selected file with the fvp and displays the combined calcomp on the fvp.
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def fileCombine(fvpTree):
    global fileOptions, curFileName, unsavedChanges
    
    #1. Choose file Name somehow

    fileOptions['title'] = 'Combine'
    combine = filedialog.askopenfilename(**fileOptions)
    combineName = combine[combine.rfind('/')+1:]


    if (len(combine) != 0):
        #write temp file
        result = cal.writeFile("./tempIn.temp",fvpTree.cal.pointer, fvpTree.cal.visibleComps)

        #Print some error if intermediate writefile fails
        if (result[:2] != "OK"):
            writeToTextLog(textLog,"&SEP&")
            writeToTextLog(textLog,"Unexpected Combine Error.\n")
            writeToTextLog(textLog,'Combine "'+str(curFileName)+'" and "'+str(combineName)+'" failed.\n')
            return()


        args = ["./caltool","-combine",str(combine)]
        inFile = open("./tempIn.temp","r")
        outFile = open("./tempOut.temp","w")
        errFile = open("./tempErr.temp","w")

        #combine into new ics 'tempOut.temp'
        Pcaltool = subprocess.Popen(args,stdin = inFile, stdout = outFile ,stderr = errFile)
        Pcaltool.wait()
        inFile.close()
        outFile.close()
        errFile.close()

        #Read In errors from actual combine
        errFile = open("tempErr.temp","r")
        errors = errFile.read()
        errFile.close()

        #on no errors
        if (len(errors) == 0):

            #Read from tempOut.temp to store new CalComp
            status = readAndDisplayComp("./tempOut.temp",fvpTree)
            if (status == "OK"):
                writeToTextLog(textLog,"&SEP&")
                writeToTextLog(textLog,'Combine "'+str(curFileName)+'" and "'+str(combineName)+'" successful!\n')
                updateTitlebar("xcal- *"+curFileName)
                unsavedChanges = 1

        #write Errors
        else:
            writeToTextLog(textLog,"&SEP&")
            writeToTextLog(textLog,errors)
            writeToTextLog(textLog,'Error in "'+str(combineName)+'":\n')
            writeToTextLog(textLog,'Combine "'+str(curFileName)+'" and "'+str(combineName)+'" failed.\n')

        os.remove("tempIn.temp")
        os.remove("tempOut.temp")
        os.remove("tempErr.temp")

#####################################################################
# fileFilter
#
# Purpose: Filters the currently open CalComp to only either its events or its todo items
#          between a time period specified by the user. Upon successful filtering, the new filtered
#          component is displayed on the file view panel
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def fileFilter(fvpTree):

    global curFileName, unsavedChanges
    #POP UP WINDOW

    #store result (filter/cancel) button click
    confirmFilter = IntVar()
    confirmFilter.set(0)

    #the pop up window
    winFilter = Toplevel(root)
    winFilter.title("Filter")
    
    filterType = StringVar("")
    #radio buttons to select type of filter
    radioE = ttk.Radiobutton(winFilter, text = "Events", variable = filterType, value = "e", command = lambda: butFilter.configure(state = NORMAL), state = NORMAL)
    radioT = ttk.Radiobutton(winFilter, text = "To-dos", variable = filterType, value = "t", command = lambda: butFilter.configure(state = NORMAL), state = NORMAL)
    radioE.pack()
    radioT.pack()
    
    #Frame for textboxes that store 'date to ' and 'date from' strings
    frameFromTo = Frame(winFilter)
    dateFrom = StringVar()
    dateTo = StringVar()
    dateFrom.set("")
    dateTo.set("")
    
    entryFrom = ttk.Entry(frameFromTo, textvariable=dateFrom)
    entryTo = ttk.Entry(frameFromTo, textvariable=dateTo)

    #lables for the text boxes
    fromLabel = ttk.Label(frameFromTo, text='From: ')
    toLabel = ttk.Label(frameFromTo, text='To: ')
    fromLabel.grid(row = 0, column = 0)
    entryFrom.grid(row = 0, column = 1)
    toLabel.grid(row = 1, column = 0)
    entryTo.grid(row = 1, column = 1)
    frameFromTo.pack()

    #frame for and the actual filter/cancel buttons
    buttonFrame = Frame(winFilter)
    butFilter = Button(buttonFrame, text = "Filter", command = lambda: confirm(winFilter,confirmFilter), state = DISABLED)
    butCancel = Button(buttonFrame, text = "Cancel", command = lambda: cancel(winFilter,confirmFilter))
    butFilter.pack(side = LEFT)
    butCancel.pack(side = LEFT)

    buttonFrame.pack()

    #make modal and non-resizeable
    winFilter.resizable(FALSE,FALSE)
    winFilter.bind("<Escape>", lambda e: winFilter.destroy())
    winFilter.grab_set()
    winFilter.wait_window(winFilter)
    #End POPUPWINDOW-----------------------------------------------------

    #On Confirm
    if (confirmFilter.get() == 1):
        filtCalData = []
        filtIndexes = []

        #Run caltool with  'date to' and 'date from' args
        dateFromstr = str(dateFrom.get())
        dateTostr = str(dateTo.get())
        kindCode = str(filterType.get())
        args = ["./caltool","-filter",kindCode]

        if (not dateFromstr.isspace() and len(dateFromstr) != 0):
            args.append("from")
            args.append(dateFromstr)

        if (not dateTostr.isspace() and len(dateTostr) != 0):
            args.append("to")
            args.append(dateTostr)

        inFile = open(curFilePath,"r")
        outFile = open("./tempOut.temp","w")
        errFile = open("./tempErr.temp","w")

        Pcaltool = subprocess.Popen(args,stdin = inFile, stdout = outFile ,stderr = errFile)
        Pcaltool.wait()
        inFile.close()
        outFile.close()
        errFile.close()

        # read temp file for errors
        errFile = open("./tempErr.temp","r")
        errors = errFile.read()
        errFile.close()

        #If there are no errors
        if (len(errors) == 0):

            #read file and collect inforamtion in a list
            status = "OK"
            status = cal.readFile("./tempOut.temp",filtCalData)

            #if there are no errors in reading
            if (status == "OK"):
                
                #create new calFile object
                filtCal = calFile(filtCalData[0],filtCalData[1],filtCalData[2])


                #write Success
                writeToTextLog(textLog,"&SEP&")
                writeToTextLog(textLog,"Filter \""+ curFileName+"\""+ " successful!\n")

                #add the new items to tree
                fvpTree.clear()
                showSelBut.configure(state=DISABLED)

                fvpTree.addCal(filtCal)
                updateTitlebar("xcal- *"+curFileName)
                unsavedChanges = 1;

            #output error to textLog
            else:
                status = "Failed to filter \""+curFileName+"\"\n" + status
                writeToTextLog(textLog,status)
        #output error to text log
        else:
            if (kindCode == "e"):
                kind = "events"
            else:
                kind = "to-dos"
            writeToTextLog(textLog,"&SEP&")
            writeToTextLog(textLog,"No "+kind+" found\n")
            writeToTextLog(textLog,"Error filtering "+kind+":\n")
        
        os.remove("tempOut.temp")
        os.remove("tempErr.temp")

#####################################################################
# fileExit
#
# Purpose: To free any alocated Calendars, close the database connection and close the program on user prompt
#
# Arguments: cursor   - the cursor of the mysql connection object
#            cnx      - the mysql connection object
#            fvpTree  - The file View Panel tree (calTree)
#
##########################
def fileExit(cursor,cnx,fvpTree):

    #Ask if they are sure
    yN = messagebox.askyesno("Quit?",message = "Are you sure you want to Quit?")
    
    if(yN):
        fvpTree.removeCal()
        # cursor.close()
        # cnx.close()
        root.quit()

#'To-DO' Menu functions
#-----------------------------------------------------------------

#####################################################################
# toDoList
#
# Purpose:  To prompt the user to mark off completed 'to-do itemes'
#           and remove the cehck off items from the fvp
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def toDoList(fvpTree):

    #make a pop-up window
    toDoWindow = Toplevel(root, width = 125, height = 250)
    toDoWindow.title("To-Do")

    toDoFrame = Frame(toDoWindow, width = 100, height = 200)

    #set up items
    checkBoxStatus = []
    checkBoxes = []

    #textarea to store checkboxes and scroll bars
    todoScrollY = Scrollbar(toDoFrame, orient = "vertical")
    todoScrollX = Scrollbar(toDoFrame, orient = "horizontal")

    toDoText = Text(toDoFrame,height=10, width=50, yscrollcommand=todoScrollY.set, xscrollcommand=todoScrollX.set, background="gray85")

    todoScrollY['command'] = toDoText.yview
    todoScrollX['command'] = toDoText.xview
    todoScrollY.grid(row = 0, column = 1, sticky = "ns")
    todoScrollX.grid(row = 1, column = 0, sticky = "we")

    #vars for done and don and cancel buttons
    confirmTodo = IntVar()
    confirmTodo.set(0)

    #buttons to go at the bottom of the frame
    buttonFrame = Frame(toDoFrame)
    butDone = Button(buttonFrame, text = "Done",state = DISABLED, command = lambda: confirm(toDoWindow,confirmTodo))
    butCancel = Button(buttonFrame, text = "Cancel", command = lambda: cancel(toDoWindow,confirmTodo))
    butDone.pack(side=LEFT, pady = 5, padx = 5)
    butCancel.pack(side=LEFT, pady = 5, padx = 5)

    cBoxes = 0
    removedComps = 0
    for i in range (0,len(fvpTree.cal.primeData)):

        #if it is a todo and that todo is visible
        if(fvpTree.cal.primeData[i][0] == "VTODO" and fvpTree.cal.visibleComps[i] == 1):
            if (cBoxes != 0):
                toDoText.insert("end", "\n")
            
            #add to list of checkBox status'
            #list is of format : [[index,checked],[index,checked]...]
            checkBoxStatus.append([i,IntVar()])
            checkBoxStatus[cBoxes][1].set(1)
            #add checkbox to list of checkboxes
            checkBoxes.append(Checkbutton(toDoText, text = str(i+1-removedComps)+": "+str(fvpTree.cal.primeData[i][3]), variable=checkBoxStatus[cBoxes][1], onvalue=0, offvalue=1, command = lambda:checkedBox(checkBoxStatus,butDone)))
            toDoText.window_create("end", window=checkBoxes[cBoxes])

            cBoxes += 1
        #counter of comps that are 'hidden' from the fvp tree
        if (fvpTree.cal.visibleComps[i] == -1):
            removedComps += 1
    #no To do items
    if(cBoxes == 0):
        toDoText.insert("end","No to-do items found\n")



    toDoText.grid(row = 0, column = 0)
    toDoText.config(state = DISABLED)
    toDoFrame.pack(side = TOP, padx = 5, pady = 5)



    buttonFrame.grid(row = 2, column = 0)

    #make modal and non-resizeable
    toDoWindow.resizable(FALSE,FALSE)
    toDoWindow.bind("<Escape>",lambda e: toDoWindow.destroy())
    toDoWindow.grab_set()
    toDoWindow.wait_window(toDoWindow)

    if(confirmTodo.get() == 1):

        #the 'done' button can only be pressed if a checkbox is pressed
        #Adjust unsaved changes
        updateTitlebar("xcal- *"+curFileName)
        unsavedChanges = 1

        #Set visibilty flags to zero on the subcompinents they selected
        for i in range(0,len(checkBoxStatus)):
            fvpTree.cal.visibleComps[checkBoxStatus[i][0]] = checkBoxStatus[i][1].get()
        fvpTree.updateTreeItems()
        toDoMenu.entryconfigure('Undo...',state = NORMAL)

#####################################################################
# checkedBox
#
# Purpose:  To determine weather or not a checkbx is checked from a list of 
#           variables toggled by a group of checkboxes and enable/disable a 
#           button if there is/isn't a box checked
#
# Arguments: checkBoxStatus  - A list vairables affected by a group of checkboxes (List)
#            but             - A button to enable/disable based on the if there is a checkbox checked
#
##########################
def checkedBox(checkBoxStatus,but):

    for i in range(len(checkBoxStatus)):
        if (checkBoxStatus[i][1].get() == 0):
            but.config(state=NORMAL)
            return
    but.config(state=DISABLED)

#####################################################################
# toDoUndo
#
# Purpose:  To undo the changes since the last save that toDoList() caused 
#
# Arguments: fvpTree  - The file View Panel tree (calTree)
#
##########################
def toDoUndo(fvpTree):

    undoWindow = Toplevel(root, width = 125, height = 50)
    undoWindow.title = "Undo"

    ttk.Label(undoWindow,text = "Are you sure you want to undo? All.").pack(side=TOP, padx= 10)
    ttk.Label(undoWindow,text = "All to-do components since the last save will be restored.\n").pack(side=TOP, padx= 10)

    confirmUndo = IntVar()
    confirmUndo.set(0)

    buttonFrame = Frame(undoWindow)
    butUndo = Button(buttonFrame, text = "Undo", command = lambda: confirm(undoWindow,confirmUndo))
    butCancel = Button(buttonFrame, text = "Cancel", command = lambda: cancel(undoWindow,confirmUndo))
    butUndo.pack(side=LEFT, pady = 5, padx = 5)
    butCancel.pack(side=LEFT, pady = 5, padx = 5)
    buttonFrame.pack(side=TOP,padx = 10, pady = 10)

    undoWindow.resizable(FALSE,FALSE)
    undoWindow.bind("<Escape>",lambda e: undoWindow.destroy())
    undoWindow.grab_set()
    undoWindow.wait_window(undoWindow)


    if (confirmUndo.get() == 1):
        #Set all hidden to visable
        for i in range (0,len(fvpTree.cal.visibleComps)):
            if(fvpTree.cal.visibleComps[i] != -1):
                fvpTree.cal.visibleComps[i] = 1

        #make them visible
        fvpTree.updateTreeItems()

        toDoMenu.entryconfigure('Undo...',state = DISABLED)
        writeToTextLog(textLog,"&SEP&")
        writeToTextLog(textLog,"Undo successful\n")


#'Help' Menu functions
#-----------------------------------------------------------------

#####################################################################
# helpDateMsk
#
# Purpose: To prompt the user to select a DateMask file to use for the
#           environment variable DATEMSK.
#
##########################
def helpDateMsk():

    global datemask

    #Changes File Options
    fileOptions['title'] = 'DATEMASK'
    fileOptions['filetypes'] = [('DateMask','.*')]

    datemask = filedialog.askopenfilename(**fileOptions)

    #Set datemask if a file was chosen
    if (len(datemask) != 0):
        os.environ["DATEMSK"] = str(datemask)

    #Changes File Options BACK
    fileOptions['filetypes'] = [('iCalendar File','.ics')]

#####################################################################
# checkDateMsk
#
# Purpose: To Check if the DATEMSK environment variable is set and prompt 
#          the user if they would like to select one.
#
##########################
def checkDateMask():

    mess = "No datemask environt variable set. Would you like to set one now?"

    if (os.environ.get("DATEMSK") == None):
        yN = messagebox.askyesno("No DateMask",message = mess)
        if (yN):
            helpMenu.invoke(0)

#####################################################################
# helpAbout
#
# Purpose: To display information about the program and its author.
#
##########################
def helpAbout():

    winAbout = Toplevel(root)
    winAbout.title("About")
    ttk.Label(winAbout, text= 'About xcal').pack(side = TOP)
    ttk.Label(winAbout, text= '-----------------').pack(side = TOP)
    ttk.Label(winAbout,text = 'Author: Nikolas Orkic').pack(side = TOP)
    ttk.Label(winAbout,text = 'Compatability: iCalendar V2.0\n').pack(side = TOP)
    ttk.Label(winAbout,text = 'Xcal is an iCalendar file managment software that allows you to obtain information').pack(side = TOP)
    ttk.Label(winAbout,text = 'about iCalendar files, combine iCalendar files, filter files based on events or to-do').pack(side = TOP)
    ttk.Label(winAbout,text = 'items within a time period, and keep track of completed to-do items.\n').pack(side = TOP)
    Button(winAbout, text = "OK", command = lambda: winAbout.destroy()).pack(side=TOP)

    winAbout.resizable(FALSE,FALSE)
    winAbout.bind("<Escape>",lambda e: winAbout.destroy())

#End of Menu functions--------------------------------------------



#####################################################################
# dbStoreAll
#
# Purpose: To attempt to store every component on the fvp into the database
#
# Arguments: cursor   - The cursor of the mysql connection object
#            cnx      - The mysql connection object
#            fvpTree  - The file View Panel tree (calTree)
#
##########################
def dbStoreAll(cursor,cnx,fvpTree):
    
    #go thorugh the information of each cal
    for i in range(0,len(fvpTree.cal.primeData)):
        #if the are visible (on the fvcal), try to insert
        if (fvpTree.cal.visibleComps[i] == 1):
            dbStoreItem(cursor,cnx,i,fvpTree)
    dbStatus(cursor)
    writeToTextLog(textLog,"Store all completed:\n")

#####################################################################
# dbStoreSelected
#
# Purpose: To find the index of the item selected in the fvp tree and
#          attempt to add the item to the database
#
# Arguments: cursor   - the cursor of the mysql connection object
#            cnx      - the mysql connection object
#            fvpTree  - The file View Panel tree (calTree)
#
##########################
def dbStoreSelected(cursor,cnx,fvpTree):

    #get the selected item
    itemIndex = getSelectedIndex(fvpTree)
    #attempt to store in database
    result = dbStoreItem(cursor,cnx,itemIndex,fvpTree)
    #write status and output
    dbStatus(cursor)
    writeToTextLog(textLog, str(result+"\n"))

#####################################################################
# dbStoreItem
#
# Purpose: To attempt to store a VEVENT or VTODO and thier organzier (if any) in
#          the database
#
# Arguments: cursor    - the cursor of the mysql connection object
#            cnx       - the mysql connection object
#            itemIndex - The index of the item on the fvpCal
#            fvpTree   - The file View Panel tree (calTree)
#
# Returns: The result of the VEVENT or VTODO insert (String)
#
##########################
def dbStoreItem(cursor,cnx,itemIndex,fvpTree):

    primeInfo = fvpTree.cal.primeData[itemIndex]
    secInfo = fvpTree.cal.secData[itemIndex]

    #store required information
    name = sanatizeString(primeInfo[0])
    summary = sanatizeString(primeInfo[3])
    start = sanatizeString(secInfo[0])
    priority = sanatizeString(secInfo[1])
    location = sanatizeString(secInfo[2])
    orgName = sanatizeString(secInfo[3])
    orgContact = sanatizeString(secInfo[4])


    orgInDB = False 
    # if there is an organizer, check if it is in database already
    if (orgName != None):

        query = 'SELECT org_id,name FROM  ORGANIZER WHERE name ='
        query += "'"+orgName+"'"
        cursor.execute(query)

        for line in cursor:
            orgInDB = True

        # if Not in DB, add to db
        if (orgInDB == False):
            if (orgContact != None):
                # Insert into db
                query = "INSERT into ORGANIZER (name,contact) VALUES ("+"'"+orgName+"','"+orgContact+"');"
                cursor.execute(query)
                cnx.commit()
    else:
        orgName = ""
    # Check VEVENT OR VTODO
    itemInDB = False
    if (name == "VEVENT"):

        if (start == None):
            return("VEVENT insert failed; no start time:")
        if (len(summary) == 0):
            return("VEVENT insert failed; no summary:")

        #ensure Item is not already in database
        query = "SELECT summary,start_time FROM EVENT WHERE summary = "+"'"+summary+"' AND start_time = "+"'"+start+"'"
        cursor.execute(query)
        for line in cursor:
            itemInDB = True
            break

        if (not itemInDB):
            #create query and insert
            query = "INSERT INTO EVENT ("
            if (location != None):
                query = "INSERT INTO EVENT (summary,start_time,location,organizer) VALUES ("+"'"+summary+"',"+"'"+start+"',"+"'"+location+"',"
                query += "(SELECT org_id FROM  ORGANIZER WHERE name = "+"'"+orgName+"'))"
            else:
                query = "INSERT INTO EVENT (summary,start_time,organizer) VALUES ("+"'"+summary+"',"+"'"+start+"',"
                query += "(SELECT org_id FROM  ORGANIZER WHERE name = "+"'"+orgName+"'))"
            cursor.execute(query)
            cnx.commit()
        else:
            return("Event Already in dataBase:")
    elif (name == "VTODO"):

        if (len(summary) == 0):
            return("VTODO: Insert failed; no summary:")

        #ensure Item is not already in database
        query = "SELECT summary FROM TODO WHERE summary = "+"'"+summary+"'"
        cursor.execute(query)
        for line in cursor:
            itemInDB = True
            break
        if (not itemInDB):
            #create query and insert
            query = "INSERT INTO TODO ("
            if (priority != None):
                query = "INSERT INTO TODO (summary,priority,organizer) VALUES ("+"'"+summary+"',"+"'"+priority+"',"
                query += "(SELECT org_id FROM  ORGANIZER WHERE name = "+"'"+orgName+"'))"
            else:
                query = "INSERT INTO TODO (summary,organizer) VALUES ("+"'"+summary+"',"
                query += "(SELECT org_id FROM  ORGANIZER WHERE name = "+"'"+orgName+"'))"
            cursor.execute(query)
            cnx.commit()
        else:
            return("To-do already in database:")
    else:
        return("Insert failed; component not a VEVENT or VTODO:")
    return("Insert Successful:")
#####################################################################
# sanatizeString
#
# Purpose: To escape any characters that may cause an issue when trying to use them
#          in a dataBase Query
#
# Arguments: string - the string to be sanatized(String)
#
# Returns: A sanatized string
#
##########################
def sanatizeString(string):
    if (string == None):
        return(string)
    needEscape = ["'",'"',"_","\\"]
    sanatizedString = ""
    for i in range (0,len(string)):
        if(string[i] in needEscape):
            sanatizedString += "\\"
        sanatizedString += string[i]
    return(sanatizedString)

#####################################################################
# dbClear
#
# Purpose: Listens for the database->clear menu action and deletes all rows in the ORGANIZER,
#          EVENT, and TODO tables and print the status of the database afterward.
#
# Arguments: cursor    - the cursor of the mysql connection object
#            cnx       - the mysql connection object
#
##########################
def dbClear(cursor,cnx):
    dbDeleteAllRows(cursor,cnx)
    dbStatus(cursor)
    writeToTextLog(textLog, "Database clear successful:\n")

#####################################################################
# dbDeleteAllRows
#
# Purpose: To Delete the every row from ORGANIZER, EVENT, and TODO database tables
#
# Arguments: cursor    - the cursor of the mysql connection object
#            cnx       - the mysql connection object

#
##########################
def dbDeleteAllRows(cursor,cnx):

    query = "DELETE FROM ORGANIZER;"
    cursor.execute(query)
    query = "DELETE FROM EVENT;"
    cursor.execute(query)
    query = "DELETE FROM TODO;"
    cursor.execute(query)
    cnx.commit()

#####################################################################
# dbStatus
#
# Purpose: To listen for the database->status action, and output the status line 
#          on the log panel. Also disables or re-enables the database->clear menu option
#
# Arguments: cursor    - the cursor of the mysql connection object
#
##########################
def dbStatus(cursor):

    numOrgs,numEvent,numTodo = dbTablesLength(cursor)
    writeToTextLog(textLog, "&SEP&")
    writeToTextLog(textLog, str("Database has "+ str(numOrgs)+" organizers, "+ str(numEvent)+" events, "+str(numTodo)+" to-do items\n"))
    dbToggleClearOption(cursor)

#####################################################################
# dbTablesLength
#
# Purpose: To determine the length of each table in the database
#
# Arguments: cursor    - the cursor of the mysql connection object
#
# Returns: the length of the ORGANZIER, EVENT, and TODO tables
#
##########################
def dbTablesLength(cursor):
    query = "SELECT COUNT(org_id) FROM ORGANIZER"
    cursor.execute(query)
    for x in cursor:
        numOrgs = x[0]

    query = "SELECT COUNT(event_id) FROM EVENT"
    cursor.execute(query)
    for x in cursor:
        numEvent = x[0]

    query = "SELECT COUNT(todo_id) FROM TODO"
    cursor.execute(query)
    for x in cursor:
        numTodo = x[0]

    return(numOrgs,numEvent,numTodo)

#####################################################################
# dbToggleClearOption
#
# Purpose: Disables the database->clear option if there are no entries in
#          any of them. Enbles the option otherwise
#
# Arguments: cursor   - the cursor of the mysql connection object
#
#
#
##########################
def dbToggleClearOption(cursor):

    numOrgs,numEvent,numTodo = dbTablesLength(cursor)
    if (numOrgs == 0 and numEvent == 0 and numTodo == 0):
        dbMenu.entryconfigure("Clear",state = DISABLED)
    else:
        dbMenu.entryconfigure("Clear",state = NORMAL)

#####################################################################
# dbCreateTable
#
# Purpose: To attempt to store a VEVENT or VTODO and thier organzier (if any) in
#          the database
#
# Arguments: cursor    - the cursor of the mysql connection object
#            tableInfo - the description of the table (string)
#
# Returns: The status of the attempted table creation (string)
#
##########################
def dbCreateTable(cursor, tableInfo):

    try:    
        query = "CREATE TABLE "+tableInfo+";"
        cursor.execute(query)
        return("OK")
    except mysql.connector.Error as err:
        if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
            return("Table already exists.")
        else:
            return(err.msg)

#####################################################################
# dbCreateOrg
#
# Purpose: To create the Organizer Table
#
# Arguments: cursor    - the cursor of the mysql connection object
#
##########################
def dbCreateOrg(cursor):

    columns = []
    query = "ORGANIZER ("
    columns.append("org_id INT AUTO_INCREMENT,")
    columns.append("name VARCHAR(60) NOT NULL,")
    columns.append("contact VARCHAR(60) NOT NULL,")
    columns.append("PRIMARY KEY(org_id)")

    for i in range (len(columns)):
        query += columns[i]
    query+= ")"
    status = dbCreateTable(cursor, query)

#####################################################################
# dbCreateEvent
#
# Purpose: To create the Event Table
#
# Arguments: cursor    - the cursor of the mysql connection object
#
##########################
def dbCreateEvent(cursor):

    columns = []

    query = "EVENT ("
    columns.append("event_id INT AUTO_INCREMENT,")
    columns.append("summary VARCHAR (60) NOT NULL,")
    columns.append("start_time DATETIME NOT NULL,")
    columns.append("location VARCHAR (60),")
    columns.append("organizer INT,")
    columns.append("PRIMARY KEY(event_id),")
    columns.append("FOREIGN KEY(organizer) REFERENCES ORGANIZER(org_id) ON DELETE CASCADE")

    for i in range (len(columns)):
        query += columns[i]
    query+= ")"
    status = dbCreateTable(cursor, query)

#####################################################################
# dbCreateTodo
#
# Purpose: To create the Todo Table
#
# Arguments: cursor    - the cursor of the mysql connection object
#
##########################
def dbCreateTodo(cursor):

    columns = []

    query = "TODO ("
    columns.append("todo_id INT AUTO_INCREMENT,")
    columns.append("summary VARCHAR (60) NOT NULL,")
    columns.append("priority SMALLINT,")
    columns.append("organizer INT,")
    columns.append("PRIMARY KEY(todo_id),")
    columns.append("FOREIGN KEY(organizer) REFERENCES ORGANIZER(org_id) ON DELETE CASCADE")

    for i in range (len(columns)):
        query += columns[i]
    query+= ")"
    status = dbCreateTable(cursor, query)


#End of DataBase functions--------------------------------------------

#####################################################################
# ctrlLDownHandel
#
# Purpose: To handel the event of the left controlKey being pressed and set a global
#          variable indicating such.
#
##########################
def ctrlLDownHandel(event):

    global ctrlPressL
    ctrlPressL = 1

#####################################################################
# ctrlLUpHandel
#
# Purpose: To handel the event of the left controlKey being released and set a global
#          variable indicating such.
#
##########################
def ctrlLUpHandel(event):

    global ctrlPressL
    ctrlPressL = 0;

#####################################################################
# ctrlRDownHandel
#
# Purpose: To handel the event of the right controlKey being pressed and set a global
#          variable indicating such.
#
##########################
def ctrlRDownHandel(event):

    global ctrlPressR
    ctrlPressR = 1

#####################################################################
# ctrlRupHandel
#
# Purpose: To handel the event of the right controlKey being released and set a global
#          variable indicating such.
#
##########################
def ctrlRUpHandel(event):

    global ctrlPressR
    ctrlPressR = 0;

#Key Press Handlers, Used for menu Shotcuts--------------------------------------------------
#####################################################################
# oPress
#
# Purpose: To handel the event of the 'o' key being pressed and to run 
#          'fileOpen' if a control key is also being pressed.
#
##########################
def oPress(event):

    global ctrlPressR, ctrlPressL
    if (ctrlPressL  or ctrlPressR):
        fileMenu.invoke(0) #'click' file>open

#####################################################################
# sPress
#
# Purpose: To handel the event of the 's' key being pressed and to run 
#          'fileSave' if a control key is also being pressed.
#
##########################
def sPress(event):

    global ctrlPressR, ctrlPressL
    if (ctrlPressL  or ctrlPressR):
        fileMenu.invoke(1) #'click' file>save

#####################################################################
# xPress
#
# Purpose: To handel the event of the 'x' key being pressed and to run 
#          'fileExit' if a control key is also being pressed.
#
##########################
def xPress(event):

    global ctrlPressR, ctrlPressL
    if (ctrlPressL  or ctrlPressR):
        fileMenu.invoke(5) #'click' file>exit

#####################################################################
# tPress
#
# Purpose: To handel the event of the 't' key being pressed and to run 
#          'toDoList' if a control key is also being pressed.
#
##########################
def tPress(event):

    global ctrlPressR, ctrlPressL
    if (ctrlPressL  or ctrlPressR):
        toDoMenu.invoke(0) #'click' todo>todo List

#####################################################################
# zPress
#
# Purpose: To handel the event of the 'z' key being pressed and to run 
#          'toDoUndo' if a control key is also being pressed.
#
##########################
def zPress(event):

    global ctrlPressR, ctrlPressL
    if (ctrlPressL  or ctrlPressR):
        toDoMenu.invoke(1)  #'click' todo>undo
#End of Key Press Handlers------------------------------------------------------------



#####################################################################
# updateTitleBar
#
# Purpose:    To update the text in the title bar of the root window
#
# Arguments:  A string to display in the title bar
#
##########################
def updateTitlebar(title):
    root.title(title)

#####################################################################
# writeToTextLog
#
# Purpose:    To write text to the textLog. If the requested string is "&SEP&", 
#             a series of '-' will be printed instead
#
# Arguments:  textLog - A textLog to output to (Text)
#             msg     - A string write to the textLog 
#
##########################
def writeToTextLog(textLog, msg):   

    textLog.config(state = NORMAL)
    if (msg == "&SEP&"):
        textLog.insert(1.0,"----------------------------"+"\n")
    else:
        textLog.insert(1.0,msg)
    textLog.config(state = DISABLED) 

#####################################################################
# clearTextLog
#
# Purpose:    To clear all the information in the text log
#
# Arguments:  textLog - A textLog to output to (Text)
#
##########################
def clearTextLog(textLog):

    textLog.config(state = NORMAL)
    textLog.delete(1.0,END)
    textLog.config(state = DISABLED)


#####################################################################
# callCalInfo
#
# Purpose:    To Call 'calTool -info' on specific subcomponents
#
# Arguments:  comp         - The address of an allocated CalComp (int)
#             writeIndexes - A list indicating the subcomponents to have in the Calendar when
#                           'caltool' is called
#             
##########################
def callCalInfo(comp, writeIndexes):

    info = ""
    err = ""

    result = cal.writeFile("tempIn.temp",comp,writeIndexes)
    if (result[0:2] == "OK"):
    
        inFile = open("./tempIn.temp","r")
        outFile = open("./tempOut.temp","w")
        errFile = open("./tempErr.temp","w")
        args = ["./caltool","-info"]
        Pcaltool = subprocess.Popen(args,stdin = inFile, stdout = outFile ,stderr = errFile)

        #wait for the process to end
        Pcaltool.wait()
        inFile.close()
        outFile.close()
        errFile.close()


        errFile = open("./tempErr.temp","r")
        errors = errFile.read()
        errFile.close()

        writeToTextLog(textLog,"&SEP&")
        if (len(errors) == 0):
            inFile = open("./tempOut.temp","r")
            info = inFile.read()
            inFile.close()
            writeToTextLog(textLog,info)
        else:
            writeToTextLog(textLog,errors)

        os.remove("tempIn.temp")
        os.remove("tempOut.temp")
        os.remove("tempErr.temp")

#####################################################################
# getSelectedIndex
#
# Purpose:    To obtain the index of the item selected on the fvp
#
# Arguments:  fvpTree  - The file View Panel tree (calTree)
#             
##########################
def getSelectedIndex(fvpTree):

    global selectedItem

    for i in range(0,len(fvpTree.cal.visibleComps)):
        if (selectedItem == fvpTree.treeIds[i]):
            itemIndex = i;
            return(itemIndex)
    return(-1)

#####################################################################
# showSelected
#
# Purpose:    Displays a selected component's properties
#
# Arguments:  fvpTree  - The file View Panel tree (calTree)
#             
##########################
def showSelected(fvpTree):
    
    global curFileName, selectedItem
    itemIndexes = []
    itemIndex = 0
    log = ""
    itemIndexes = [0 for i in range(0,len(fvpTree.cal.primeData))]

    itemIndex = getSelectedIndex(fvpTree)

    itemIndexes[itemIndex] = 1;

    cal.writeFile("./tempOut.temp",fvpTree.cal.pointer,itemIndexes)
    fh = open("./tempOut.temp","r")
    info = fh.read()
    fh.close()
    os.remove("./tempOut.temp")
    info = info.split("\n")

    ignore = 0
    depth = -1
    for i in range(len(info)-4,0,-1):
        if (info[i][:5] == "BEGIN"):
            ignore = 0
            depth += 1
        elif (info[i][:3] == "END"):
            ignore = 1
            depth -= 1
        elif (depth == 0):
            break
        elif(not ignore):
            log += info[i] + "\n"

    writeToTextLog(textLog,"&SEP&")
    writeToTextLog(textLog,log)
    writeToTextLog(textLog,"Show-Component "+str(itemIndex+1)+":\n\n")

#####################################################################
# extractKind
#
# Purpose:    Displays the events or X-properties of the CalComp currently open in the file view panel
#             on the textLog.
#             
# Arguments:  fvpTree  - The file View Panel tree (calTree)
#             kind     - the 'kind' of information to extract; "e" for events, "x" for X-properties
#             
##########################   
def extractKind(fvpTree,kind):

    global curFileName

    result = cal.writeFile("tempIn.temp",fvpTree.cal.pointer,fvpTree.cal.visibleComps)
    if (result[0:2] == "OK"):  

        inFile = open("./tempIn.temp","r")
        outFile = open("./tempOut.temp","w")
        errFile = open("./tempErr.temp","w")

        if (not(kind == "e" or kind == "x")):
            writeToTextLog(textLog,"BAD KIND\n")

        if (kind == "e"):
            info = "Extract Events - "+str(curFileName)+":\n\n"
        else:
            info = "Extract X-Props - "+str(curFileName)+":\n\n"


        args = ["./caltool","-extract",kind]

        Pcaltool = subprocess.Popen(args,stdin = inFile, stdout = outFile ,stderr = errFile)
        Pcaltool.wait()
    

        inFile.close()
        outFile.close()
        errFile.close()
       

        inFile = open("./tempOut.temp","r")
        extractOut = inFile.read()
        inFile.close()
        errFile = open("./tempErr.temp","r")
        errors = errFile.read()
        errFile.close()

        #remove Files
        os.remove("tempIn.temp")
        os.remove("tempOut.temp")
        os.remove("tempErr.temp")



    writeToTextLog(textLog,"&SEP&")
    if (len(errors) == 0):
        #if nothing was found
        if (len(extractOut) == 0):
            if (kind == "e"):
                info += "No Events found.\n"
            else:
               info += "No X-Properties found.\n"
        else:
            info += extractOut
        writeToTextLog(textLog,info)
    else:
        writeToTextLog(textLog,errors)


#####################################################################
# changeSelection
#
# Purpose:  Changes the items that are selected on the tree and changes
#           if the 'show selected' is enabled or diabled (disabled on no selection
#           enabled otherwise).
#
##########################  
def changeSelection(event):
    global selectedItem, fvpTree

    #get the item id of the item they clicked on
    item = event.widget.identify("item", event.x, event.y)

    #Deselect if click on selected item
    if (item == selectedItem):
        fvpTree.tree.selection_remove(item)
        selectedItem = None
    #change what tree has selected on differrent item select
    else:
        selectedItem = item
        fvpTree.tree.selection_set(item)

    #detemine if 'show selected' button and 'store selected' menu option is enabled
    if(len(fvpTree.tree.selection()) == 0):
        showSelBut.config(state = DISABLED)
        dbMenu.entryconfigure('Store Selected',state = DISABLED)
    else: 
        showSelBut.config(state = NORMAL)
        dbMenu.entryconfigure('Store Selected',state = NORMAL)


#####################################################################
# confirm
#
# Purpose:    To close a parent window and set status to 1 indicating that an 
#             "ok" button was clicked.
#
# Arguments   parent  - The parent window to be destroyed
#             status  - A status to be set (IntVar)
##########################   
def confirm(parent,status):

    parent.destroy()
    status.set(1)

#####################################################################
# canel
#
# Purpose:    To close a parent window and set status to 0 indicating that a
#             "canel" button was clicked.
#
# Arguments   parent - The parent window to be destroyed
#             status - A status to be set (IntVar)
##########################   
def cancel(parent,status):

    parent.destroy()
    status.set(0)

#####################################################################
# filterWindowEscape
#
# Purpose:    To handel when the escape key is pressed with the
#             filter window in focus.
#
##########################   
def filterWindowEscape(event):
    event.destroy()

#BEGIN PROGRAM------------------------------------------------------------------------------------------------------------------------------------------

#Check number of args
if (len(sys.argv) == 1 or len(sys.argv) > 3):
    print("Usage: \"./xcal.py 'userName' 'hostName'\" (hostName is optional)")
    exit()

#determine host and username
host = 'dursley.socs.uoguelph.ca'
for i in range(1,len(sys.argv)):
    if (i == 1):
        userName = sys.argv[i]
    elif (i == 2):
        host = sys.argv[i]

#3 trys to connect
connected = False
for i in range(0,3):

    password = getpass.getpass()
    try:
        cnx = mysql.connector.connect(user=userName, password=password,
                                      host=host,
                                      database=userName)
        cursor = cnx.cursor()
        connected = True
        break

    except mysql.connector.Error as err:
        if (err.errno == errorcode.ER_ACCESS_DENIED_ERROR):
            print("Error: Invalid user name or password")
        elif (err.errno == errorcode.ER_BAD_DB_ERROR):
            print("Error: Database '"+userName+"' does not exist")
        else:
            print(err)

#Failed to connect
if (connected == False):
    print ("Maximum retries exceeded")
    exit()

#Create Tables
dbCreateOrg(cursor)
dbCreateEvent(cursor)
dbCreateTodo(cursor)

root = Tk()
#Width and height for window
rootW = 675
rootH = 500
#file dialog Options
fileOptions = {}
fileOptions['title'] = 'Open'
fileOptions['defaultextension'] = '.ics'
fileOptions['filetypes'] = [("iCalendar File",'.ics')]

#crtl handler variables
ctrlPressL = 0
ctrlPressR = 0

#the path and name of the file open in the file view panel
curFilePath = ""
curFileName = ""
unsavedChanges = 0
dateMaskSet = 0;
selectedItem = None

#Frames 
#---------------------------------------

#fileView--------------------------------------------------------------------------------
fileViewPanel = Frame(root)
treePanel = Frame(fileViewPanel)
buttonPanel = Frame(fileViewPanel)

#Tree
fvpTree = calTree(ttk.Treeview(treePanel))
fvpTree.tree.grid(row = 0, column = 0)

#tree Scroll Bar
fvpScroll = Scrollbar(treePanel, command = fvpTree.tree.yview)
fvpScroll.grid(row = 0, column = 1, sticky = "nsew")
fvpTree.tree.config(yscrollcommand = fvpScroll.set)

#file view panel buttons
#file vieselectmode buttons
showSelBut = Button(buttonPanel,text = "Show Selected",command = lambda: showSelected(fvpTree), state = DISABLED)
exEvBut = Button(buttonPanel,text = "Extract Events", command = lambda: extractKind(fvpTree,"e"), state = DISABLED)
exXBut = Button(buttonPanel,text = "Extract X-Props", command = lambda: extractKind(fvpTree,"x"), state = DISABLED)

showSelBut.grid(row = 0, column = 0)
exEvBut.grid(row = 0, column = 1)
exXBut.grid(row = 0, column = 2)
treePanel.pack()
buttonPanel.pack()
#End of FileView-------------------------------------------------------------------------



#LogView---------------------------------------------------------------------------------
logPanel = Frame(root)

textPanel = Frame(logPanel)

#Y scroll Bar for text area
textLogscrollY = Scrollbar(textPanel)
textLogscrollY.grid(row = 0, column = 1,  sticky = 'NSEW')

#text Area
textLog = Text(textPanel,relief = 'groove',wrap = WORD, height = 10,bd = 20, yscrollcommand = textLogscrollY.set,state = DISABLED)
textLogscrollY.config(command = textLog.yview)
textLog.grid(row = 0, column = 0, sticky  = "NSEW")

textPanel.pack(side = TOP)

#Clear Button
butt = Button(logPanel,text = "Clear", command = lambda: clearTextLog(textLog))
butt.pack(side = RIGHT)

fileViewPanel.pack(side = TOP)
logPanel.pack(side = BOTTOM)
#End of LogView--------------------------------------------------------------------------

#mainMenubar-------------------------------------------------------------------------------------------
menuBar = Menu(root, tearoff = 0)

#'File' menu, first item in main menu bar
fileMenu = Menu(root, tearoff = 0)
fileMenu.add_command(label = 'Open...', command = lambda: fileOpen(fvpTree))
fileMenu.add_command(label = 'Save', command = lambda: fileSave(fvpTree), state = DISABLED)

fileMenu.add_command(label = 'Save as...', command = lambda: fileSaveAs(fvpTree), state = DISABLED)
fileMenu.add_command(label = 'Combine...', command = lambda: fileCombine(fvpTree), state = DISABLED)
fileMenu.add_command(label = 'Filter...', command = lambda: fileFilter(fvpTree), state = DISABLED)
fileMenu.add_command(label = 'Exit', command = lambda: fileExit(cursor,cnx,fvpTree))

#'To-do' menu, second item in main menu bar
toDoMenu = Menu(root,tearoff = 0)
toDoMenu.add_command(label = 'To-do List...', command = lambda: toDoList(fvpTree),state = DISABLED)
toDoMenu.add_command(label = 'Undo...', command = lambda: toDoUndo(fvpTree),state = DISABLED)

#'Help' menu, third item in main menu bar
helpMenu = Menu(root,tearoff = 0)
helpMenu.add_command(label = 'Date Mask...', command = lambda: helpDateMsk())
helpMenu.add_command(label = 'About xCal', command = lambda: helpAbout())

#'DataBase' menu, first item in main menu bar
dbMenu = Menu(root, tearoff = 0)
dbMenu.add_command(label = 'Store All', command = lambda: dbStoreAll(cursor,cnx,fvpTree),state = DISABLED)
dbMenu.add_command(label = 'Store Selected', command = lambda: dbStoreSelected(cursor,cnx,fvpTree), state = DISABLED)

dbMenu.add_command(label = 'Clear', command = lambda: dbClear(cursor,cnx),state = DISABLED)
dbMenu.add_command(label = 'Status', command = lambda: dbStatus(cursor))
dbToggleClearOption(cursor) #determine if there are entries in any row and enable 'clear' button if so

#add menus to main menu bar
menuBar.add_cascade(label = 'File',menu = fileMenu)
menuBar.add_cascade(label = 'To-do',menu = toDoMenu)
menuBar.add_cascade(label = 'Help',menu = helpMenu)
menuBar.add_cascade(label = 'DataBase',menu = dbMenu)

root.config(menu = menuBar)
#end of menuBar-----------------------------------------

root.title("xcal")

#binding for clicking on tree components
fvpTree.tree.bind("<Button-1>", changeSelection)

#ctrl key bindings
root.bind("<Control_L>",ctrlLDownHandel)
root.bind("<KeyRelease-Control_L>",ctrlLUpHandel)

root.bind("<Control_R>",ctrlRDownHandel)
root.bind("<KeyRelease-Control_R>",ctrlRUpHandel)

#bindings for menu bar options
root.bind("<o>",oPress)
root.bind("<O>",oPress)

root.bind("<s>",sPress)
root.bind("<S>",sPress)

root.bind("<x>",xPress)
root.bind("<X>",xPress)
#Bind the 'to-do' Menu shorcuts to  keys
root.bind("<t>",tPress)
root.bind("<T>",tPress)

root.bind("<z>",zPress)
root.bind("<Z>",zPress)


root.protocol("WM_DELETE_WINDOW", lambda: fileExit(cursor,cnx,fvpTree))

checkDateMask()
root.minsize(rootW,rootH)


root.mainloop()
