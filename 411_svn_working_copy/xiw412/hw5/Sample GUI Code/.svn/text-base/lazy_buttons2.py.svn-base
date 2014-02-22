#!/opt/ActivePython-2.7/bin/python

# lazy_buttons2.py

from Tkinter import *

class Application(Frame):
    """ A GUI application with three buttons. """ 
    def __init__(self, master):
	Frame.__init__(self, master)
	self.grid()
	self.create_widgets()

    def create_widgets(self):
	self.bttn1 = Button(self, 
		text = "I do nothing!")
    	self.bttn1.grid()
       	self.bttn2 = Button(self)
       	self.bttn2.grid()	
       	self.bttn2.configure(text = "Me too!")
       	self.bttn3 = Button(self)
       	self.bttn3.grid()
	self.bttn3["text"] = "Same here!"

# main
root = Tk()
root.title("Lazy Buttons 2")
root.geometry("200x85")
app = Application(root)
root.mainloop()
