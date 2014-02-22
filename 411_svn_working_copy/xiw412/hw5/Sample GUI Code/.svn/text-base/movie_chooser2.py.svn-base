#!/opt/ActivePython-2.7/bin/python

# movie_chooser2.py

from Tkinter import *

class Application(Frame):
    def __init__(self, master):
        Frame.__init__(self, master)
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        Label(self,
	    text = "Choose your favorite movie types"
	    ).grid(row = 0, column = 0, sticky = W)

	# create variable for single, favorite type 
        self.favorite = StringVar()

	# create Comedy radio button
	Radiobutton(self,
            text = "Comedy",
            variable = self.favorite,
            value = "comedy.",
            command = self.update_text
            ).grid(row = 2, column = 0, sticky = W)

        # create Drama radio button
        Radiobutton(self,
            text = "Drama",
            variable = self.favorite,
            value = "drama.",
            command = self.update_text
            ).grid(row = 3, column = 0, sticky = W)

        # create Romance radio button
        Radiobutton(self,
            text = "Romance",
            variable = self.favorite,
            value = "romance.",
            command = self.update_text
            ).grid(row = 4, column = 0, sticky = W)

        # create text field to display results
        self.results_txt = Text(self, width = 40, height = 5, wrap = WORD)
        self.results_txt.grid(row = 5, column = 0, columnspan = 3)

    def update_text(self):
        """ Update text area and display user's favorite movie type. """
        message = "Your favorite type of movie is "
        message += self.favorite.get()
            
        self.results_txt.delete(0.0, END)
        self.results_txt.insert(0.0, message)

root = Tk()
root.title("Movie Chooser 2")
app = Application(root)
root.mainloop()
