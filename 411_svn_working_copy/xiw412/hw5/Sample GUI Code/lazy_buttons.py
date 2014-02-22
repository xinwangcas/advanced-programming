#!/opt/ActivePython-2.7/bin/python

# lazy_buttons.py

from Tkinter import *

root = Tk()
root.title("Lazy Buttons")
root.geometry("200x100")

app = Frame(root)
app.grid()

bttn1 = Button(app, text = "I do nothing!")
bttn1.grid()

bttn2 = Button(app)
bttn2.grid()
bttn2.configure(text = "Me too!")

bttn3 = Button(app)
bttn3.grid()
bttn3["text"] = "Same here!"

root.mainloop()
