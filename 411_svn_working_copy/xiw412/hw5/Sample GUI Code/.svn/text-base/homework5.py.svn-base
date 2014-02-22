#!/opt/ActivePython-2.7/bin/python

# Mad Lib
# Create a story based on user input

from Tkinter import *

class Application(Frame):
    """ GUI application that creates a story based on user input. """
    def __init__(self, master):
        Frame.__init__(self, master)
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        """ Create widgets to get story information and to display story. """
        # create instruction label
        Label(self,
              text = "Enter information for a new story"
              ).grid(row = 0, column = 0, columnspan = 2, sticky = W)

        # create a label and text entry for input string S
        Label(self,
	      text = "The first string S: "
              ).grid(row = 1, column = 0, sticky = W)
        self.StringS_ent = Entry(self)
        self.StringS_ent.grid(row = 1, column = 1, sticky = W)

        # create a label and text entry for input string T
        Label(self,
              text = "The second string T:"
              ).grid(row = 2, column = 0, sticky = W)
        self.StringT_ent = Entry(self)
        self.StringT_ent.grid(row = 2, column = 1, sticky = W)

	Label(self, text = "weight of insertion cins:").grid(row = 11, column = 0, sticky = W)
	self.cins = Scale(self,  from_ = 1, to = 20, orient = HORIZONTAL)
	self.cins.grid(row = 11, column = 1)
	Label(self, text = "weight of deletion cdel:").grid(row = 12, column = 0, sticky = W)
	self.cdel = Scale(self, from_ = 1, to = 20, orient = HORIZONTAL)
	self.cdel.grid(row = 12, column = 1)
	Label(self, text = "weight of substitution csub:").grid(row = 13, column = 0, sticky = W)
	self.csub = Scale(self, from_ = 1, to = 20, orient = HORIZONTAL)
	self.csub.grid(row = 13, column = 1)
#	cin.pack(row = 5, column = 1, sticky = W)
#	cdel = Scale(master, from_ = 0, to = 20, orient = HORIZONTAL)
#	cdel.pack()
#	csub = Scale(master, from_ = 0, to = 20, orient = HORIZONTAL)
#	csub.pack()
#	print cin.get()


       # create electric check button
 #       self.is_electric = BooleanVar()
 #       Checkbutton(self,
 #                   text = "electric",
 #                   variable = self.is_electric
 #                   ).grid(row = 4, column = 3, sticky = W)

        # create a label for body parts radio buttons
        Label(self,
	      text = "Please choose your preferred output contents:"
              ).grid(row = 5, column = 0, sticky = W)
	
        # create variable for single, body part
	self.get_edit = BooleanVar()
        Checkbutton(self,
                       text = "The full edit distance matrix",
                       variable = self.get_edit,
                    #   command = self.updata_text
                       ).grid(row = 5, column = 1, sticky = W)
#            column = 2
	self.get_back = BooleanVar()
        Checkbutton(self,
                        text = "The backtrack matrix",
                        variable = self.get_back,
                      #  command = self.updata_text
                        ).grid(row = 5, column = 2, sticky = W)
