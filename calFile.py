#!/usr/bin/python3

#####################################################################
# calFile.py --  A class to store the information about calendar components
# 
# Last updated: 4:27am Apr. 4, 2016
#
# Nikolas Orkic 
# E-mail: norkic@mail.uoguelph.ca
##########################

import cal
class calFile:

    #####################################################################
    # __init__
    #
    # Purpose:    To initialize a calFile object and store it's information
    #
    # Arguments:  pointer    - The memory address of an allocated calComp structure (int)
    #             primeData  - A list of tuples containing each component's name,number of properties, 
    #                          number ofsubcomponents, and summery. This information will be displayed in
    #                          a calTree
    #             secData    - A list of tuples containg the each component's starting date,priority,location
    #                          organizer name, and orgizer contact. 
    #
    ##########################
    def __init__(self,pointer,primeData,secData):

        self.pointer = pointer

        self.primeData = []
        self.secData = []
        self.visibleComps = []

        for i in range(0,len(primeData)):
            self.primeData.append(primeData[i])
            self.secData.append(secData[i])
            self.visibleComps.append(1)

    #####################################################################
    # isEqual
    #
    # Purpose:    To check if two calFile's are equal, based off their CalComp's memory
    #             address
    #
    # Arguments:  otherCal: a different calFile object
    #
    # Returns:    True if they are equal, False if they are not
    #
    ##########################
    def isEqual(self,otherCal):
        if (otherCal.pointer == self.pointer):
            return(True)
        else:
            return(False)

    #####################################################################
    # hideSubs
    #
    # Purpose:    To set an invisible component's visibility flag to -1 (hidden from tree)
    #             if it is currently set to invisible
    #
    ##########################
    def hideSubs(self):
        for i in range(0, len(self.visibleComps)):
            if (self.visibleComps[i] == 0):
                self.visibleComps[i] = -1; 

    #####################################################################
    # freeCal
    #
    # Purpose:    To free memory assosiated with the calendar file and set it's address to None
    #
    ##########################
    def freeCal(self):

        cal.freeFile(self.pointer)
        self.pointer = None

