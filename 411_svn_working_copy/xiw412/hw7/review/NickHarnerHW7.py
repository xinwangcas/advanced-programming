#!/opt/ActivePython-2.7/bin/python

#####################################################################################################

# Nicholas Harner
# CSE411
# Homework 7
# 11/19/2012

# This program is an exact mimic of Coelacanth:  a simple password generator.  It is a very simple
# application for creating passwords that can be used securely.  Coelacanth was originally written
# in C#.  My version is written in python, using Tkinter for the GUI.  This program allows the user
# to designate if he/she would like a password containing any combination of uppercase letters,
# lowercase letters, or digits.  The user can also request that the password be copied to the clipboard
# in windows following password generation.  In addition, the user can specify the length of the
# password to be generated.  Upon clicking generate, the password is created using the user
# specifications.  Upon clicking clear, the text box containing the generated password is cleared.
# This clear does not clear the clipboard however.

#To Run on Suns, type:
#   python NickHarnerHW7.py

# Coelacanth can be found at:
#       http://code.google.com/p/coelacanth/

#####################################################################################################

import os,sys
import math
import random  #need this to create random password
from Tkinter import *

def fatal():
    sys.exit()
    
class Application(Frame):
    """ GUI application that creates a new random password based on user specs """
    def __init__(self, master):
        Frame.__init__(self, master)
        self.grid(ipadx = 5, ipady = 2) #add some padding to make the GUI look nice
        self.create_widgets()  #create all those buttons and other things you see in GUI

    def create_widgets(self):
        """ Create widgets to get new password user specs and to display the password. """

        # create uppercase check button
        self.is_uppercase = BooleanVar()
        Checkbutton(self,
                    text = "Uppercase",
                    variable = self.is_uppercase
                    ).grid(row = 0, column = 0, columnspan = 2, sticky = W)

        # create lowercase check button
        self.is_lowercase = BooleanVar()
        Checkbutton(self,
                    text = "Lowercase",
                    variable = self.is_lowercase
                    ).grid(row = 1, column = 0, columnspan = 2, sticky = W)

        # create clipboard check button
        self.is_clipboard = BooleanVar()
        Checkbutton(self,
                    text = "Clipboard",
                    variable = self.is_clipboard
                    ).grid(row = 0, column = 2, columnspan = 2, sticky = W)

        # create digit check button
        self.is_digit = BooleanVar()
        Checkbutton(self,
                    text = "Digits",
                    variable = self.is_digit
                    ).grid(row = 2, column = 0, columnspan = 2, sticky = W)

        # create password label for spinbox
        self.pass_length = StringVar()
        self.previous_pass_length = StringVar()
        Label(self,
              text = "Length"
              ).grid(row = 2, column = 2, sticky = W)

        # create password length spinbox
        vcmd = (self.register(self.validate_float), '%s', '%P') #create validation command for spinbox monitoring
        self.spinbox = Spinbox(self,
                               textvariable = self.pass_length, from_=1, to=20, width = 6,
                               validate="key", validatecommand = vcmd)  #checks for validation upon key entry
        self.spinbox.grid(row = 2, column = 2, columnspan = 1, sticky = E)

        # create a generate button
        Button(self,
               text = "Generate", height = 1, width = 10,
               command = self.get_password
               ).grid(row = 3, column = 0, columnspan = 2, sticky = N, pady = 2, padx = 5)

        # create a clear button for password text box
        Button(self,
               text = "Clear", height = 1, width = 10,
               command = self.clear_password
               ).grid(row = 3, column = 2, columnspan = 2, sticky = W, pady = 2, padx = 5)
        
        # create a text box label for generated password
        Label(self,
              text = "Password"
              ).grid(row = 4, column = 0, columnspan = 1, sticky = E, pady = 5)

        # create a text box for generated password
        self.password_txt = Text(self,
                                 width = 21, height = 1, wrap = NONE)
        self.password_txt.grid(row = 4, column = 2, columnspan = 2,sticky = S, pady = 5)
        
        # set all check buttons to checked initially
        # also set default password length to 8
        self.is_digit.set(True)
        self.is_clipboard.set(True)
        self.is_uppercase.set(True)
        self.is_lowercase.set(True)
        self.pass_length.set('8')

    # validate spinbox entries
    def validate_float(self, previous, new):
        """ Validate the spinbox entry for only float values. """
        if new == "" or new == "-":
            return True

        #check to see if the input is a float
        try:
            float(new)
            self.previous_pass_length.set(new) #record most up-to-date valid input
            return True
        except ValueError:
            return False

    # clear password text box
    def clear_password(self):
        """ Clear text box with generated password. """
        self.password_txt.delete(0.0, END)

    # get password from entered specifications
    def get_password(self):
        """ Fill password text box with new password based on user specs. """

        #convert all entered decimal numbers into the next integer
        passwordLength = int(math.ceil(float(self.previous_pass_length.get())))

        # get values from the GUI
        digit = self.is_digit.get()
        uppercase = self.is_uppercase.get()
        lowercase = self.is_lowercase.get()
        clipboard = self.is_clipboard.get()

        # reset value in GUI to converted passwordLength
        self.pass_length.set(passwordLength)

        # If the user overrides the spinbox and enters a number manually, set values to closest option
        if passwordLength > 20:
            passwordLength = 20
            self.pass_length.set('20')
        if passwordLength < 1:
            passwordLength = 1
            self.pass_length.set('1')
        
        # generate password
        g = passwordGenerator(passwordLength,digit,uppercase,lowercase,clipboard)
        password = g.createPassword()

        # print password to text box
        self.clear_password()
        self.password_txt.insert(END, g.printPassword(password))

