#xcal
--------------------------------------------------------------------------------

Author: Nikolas Orkic
E-mail: norkic@mail.uoguelph.ca


Compatability
--------------------------------------------------------------------------------
Requires:
    gcc 5.2.1
    Python 3.4.3
    Tkinter based on Tcl/Tk 8.6.0 
    mySQL version 5.5

Tested using above on Ubuntu 15.10 (64-bit)

Description
--------------------------------------------------------------------------------
xcal is an iCalendar file (.ics) utility that allows you to view  information about a calendar's components,
it's events, and X-Properties. It also allows you to filter a calendar down to only it's events or to-do
components between a desired period, combine two calendars, and keep check off to-do components to remove them from the
calendar as they are completed.
Finally, you may store event and todo components along with information about the organizers of those components in a 
mySQL database.


A datemask variable must be set in order for the program to intrepret dates.
While using the provided 'datemsk.txt' as a datemask, one valid way to enter a date
is in the form:
    'Mmm dd, yyyy' ex. 'Dec 14, 2015'

you can also use the keyword 'today' to enter the current day as a date.

How to compile and run
---------------------------------------------------------------------------------
To compile, type the command 'make' into the terminal window.

To run, type the command:
            './xcal.py  'username' 'hostname''.

If you wish to run the program without database features, type the command:
            './xcal_noSQL.py'
            

Keyboard Shortcuts
--------------------------------------------------------------------------------

ctrl+O...............File>Open
ctrl+S...............File>Save
ctrl+T...............To-do>To-do List
ctrl+Z...............To-do>Undo
ctrl+X...............File>Exit
Esc..................Closes any pop-up windows/dialog boxes
