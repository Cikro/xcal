#!/usr/bin/python3

#####################################################################
# calTree.py --  A class to keep track of a ttk.Treeview tree that displays 
#                information about calendar components
# 
# Last updated: 4:27am Apr. 4, 2016
#
# Nikolas Orkic 
# E-mail: norkic@mail.uoguelph.ca
##########################

class calTree:

    #####################################################################
    # __init__
    #
    # Purpose:    Initialize a calTree object and assign its collumn headers
    #
    # Arguments:  - A tree (ttk.Treeview)
    ##########################
    def __init__(self,tree):

        self.tree = tree
        
        self.treeIds = []
        self.cal = None

        #set up the headings
        self.tree['columns'] = ('Name','Props','Subs','Summary')
        self.tree['selectmode'] = 'none'


        self.tree.column('#0',width = 50 , anchor = 'e')
        self.tree.heading('#0', text = 'No.')

        self.tree.column('Name', width = 100, anchor = 'e')
        self.tree.heading('Name', text = 'Name')

        self.tree.column('Props', width = 50, anchor = 'e')
        self.tree.heading('Props', text = 'Props')

        self.tree.column('Subs', width = 50, anchor = 'e')
        self.tree.heading('Subs', text = 'Subs')

        self.tree.column('Summary', width = 400, anchor = 'w')
        self.tree.heading('Summary', text = 'Summary')


    #####################################################################
    # removeCal
    #
    # Purpose:    To free memory allocated in a calFile object and set the calTree's cal it None
    #
    ##########################
    def removeCal(self):
        if (self.cal != None):
            self.cal.freeCal()
            self.cal = None

    #####################################################################
    # addCal
    #
    # Purpose:    To set the calTree object's cal to a given calFile and populate 
    #             it with that calFile's information
    #
    # Arguments:  cal - A calFile object to add to the calTree
    #
    ##########################
    def addCal(self,cal):
        

        #if there is no cal assigned
        if (self.cal == None):
            self.cal = cal

        #if a cal different from the current cal is trying to be assigned
        elif(not(self.cal.isEqual(cal))):
            self.removeCal()
            self.cal = cal

        #populate the tree
        numItems = 1
        for i in range(0, len(self.cal.primeData)):

            #if the item belongs on the tree (is visible or hidden)
            if (self.cal.visibleComps[i] != -1):
                tId = self.tree.insert('','end', text = (numItems), values = self.cal.primeData[i])
                numItems += 1
                self.treeIds.append(tId)
            else:
                self.treeIds.append(None)

    #####################################################################
    # updateTreeItems
    #
    # Purpose:    To show/hide tree items from view based off of each component's 
    #             visibility flag
    #
    ##########################
    def updateTreeItems(self):
        
        #Show/hide tree items based on their visibility
        for i in range(0, len(self.cal.visibleComps)):

            #hide if notVisible
            if (self.cal.visibleComps[i] == 0):
                self.tree.detach(self.treeIds[i])
            #show if visible
            elif (self.cal.visibleComps[i] == 1):
                self.tree.reattach(self.treeIds[i],'',i)
                
    #####################################################################
    # clearTree
    #
    # Purpose:    To clear all elements from a tree
    #
    ##########################
    def clear(self):
        for i in self.tree.get_children(''):
            self.tree.delete(i)
        del(self.treeIds)
        self.treeIds = []