#####################################################################################################

class passwordGenerator:
    """A class for generating a password"""

    def __init__(self,passwordLength,digit,uppercase,lowercase,clipboard):
        self.pwdLength = int(passwordLength) #convert all these boolean and string vars to integers for use
        self.dig = int(digit)
        self.upper = int(uppercase)
        self.lower = int(lowercase)
        self.clip = int(clipboard)
        self.noChoices = 0

    # this function chooses random characters based on user input and returns them as a list
    def createPassword(self):	
        """ Create password based on user specs. """
        #initialize list
        passwordCharList = []
        pwd = []

        #define letters and digits
        lowercaseLetters = 'abcdefghijklmnopqrstuvwxyz'
        uppercaseLetters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
        digits = '0123456789'

        # make digits an option when randomly selecting a char (user must specify)
        if self.dig:
            passwordCharList.extend(list(digits))

        # make uppercase letters an option when randomly selecting a char (user must specify)
        if self.upper:
            passwordCharList.extend(list(uppercaseLetters))

        # make lowercase letters an option when randomly selecting a char (user must specify)
        if self.lower:
            passwordCharList.extend(list(lowercaseLetters))
            
        # choose random characters from option above (number of choices depends on password length)
        if self.dig or self.upper or self.lower:
            pwd = self.sample_with_replacement(passwordCharList,self.pwdLength)
        else:
            self.noChoices = 1  #record when no options are selected
        
        return pwd
    
    #This function a random sample list from a given list and the number of choices requested
    #NOTE:  This is sampling with replacement, so digits and letters can repeat in a password
    def sample_with_replacement(self, popList, numChoices):
        """ Sample a list with replacement. """

        passwordList = []

        # select random characters for password
        for x in range(0,numChoices):
            passwordList.extend(random.sample(popList,1))
        
        return passwordList

    #This function returns the string form of a given password character list
    def printPassword(self,pwd):
        """ Convert list to string and copy to clipboard if required. """

        #set generic empty password
        stringPassword = ""

        #only create new password if there are characters to choose from
        if self.noChoices != 1:
            # add characters in password list to password string
            for x in range(0,self.pwdLength):
                stringPassword += pwd[x]

            # add password to clipboard if user specified to do that
            if self.clip:
                c = Tk()
                c.withdraw()
                c.clipboard_clear()
                c.clipboard_append(stringPassword)
                c.destroy()

        return stringPassword

#####################################################################################################
# the main program

root = Tk()
root.title("Coelacanth")
app = Application(root)
root.mainloop()
