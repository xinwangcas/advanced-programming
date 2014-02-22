#!/opt/ActivePython-2.7/bin/python

# Sequence Creation
# Create a sequence comparision based on user input or file reading

from Tkinter import *

class Application(Frame):
    """ GUI application that creates  based on user input. """
    def __init__(self, master):
        Frame.__init__(self, master)
        self.grid()
        self.create_widgets()

       
    def create_widgets(self):
        """ Create widgets to get story information and to display story. """
        # create instruction label
	self.selection = StringVar()
#	self.selection.set(None)
	column = 0
 	self.entry_selection = ["Enter two strings" , "Enter two file names"]
	for part in self.entry_selection:
		Radiobutton(self,
			    text = part,
			    variable = self.selection,
			    value = part
			   ).grid(row = 0, column = column, sticky = W)
		column += 1

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

	# create labels and scrolling text entries for costs of deleting, inserting and substituting
	# characters, cdel, cins, csub
	Label(self, text = "weight of insertion cins:").grid(row = 3, column = 0, sticky = W)
	self.cins = Scale(self,  from_ = 1, to = 20, orient = HORIZONTAL)
	self.cins.grid(row = 3, column = 1, sticky = W)
	Label(self, text = "weight of deletion cdel:").grid(row = 4, column = 0, sticky = W)
	self.cdel = Scale(self, from_ = 1, to = 20, orient = HORIZONTAL)
	self.cdel.grid(row = 4, column = 1, sticky = W)
	Label(self, text = "weight of substitution csub:").grid(row = 5, column = 0, sticky = W)
	self.csub = Scale(self, from_ = 1, to = 20, orient = HORIZONTAL)
	self.csub.grid(row = 5, column = 1, sticky = W)
        
	#create output preferences, use Checkbutton for multiple choices of distance matrix, backtrack
	#matrix, and alignment
	Label(self,
	      text = "Please choose your preferred output contents:"
              ).grid(row = 6, column = 0, rowspan = 3, sticky = W)
	
	self.get_edit = BooleanVar()
        Checkbutton(self,
                       text = "The full edit distance matrix",
                       variable = self.get_edit,
                       ).grid(row = 6, column = 1, sticky = W)
	self.get_back = BooleanVar()
        Checkbutton(self,
                        text = "The backtrack matrix",
                        variable = self.get_back,
                        ).grid(row = 7, column = 1, sticky = W)
	self.get_alignment = BooleanVar()
        Checkbutton(self,
                        text = "The alignment",
                        variable = self.get_alignment,
                        ).grid(row = 8, column = 1, sticky = W)

        # create a submit button
        Button(self,
	       text = "Show output:",
               command = self.get_matrix
               ).grid(row = 9, column = 0, columnspan = 2, sticky = W)

	# create a scroll bar
	self.scrollbar = Scrollbar(self, orient = VERTICAL)
	self.scrollbar.grid(row = 0, column = 3, rowspan = 9, sticky = W + S + N + E)
	self.scrollbar1 = Scrollbar(self, orient = HORIZONTAL)
	self.scrollbar1.grid(row = 9, column = 2, sticky = W + S + N + E)
	self.font = ('courier',16)
        self.matrix_txt = Text(self, width = 30, height = 40, wrap = WORD, font = self.font, \
			yscrollcommand = self.scrollbar.set, xscrollcommand = self.scrollbar1.set)

        self.matrix_txt.grid(row = 0, column = 2, rowspan = 9)

	self.scrollbar.config(command = self.matrix_txt.yview)
	self.scrollbar1.config(command = self.matrix_txt.xview)

    def get_matrix(self):
        """ Fill text box with new story based on user input. """

        # get values from the GUI, use audiobutton for either input, two strings or two files
	self.String_S = self.StringS_ent.get()
	self.String_T = self.StringT_ent.get()
	print self.selection.get()
	if self.selection.get() == "Enter two strings":
		pass
	elif self.selection.get() == "Enter two file names":
		self.fp1 = open(self.String_S,"r")
		self.fp2 = open(self.String_T,"r")
		self.String_S = self.fp1.read()
		self.String_T = self.fp2.read()
		self.String_S = self.String_S[0: len(self.String_S)-1]
		self.String_T = self.String_T[0: len(self.String_T)-1]
	self.cins_value = self.cins.get()
	self.cdel_value = self.cdel.get()
	self.csub_value = self.csub.get()
        self.dist = {}
	self.backtrack = {}
	self.dist[(0 , 0)] = 0
	self.backtrack[(0 , 0)] = 0

	# calculate the values in edit distance matrix and obtain back track matrix according to dynamic programming
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

	# print distance matrix, back track matrix and alignment
    def print_matrix(self):
	distance = ""
	if self.get_edit.get() == 1:
#if the checkbutton of edit distance matrix is pressed, calculate
#values according to dynamic programming
		for i in range (0 , len(self.String_S) + 1):
			for j in range (0 , len(self.String_T) + 1):
				print self.dist[(i , j)],
				distance += str(self.dist[(i , j)])
				distance += " "
			print "\n"
			distance += "\n"
		distance += "\n"

        if self.get_back.get() == 1:
# if the checkbutton of back track matrix is pressed, trace back
# the way to obtain minimal edit distance value
		for i in range (0 , len(self.String_S) + 1):
			for j in range (0 , len(self.String_T) + 1):
				print self.backtrack[(i , j)],
				distance += str(self.backtrack[(i , j)])
				distance += " "
			print "\n"
			distance += "\n"
		distance += "\n"

#get the alignment accroding to the charactor on path that optimal
#value is obtained
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
		
		# find corresponding character in alignment in strings according to path in back track matrix
		m = m - 1
		i = m - 1
		j = 0
		while (i >= 0):
			if self.dict[i] == "-":
				print "-",
				distance += "-"
				distance += " "
				j += 1
			else:
				print self.String_S[m - i - j - 1],
				distance += str(self.String_S[m - i - j - 1])
				distance += " "
			i -= 1
		print "\n"
		distance += "\n"

		i = m - 1
		j = 0
		k = 0
		while (i >= 0):
			if self.dict[i] == "\\":
				if self.String_T[m - i - j - 1] == self.String_S[m - i - k - 1]:
					print "|",
					distance += "|"
					distance += " "
				else:
					print " ",
					distance += " "
					distance += " "
			elif self.dict[i] == "|":
				j += 1
				print " ",
				distance += " "
				distance += " "
			elif self.dict[i] == "-":
				k += 1
				print " ",
				distance += " "
				distance += " "
			i -= 1
		print "\n"
		distance += "\n"

		i = m - 1
		j = 0
		while (i >= 0):
			if self.dict[i] == "|":
				print "-",
				distance += "-"
				distance += " "
				j += 1
			else:
				print self.String_T[m - i - j - 1],
				distance += str(self.String_T[m - i - j - 1])
				distance += " "
			i -= 1
		print "\n"
		distance += "\n"
	distance += "\n"
	distance += "final edit distance value:"
	distance += " "
	distance += str(self.dist[len(self.String_S), len(self.String_T)])
	distance += "\n"

	self.matrix_txt.insert(0.0 , distance)

# main
root = Tk()
root.title("Sequence Compare")
app = Application(root)
root.mainloop()