#            column = 3
	self.get_alignment = BooleanVar()
        Checkbutton(self,
                        text = "The alignment",
                        variable = self.get_alignment,
#                        command = self.updata_text
                        ).grid(row = 5, column = 3, sticky = W)
 
        # create a submit button
        Button(self,
	       text = "Show output:",
               command = self.get_matrix
               ).grid(row = 6, column = 0, sticky = W)

        self.story_txt = Text(self, width = 75, height = 10, wrap = WORD)
        self.story_txt.grid(row = 7, column = 0, columnspan = 4)

    def get_matrix(self):
        """ Fill text box with new story based on user input. """
        # get values from the GUI
        self.String_S = self.StringS_ent.get()
        self.String_T = self.StringT_ent.get()
	self.cins_value = self.cins.get()
	self.cdel_value = self.cdel.get()
	self.csub_value = self.csub.get()
        self.dist = {}
	self.backtrack = {}
	self.dist[(0 , 0)] = 0
	self.backtrack[(0 , 0)] = 0
	for i in range (1 , len(self.String_S) + 1):
		self.dist[(i , 0)] = self.dist[(i-1 , 0)] + self.cdel_value
		self.backtrack[(i , 0)] = "|"
	for j in range (1, len(self.String_T) + 1):
		self.dist[(0 , j)] = self.dist[(0 , j-1)] + self.cins_value
		self.backtrack[(0 , j)] = "-"
	for i in range (1 , len(self.String_S) + 1):
		for j in range (1 , len(self.String_T) + 1):
			self.dist1 = self.dist[(i - 1 , j)] + self.cdel_value
			self.dist2 = self.dist[(i , j - 1)] + self.cins_value
			if self.String_T[j - 1] != self.String_S[i - 1]:
				self.dist3 = self.dist[(i - 1, j - 1)] + self.csub_value
			else:
				self.dist3 = self.dist[(i - 1, j - 1)]
			if self.dist1 <= self.dist2 and self.dist1 <= self.dist3:
				self.dist[(i , j)] = self.dist1
				self.backtrack[(i , j)] = "|"
			elif self.dist2 < self.dist1 and self.dist2 <= self.dist3:
				self.dist[(i , j)] = self.dist2
				self.backtrack[(i , j)] = "-"
			elif self.dist3 < self.dist1 and self.dist3 < self.dist2:
				self.dist[(i , j)] = self.dist3
				self.backtrack[(i , j)] = "\\"

	for i in range (1 , len(self.String_S) + 1):
		for j in range (1 , len(self.String_T) + 1):
			self.dist1 = self.dist[(i - 1 , j)] + self.cdel_value
			self.dist2 = self.dist[(i , j - 1)] + self.cins_value
			if self.String_T[j - 1] != self.String_S[i - 1]:
				self.dist3 = self.dist[(i - 1, j - 1)] + self.csub_value
			else:
				self.dist3 = self.dist[(i - 1, j - 1)]
			if self.dist1 < self.dist2 and self.dist1 < self.dist3:
				self.dist[(i , j)] = self.dist1
				self.backtrack[(i , j)] = "|"
			elif self.dist2 < self.dist1 and self.dist2 < self.dist3:
				self.dist[(i , j)] = self.dist2
				self.backtrack[(i , j)] = "-"
			elif self.dist3 <= self.dist1 and self.dist3 <= self.dist2:
				self.dist[(i , j)] = self.dist3
				self.backtrack[(i , j)] = "\\"
	self.print_matrix()
    def print_matrix(self):
	if self.get_edit.get() == 1:
		for i in range (0 , len(self.String_S) + 1):
			for j in range (0 , len(self.String_T) + 1):
				print self.dist[(i , j)],
			print "\n"

        if self.get_back.get() == 1:
		for i in range (0 , len(self.String_S) + 1):
			for j in range (0 , len(self.String_T) + 1):
				print self.backtrack[(i , j)],
			print "\n"
	if self.get_alignment.get() == 1:
		m = 0
		self.dict = {}
		i = len(self.String_S)
		j = len(self.String_T)
		while i >= 0 and j >= 0:
			if self.backtrack[(i , j)] == "|":
				self.dict[m] = "|"
				m += 1
				i -= 1
				continue
			elif self.backtrack[(i , j)] == "-":
				self.dict[m] = "-"
				m += 1
				j -= 1
				continue
			elif self.backtrack[(i , j)] == "\\":
				self.dict[m] = "\\"
				m += 1
				i -= 1
				j -= 1
				continue
			else:
				self.dict[m] = "0"
				i -= 1
				j -= 1
				m += 1
		m = m - 1
		i = m - 1
		j = 0
		while (i >= 0):
			if self.dict[i] == "-":
				print "-",
				j += 1
			else:
				print self.String_S[m - i - j - 1],
			i -= 1
		print "\n"

		i = m - 1
		j = 0
		k = 0
		while (i >= 0):
			if self.dict[i] == "\\":
				if self.String_T[m - i - j - 1] == self.String_S[m - i - k - 1]:
					print "|",
				else:
					print " ",
			elif self.dict[i] == "|":
				j += 1
				print " ",
			elif self.dict[i] == "-":
				k += 1
				print " ",
			i -= 1
		print "\n"

		i = m - 1
		j = 0
		while (i >= 0):
			if self.dict[i] == "|":
				print "-",
				j += 1
			else:
				print self.String_T[m - i - j - 1],
			i -= 1
		print "\n"

# main
root = Tk()
root.title("Mad Lib")
app = Application(root)
root.mainloop()

