# Includes but since this is python, imports
import tkinter

################################################################
# Various definitions for the interface
# create a new window
window = tkinter.Tk()

window.title("Send Bender Command")
window.geometry("400x400")
# Create Labels
cmndBnr = tkinter.Label(window, text='Select a Command Type', fg="white", bg="black")
valueBnr = tkinter.Label(window, text="Enter a Value", fg="white", bg="black")

# Create Buttons
bendBtn = tkinter.Button(window, text="Bend Angle", fg="white", bg="black")
feedBtn = tkinter.Button(window, text="Feed Length", fg="white", bg="black")
sendBtn = tkinter.Button(window, text="Send", fg="white", bg="black")

# Create Entries
numberEntry = tkinter.Entry(window)

# print the widgets to the window
cmndBnr.pack()
bendBtn.pack()
feedBtn.pack()
valueBnr.pack()
numberEntry.pack()
sendBtn.pack()


################################################################


# Run the window
window.mainloop()





