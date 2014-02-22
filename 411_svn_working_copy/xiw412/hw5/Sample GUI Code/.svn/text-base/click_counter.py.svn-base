#!/opt/ActivePython-2.7/bin/python

# click_counter.py

from Tkinter import *

class Application(Frame):
    def __init__(self, master):
        Frame.__init__(self, master)
        self.grid()
        self.bttn_clicks = 0    # number clicks
        self.create_widget()

    def create_widget(self):
        self.bttn = Button(self)
        self.bttn["text"]= "Total Clicks: 0"
        self.bttn["command"] = self.update_count
        self.bttn.grid()

    def update_count(self):
        self.bttn_clicks += 1
        self.bttn["text"] = "Total Clicks: " + str(self.bttn_clicks)

# main
root = Tk()
root.title("Click Counter")
root.geometry("200x85")
app = Application(root)
root.mainloop()
