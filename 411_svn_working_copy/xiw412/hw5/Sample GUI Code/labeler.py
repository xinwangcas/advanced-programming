#!/opt/ActivePython-2.7/bin/python

# labeler.py

from Tkinter import *

root = Tk()
root.title("Labeler")
# root.geometry("200x100")

app = Frame(root)
app.grid()

lbl = Label(app, text = "I'm a label!")
lbl.grid()

root.mainloop()
