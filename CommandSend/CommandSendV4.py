#######################################################################
# Includes but since this is python, imports
#######################################################################
import sys
import glob
import tkinter
from tkinter import *
from tkinter.ttk import Frame,Style,Button
import serial

#######################################################################
# General Stuff
#######################################################################
send = False
keyword = 'SEND_COMMAND'
listCommands = ["START","END"]

#######################################################################
# Class Definition
#######################################################################
class benderApp(Frame):
	def __init__(self, parent):
		Frame.__init__(self, parent)

		self.parent = parent
		self.initUI()

#######################################################################
# Working Functions
#######################################################################
	def initUI(self):
		self.parent.title("Bender Interface")
		self.pack(fill=BOTH,expand=1)
		blackBnr = tkinter.Label(self, width=575, height=400, bg="black")
		blackBnr.place(x=0,y=0)

#######################################################################
# Code to make a drop down for port
#######################################################################
		self.portNum = StringVar(self)
		self.portNum.set("COM0") # default value
		comChoice = OptionMenu(self, self.portNum,
			"COM0",
			"COM1",
		    "COM2",
		    "COM3",
		    "COM4",
		    "COM5",
		    "COM6",
		    "COM7",
		    "COM8",
		    "COM9",
		    "COM10")
		comChoice.place(x=180,y=57)

#######################################################################
# Create Scrollbar
#######################################################################
		scrollbar = Scrollbar(self)
		scrollbar.pack(side=RIGHT, fill=Y)
		self.listbox = Listbox(self, yscrollcommand=scrollbar.set,bg='white',fg='black')
		self.listbox.pack(side=RIGHT,fill=BOTH)
		# self.listbox.place(x=100,y=340)
		scrollbar.config(command=self.listbox.yview)

#######################################################################
# Create Labels
#######################################################################
		mainBnr = tkinter.Label(self, text='Send the Bender a Command', 
			fg="#1f929f", bg="black", font=("Helvetica", 16, "bold underline"))
		mainBnr.place(x=65,y=120)

		cmndBnr = tkinter.Label(self, text='Save as Command Type', 
			fg="#1f929f", bg="black", font=("Helvetica", 16))
		cmndBnr.place(x=100,y=220)

		valueBnr = tkinter.Label(self, text="Enter a Value for Length or Angle\nBend Angle is in Degrees while Length is in mm", 
			fg="white", bg="black")
		valueBnr.place(x=90,y=152)

		comBnr = tkinter.Label(self, text="Welcome to the Bender Interface\nPlease Select a COM Port", 
			fg="#1f929f", bg="black", font=("Helvetica", 16, "bold underline"))
		comBnr.place(x=50,y=000)		

#######################################################################
# Create Buttons
#######################################################################
		angleBtn = tkinter.Button(self, text="Save Bend Angle",
			width = 25,fg="white",bg="black",
			command=lambda: self.writeAngle(numberEntry.get()))
		angleBtn.place(x=20,y=265)

		feedBtn = tkinter.Button(self, text="Save Feed Length", 
			width=25, fg="white", bg="black", 
			command=lambda: self.writeLength(numberEntry.get()))
		feedBtn.place(x=220,y=265)

		sendBtn = tkinter.Button(self, text="Send", 
			width=20, fg="white", bg="black", 
			command=lambda: self.sendList())
		sendBtn.place(x=135,y=350)

		clearBtn = tkinter.Button(self, text="Clear", 
			width=20, fg="white", bg="black",
			command=lambda: self.clearCommands())
		clearBtn.place(x=135,y=300)

		endBtn = tkinter.Button(self, text="Exit", 
			width=20, fg="white", bg="black",
			command=lambda: self.closeProg())

		comBtn = tkinter.Button(self, text="Update COM Port", 
			width=20, fg="white", bg="black",
			command=lambda: self.updatePort())
		comBtn.place(x=135,y=90)

#######################################################################
# Create Entries
#######################################################################
		numberEntry = tkinter.Entry(self)
		numberEntry.place(x=150,y=190)

#######################################################################
# Functions
#######################################################################
	def clearCommands(self):
		listCommands=["START","END"]
		self.listbox.delete(0, 'end')

	def printCommands(self):
		if (listCommands[-2][0]=="B"):
			self.listbox.insert(END, listCommands[-2] + " degrees")
		if (listCommands[-2][0]=="F"):
			self.listbox.insert(END, listCommands[-2] + " mm")

	def updatePort(self):
		self.ser= serial.Serial(baudrate = 9600)
		self.ser.port = self.portNum.get()
		self.ser.open()

	def writeAngle(self,value):
		listCommands.insert(-1,'BEND '+str(value))
		self.printCommands()
		listCommands.insert(-1,'BEND '+str(value*-1))
		
	def writeLength(self,value):
		listCommands.insert(-1,'FEED '+ str(value))
		self.printCommands()

	def sendList(self):
		index = 0
		self.ser.write(listCommands[0].encode())
		index = 1
		while self.ser.isOpen():
			line = self.ser.readline().rstrip()
			if line == b'SEND_COMMAND':
				self.ser.write(listCommands[index].encode())
				print(line)
				# print()
				index+=1
			if listCommands[index-1] == "END":
				self.clearCommands()
				self.listbox.delete(0, 'end')
				# self.ser.close()
				# sys.exit()

#######################################################################
# Run Main Loop
#######################################################################
def main():
	window = tkinter.Tk()
	window.geometry("575x400")
	app = benderApp(window)
	app.parent.configure(bg='black')
	window.mainloop()
	
if __name__ == "__main__":
	main()