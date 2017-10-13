#!/usr/bin/python3
# File: xcal.py

# Lindsay Elliott 
# CIS 2750 - Last updated: April 7th 2016
# lellio04 

# xcal.py, works with iCalModule.c, calutil.c and caltool.c to organize and display .ics 
#	calendar files 

from tkinter import * 
from tkinter.messagebox import *
from tkinter.scrolledtext import * 
from tkinter import ttk
from tkinter import filedialog

import os 
import cal
import mysql.connector
import getpass

class xcal_GUI:
	def __init__(self, master):
		root.title("xcal")
		root.minsize(600,600)
		root.configure(background="pink")
		self.cur_file = "hi" 
		
		self.mainWindow = self.buildMainScreen(master)
		
		try:
			os.environ ["DATEMSK"] 
		except KeyError: 
			if askyesno ("Date Mask", "Date Mask not set, would you like to set it?") == TRUE:
				self.datemask_btn()
			
		self.saved = IntVar()		
		self.saved = 0
			
		self.mainWindow.bind("<Control-o>", lambda o: self.open_btn(o))
		self.mainWindow.bind("<Escape>", lambda x: self.exit_btn(x))
		self.mainWindow.bind("<Control-s>", lambda s: self.save_btn(s))
		self.mainWindow.bind("<Control-x>", lambda x: self.exit_btn(x)) 
		self.mainWindow.bind("<Control-t>", lambda t: self.todo_btn(t))
		self.mainWindow.bind("<Control-z>", lambda z: self.undo_btn(z)) 
		  
	def open_btn(self, event):
		open_ok = IntVar() 
		open_ok = 1 
		if (self.saved == 0):
			new_file_name = filedialog.askopenfilename()
			if(new_file_name):
				open_ok = 0 
		else: 
			if askyesno("WAIT!", "You haven't saved changes, do you still want to open a new file?") == TRUE: 
				new_file_name = filedialog.askopenfilename()
				if (new_file_name): 
					open_ok = 0
				
		if (open_ok == 0):
			open(new_file_name, 'r')
			root.title("%s" % (new_file_name))
			self.cur_file = new_file_name; 
			self.show_button.config(state = NORMAL)
			self.extract_e.config(state = NORMAL)
			self.extract_x.config(state = NORMAL)
			self.menu_file.entryconfig("Save", state = NORMAL)
			self.menu_file.entryconfig("Save As...", state = NORMAL)
			self.menu_file.entryconfig("Combine", state = NORMAL)
			self.menu_file.entryconfig("Filter...", state = NORMAL)
			self.menu_todo.entryconfig("To-Do List...", state = NORMAL)
			self.menu_todo.entryconfig("Undo...", state = NORMAL)
			self.menu_data.entryconfig("Store All", state = NORMAL)
			self.menu_data.entryconfig("Store Selected", state = NORMAL)
			self.menu_data.entryconfig("Clear", state = NORMAL)
			self.menu_data.entryconfig("Status", state = NORMAL)
			self.menu_data.entryconfig("Query...", state = NORMAL)
			
			self.result = []
			cal_status = cal.readFile(new_file_name, self.result) 
			self.current_fvp = []
			self.current_fvp = self.result 
			self.saved = 0 
			
			self.organizers = []
			cal.getOrganizers(new_file_name, self.result, self.organizers)
			
			self.events = [] 
			cal.getEvents(new_file_name, self.result, self.events)
			
			self.todos = []
			cal.getTodos(new_file_name, self.result, self.todos)

			self.prop_tree.delete(*self.prop_tree.get_children())
			size = (len(self.result[1]))
			for i in range (0, size):
				self.prop_tree.insert("", END, text = i+1, values = (self.result[1][i][0], self.result[1][i][1], self.result[1][i][2], self.result[1][i][3]))
		
			write_info = open("temp.txt", "w+")
			info_string = "./caltool -info < %s > temp.txt" % (new_file_name) 
			os.system(info_string)
			write_info.close() 
			cal_info = open ("temp.txt", "r")
			cal_info_string = cal_info.read()
			cal_info.close()
			
			self.log_text.config(state = NORMAL)
			self.log_text.insert(END, cal_info_string + "\n")
			self.log_text.see(END)
			self.log_text.config(state = DISABLED)
		
	def save_btn(self, event):
		cur_file_string = "%s" % (self.cur_file)
		save_status = cal.writeFile(cur_file_string, self.result[0], self.result[1][1])
		self.saved = 0 
		root.title("%s" % (self.cur_file))
		self.log_text.config(state = NORMAL)
		self.log_text.insert(END, save_status + "\n")
		self.log_text.see(END)
		self.log_text.config(state = DISABLED)
		
	def saveas_btn(self, event):
		save_as_name = filedialog.asksaveasfilename()
		if save_as_name:
			open(save_as_name, 'w')
			root.title("%s" % (save_as_name))
			self.cur_file = save_as_name
			self.saved = 0 
			save_status = cal.writeFile(self.cur_file, self.result[0], self.result[1][1])
			self.log_text.config(state = NORMAL)
			self.log_text.insert(END, save_status + "\n")
			self.log_text.see(END)
			self.log_text.config(state = DISABLED)

	def combine_btn(self):
		combine_file = filedialog.askopenfilename()
		if (combine_file): 
			write_combine = open("temp.ics", "w+") 
			combine_string = "./caltool -combine %s < %s > temp.ics" % (combine_file, self.cur_file) 
			os.system(combine_string) 
			cal_combine_string = write_combine.read()
			self.result = [] 
			cal_status = cal.readFile("temp.ics", self.result)
			write_combine.close() 
			self.saved = FALSE 
			
			self.prop_tree.delete(*self.prop_tree.get_children()) 
			size = (len(self.result[1]))
			for i in range (0, size):
				self.prop_tree.insert("", END, text = i+1, values = (self.result[1][i][0], self.result[1][i][1], self.result[1][i][2], self.result[1][i][3]))
			
			write_info = open("temp.txt", "w+")
			info_string = "./caltool -info < temp.ics > temp.txt" 
			os.system(info_string)
			write_info.close() 
			cal_info = open ("temp.txt", "r")
			cal_info_string = cal_info.read()
		
			self.log_text.config(state = NORMAL)
			self.log_text.insert(END, cal_info_string + "\n")
			self.log_text.see(END)
			self.log_text.config(state = DISABLED)
			
	def filter_lvl(self):
		self.filter_msg = Toplevel(width = 200, height = 100)
		self.filter_msg.grab_set()
		self.filter_msg.title ("Filter...")
		self.filter_msg.minsize(100,100)
		filter_frame = Frame(self.filter_msg, width = 200, height = 100)
		filter_frame.pack()
		test_msg = Label(filter_frame, text = "Choose one:").pack()
		
		num = IntVar()
		todo_radio = Radiobutton(filter_frame, text = "Todos", variable = num, value = 1).pack()
		events_radio = Radiobutton(filter_frame, text = "Events", variable = num, value = 2).pack()
		
		to_frame = Frame(filter_frame)
		from_frame = Frame(filter_frame)
		to_var = StringVar()
		from_var = StringVar()
		from_label = Label(from_frame, text = "From Date: ").pack(anchor = "w", side = LEFT)
		from_txt = Entry(from_frame, width = 28, textvariable = from_var).pack(anchor = "e")
		to_label = Label(to_frame, text = "To Date: ").pack(anchor = "w", side = LEFT)
		to_txt = Entry(to_frame, width = 30, textvariable = to_var).pack(anchor = "e")
		
		from_frame.pack(pady = 5)
		to_frame.pack(pady = 5)
		
		self.filterOK_btn = Button(filter_frame, text = "Filter", command = lambda: self.filter_btn(num, to_var, from_var)).pack(fill = X, pady = 5, side = LEFT, expand = 1)
		self.cancel_btn = Button(filter_frame, text = "Cancel", command = self.filter_msg.destroy).pack(fill = X, pady = 5, side = LEFT, expand = 1)
		
	def filter_btn(self, num, to_var, from_var):
		n1 = num.get()
		from1 = from_var.get()
		to1 = to_var.get()

		self.filter_msg.destroy() 		
		write_filter = open("filter.ics", "w+")
		root.title("%s*" % (self.cur_file))
		self.saved = 1
		
		if n1 == 1:
			if from1 == "": 
				filter_string = "./caltool -filter t < %s > filter.ics" % (self.cur_file) 
			else: 
				filter_string = "./caltool -filter t from \"%s\" to \"%s\" < %s > filter.ics" % (from1, to1, self.cur_file) 
		else: 
			if from1 == "": 
				filter_string = "./caltool -filter e < %s > filter.ics" % (self.cur_file) 
			else: 
				filter_string = "./caltool -filter e from \"%s\" to \"%s\" < %s > filter.ics" % (from1, to1, self.cur_file) 
		
		os.system(filter_string)
		cal_filter_string = write_filter.read()
		write_filter.close()
		self.result = []
		cal_status = cal.readFile("filter.ics", self.result) 
		
		self.prop_tree.delete(*self.prop_tree.get_children())
		size = (len(self.result[1]))
		for i in range (0, size):
			self.prop_tree.insert("", END, text = i+1, values = (self.result[1][i][0], self.result[1][i][1], self.result[1][i][2], self.result[1][i][3]))
		
		write_info = open("info.txt", "w+")
		info_string = "./caltool -info < filter.ics > info.txt" 
		os.system(info_string)
		write_info.close() 
		cal_info = open ("info.txt", "r")
		cal_info_string = cal_info.read()
		
		self.log_text.config(state = NORMAL)
		self.log_text.insert(END, cal_info_string + "\n")
		self.log_text.see(END)
		self.log_text.config(state = DISABLED)
				
	def exit_btn(self, event):
		#delete all temporary files and free all C memory
		askyesno("Quit?", "Are you sure you want to quit?") #change this to yes or no 
		self.mainWindow.quit()
		
	def todo_btn(self, event):
		todo_msg = Toplevel(width = 300, height = 400, background = "pink")
		todo_msg.title("Todo List")
		todo_msg.minsize(300,400)
		todo_frame = Frame(todo_msg, width = 300, height = 100, background = "pink") 
		todo_frame.pack(fill = X)
		todo_lbl = Label (todo_frame, text = "To-Do List", font = ("Helvetica Light", 14), background = "pink").pack(pady = 10, expand = 1)
		
		write_filter = open("filter.ics", "w+")
		filter_todo = "./caltool -filter t < %s > filter.ics" % (self.cur_file) 
		os.system(filter_todo)
		cal_filter_string = write_filter.read()
		write_filter.close()
		self.todos = []
		cal_status = cal.readFile("filter.ics", self.todos) 
		
		check_frame = Frame(todo_frame, width = 300, height = 200, background = "pink")
		check_frame.pack(fill = X)

		for i in range (1, len(self.todos[1])):
			test1 = Checkbutton(check_frame, text = self.todos[1][i][3], variable = i, bg = "pink").pack(fill = X, anchor = "w", pady = 5)
			
		done_btn = Button(check_frame, text = "Done", command = todo_msg.destroy, background = "white").pack(fill = X, pady = 5, expand = 1, padx = 20, anchor = "s")
		
	def undo_btn(self, event):
		undo_msg = Toplevel(width = 200, height = 100)
		undo_msg.title("Undo")  
		undo_frame = Frame(undo_msg, width = 200, height = 100)
		undo_frame.pack()
		undo_txt = Label(undo_frame, text = "All Todo components removed since last\nsave will be restored?").pack()
		undo_button = Button(undo_frame, text = "Undo", command = undo_msg.destroy).pack(fill = X, pady = 5, side = LEFT, expand = 1)
		cancel_button = Button(undo_frame, text = "Cancel", command = undo_msg.destroy).pack(fill = X, pady = 5, side = LEFT, expand = 1)
		
	def datemask_btn(self):
		datemask_name = filedialog.askopenfilename()
		#os.environ["DATEMSK"] = datemask_name

	def about_btn(self): 
		showinfo("About", "xCalendar\nLindsay Elliott\nCompatable with iCalendar V2.0\n")

	def clear_btn(self):
		self.log_text.config(state = NORMAL) 
		self.log_text.delete('1.0', END)
		self.log_text.config(state = DISABLED)
		
	def show_btn(self):  
		#print (self.prop_tree.item(select, "text"))

		#num = IntVar() 
		#num = self.prop_tree.item(select, "text")
		#self.result[1] = self.result[1][num]
		
		#self.prop_tree.delete(*self.prop_tree.get_children())
		#size = (len(self.result[1]))
		#print (size)
		#for i in range (0, size):
		#	self.prop_tree.insert("", END, text = i, values = (self.result[1][i][0], self.result[1][i][1], self.result[1][i][2], self.result[1][i][3]))
		select = self.prop_tree.selection()
			
	def extract_e_btn(self):
		write_extract = open("temp.txt", "w+")
		extract_string = "./caltool -extract e < %s > temp.txt" % (self.cur_file) 
		os.system(extract_string)
		#write_extract.close() 
		#cal_extract = open ("extract.txt", "r")
		cal_extract_string = write_extract.read()
		write_extract.close()
		
		self.log_text.config(state = NORMAL)
		self.log_text.insert(END, cal_extract_string + "\n")
		self.log_text.see(END)
		self.log_text.config(state = DISABLED)
		
	def extract_x_btn(self):
		write_extract = open("temp.txt", "w+")
		extract_string = "./caltool -extract x < %s > temp.txt" % (self.cur_file) 
		os.system(extract_string)
		#write_extract.close() 
		#cal_extract = open ("extract.txt", "r")
		cal_extract_string = write_extract.read()
		write_extract.close()
		
		self.log_text.config(state = NORMAL)
		self.log_text.insert(END, cal_extract_string + "\n")
		self.log_text.see(END)
		self.log_text.config(state = DISABLED)

	def store_all(self):
		for i in range (len(self.organizers[0])):
			check = "SELECT org_id FROM ORGANIZER WHERE name = \"" + self.organizers[0][i][0] + "\";"
			cursor.execute(check)
			result = cursor.fetchone()
			
			if (result == None):
				org_string = "INSERT INTO ORGANIZER (org_id, name, contact) VALUES (NULL, '" + self.organizers[0][i][0] + "', '" + self.organizers[0][i][1] + "');"
				cursor.execute(org_string)
 	
		for i in range (len(self.events[0])):
			check = "SELECT event_id FROM EVENT WHERE summary = '" + self.events[0][i][0] + "' AND start_time = '" + self.events[0][i][1] + "';"
			cursor.execute(check)
			result = cursor.fetchone()

			if (result == None):
				if (self.events[0][i][3] != ""):
					check = "SELECT org_id from ORGANIZER WHERE name = '" + self.events[0][i][3] + "';"
					cursor.execute(check)
					result = cursor.fetchone()[0]
					event_string = "INSERT INTO EVENT (event_id, summary, start_time, location, organizer) VALUES (NULL, \"" + self.events[0][i][0] + "\", \"" + self.events[0][i][1] + "\", \"" + self.events[0][i][2] + "\", " + str(result) + ");"
				else: 
					event_string = "INSERT INTO EVENT (event_id, summary, start_time, location, organizer) VALUES (NULL, \"" + self.events[0][i][0] + "\", \"" + self.events[0][i][1] + "\", \"" + self.events[0][i][2] + "\", NULL);"
			
				cursor.execute(event_string)
		
		for i in range (len(self.todos[0])):
			check = "SELECT todo_id FROM TODO WHERE summary = '" + self.todos[0][i][0] + "';"
			cursor.execute(check)
			result = cursor.fetchone()
			
			if (result == None):
				if (self.events[0][i][2] != ""):
					check = "SELECT org_id from ORGANIZER WHERE name = '" + self.todos[0][i][2] + "';"
					cursor.execute(check)
					result = cursor.fetchone()[0]
					todo_string = "INSERT INTO TODO (todo_id, summary, priority, organizer) VALUES (NULL, \"" + self.todos[0][i][0] + "\", " + str(self.todos[0][i][1]) + ", " + str(result) + ");"
				else:
					todo_string = "INSERT INTO TODO (todo_id, summary, priority, organizer) VALUES (NULL, \"" + self.todos[0][i][0] + "\", " + int(self.todos[0][i][1]) + ", NULL);"
			
				cursor.execute(todo_string)
		
		connect.commit()
		self.status_btn()
		
	def store_selected(self):
		selected = self.prop_tree.focus()
		index = self.prop_tree.item(selected, "text")
		
		write_status = cal.writeSelected("temp.txt", self.result[0], index)
		
		self.selection = []
		selected_status = cal.readFile("temp.txt", self.selection) 
		
		self.organizers_s = []
		cal.getOrganizers("temp.txt", self.selection, self.organizers_s)
		check = "SELECT org_id FROM ORGANIZER WHERE name = \"" + self.organizers[0][0][0] + "\";"
		cursor.execute(check)
		result = cursor.fetchone()
		if (result == None):
			org_string = "INSERT INTO ORGANIZER (org_id, name, contact) VALUES (NULL, '" + self.organizers[0][0][0] + "', '" + self.organizers[0][0][1] + "');"
			cursor.execute(org_string)
		
		if (self.selection[1][1][0] == "VEVENT"):
			self.events_s = [] 
			cal.getEvents("temp.txt", self.selection, self.events_s)
			
			check = "SELECT event_id FROM EVENT WHERE summary = '" + self.events_s[0][0][0] + "' AND start_time = '" + self.events_s[0][0][1] + "';"
			cursor.execute(check)
			result = cursor.fetchone()

			if (result == None):
				if (self.events_s[0][0][3] != ""):
					check = "SELECT org_id from ORGANIZER WHERE name = '" + self.events_s[0][0][3] + "';"
					cursor.execute(check)
					result = cursor.fetchone()[0]
					event_string = "INSERT INTO EVENT (event_id, summary, start_time, location, organizer) VALUES (NULL, \"" + self.events_s[0][0][0] + "\", \"" + self.events_s[0][0][1] + "\", \"" + self.events_s[0][0][2] + "\", " + str(result) + ");"
				else: 
					event_string = "INSERT INTO EVENT (event_id, summary, start_time, location, organizer) VALUES (NULL, \"" + self.events_s[0][0][0] + "\", \"" + self.events_s[0][0][1] + "\", \"" + self.events_s[0][0][2] + "\", NULL);"
			
				cursor.execute(event_string)
				
		elif (self.selection[1][1][0] == "VTODO"):
			self.todos_s = []
			cal.getTodos("temp.txt", self.selection, self.todos_s)
			
			check = "SELECT todo_id FROM TODO WHERE summary = '" + self.todos_s[0][0][0] + "';"
			cursor.execute(check)
			result = cursor.fetchone()
			
			if (result == None):
				if (self.todos_s[0][0][2] != ""):
					check = "SELECT org_id from ORGANIZER WHERE name = '" + self.todos_s[0][0][2] + "';"
					cursor.execute(check)
					result = cursor.fetchone()[0]
					todo_string = "INSERT INTO TODO (todo_id, summary, priority, organizer) VALUES (NULL, \"" + self.todos_s[0][0][0] + "\", " + str(self.todos_s[0][0][1]) + ", " + str(result) + ");"
				else:
					todo_string = "INSERT INTO TODO (todo_id, summary, priority, organizer) VALUES (NULL, \"" + self.todos_s[0][0][0] + "\", " + str(self.todos_s[0][0][1]) + ", NULL);"
			
				cursor.execute(todo_string)
		
		connect.commit()
		self.status_btn()
		
	def clear_data(self): 
		self.results_txt.config(state = NORMAL) 
		self.results_txt.delete('1.0', END)
		self.results_txt.config(state = DISABLED)
		
	def status_btn(self):
		cursor.execute("""SELECT * FROM ORGANIZER;""")
		org_output = cursor.fetchall()
		org_num = len(org_output)
		
		cursor.execute("""SELECT * FROM EVENT;""")
		event_output = cursor.fetchall()
		event_num = len(event_output)
		
		cursor.execute("""SELECT * FROM TODO;""")
		todo_output = cursor.fetchall()
		todo_num = len(todo_output)
		
		status = "Database has " + str(org_num) + " organizers, " + str(event_num) + " events, " + str(todo_num) + " to-do items."
		self.log_text.config (state = NORMAL) 
		self.log_text.insert(END, status + "\n\n")
		self.log_text.see(END)
		self.log_text.config(state = DISABLED) 

	def truncate_table(self):
		cursor.execute("""SET FOREIGN_KEY_CHECKS = 0;""")
		cursor.execute("""TRUNCATE TABLE EVENT;""")
		cursor.execute("""TRUNCATE TABLE TODO;""")
		cursor.execute("""TRUNCATE TABLE ORGANIZER;""")
		cursor.execute("""SET FOREIGN_KEY_CHECKS = 1;""")
		self.status_btn()
			
	def help_btn(self):
		self.help_msg = Toplevel(width = 300, height = 500, background = "pink")
		self.help_msg.title ("Help")
		self.help_msg.minsize(200,300)
		
		self.help_frame = Frame(self.help_msg, width = 200, height = 500, background = "pink")
		
		cursor.execute("""DESCRIBE ORGANIZER;""")
		org_output = cursor.fetchall()
		cursor.execute("""DESCRIBE EVENT;""")
		event_output = cursor.fetchall()
		cursor.execute("""DESCRIBE TODO;""")
		todo_output = cursor.fetchall()
		
		org_title = Label (self.help_frame, text = "Organizer Table", background = "pink").pack()
		org_lbl = Text(self.help_frame, width = 50, height = 5)
		org_lbl.pack(pady = 10, padx = 10, fill = X)
		event_title = Label (self.help_frame, text = "Event Table", background = "pink").pack()
		event_lbl = Text(self.help_frame, width = 50, height = 5)
		event_lbl.pack(pady = 10, padx = 10, fill = X)
		todo_title = Label (self.help_frame, text = "Todo Table", background = "pink").pack()
		todo_lbl = Text(self.help_frame, width = 50, height = 5)
		todo_lbl.pack(pady = 10, padx = 10, fill = X)
		
		org_lbl.insert(INSERT, org_output)
		event_lbl.insert(INSERT, event_output)
		todo_lbl.insert(INSERT, todo_output)
		
		close_btn = Button (self.help_frame, text = "Close", command = self.help_msg.destroy)
		close_btn.pack(fill = X, padx = 10, pady = 5)
		
		self.help_frame.pack(padx = 5, pady = 5)
		
	def submit_btn(self):
		self.results_txt.config(state = NORMAL) 
		
		if (self.choice == self.OPTIONS[0]):
			query_string = self.query_txt.get("1.0", END)
			query_string = query_string[:-1]
			query_string = query_string + ";"
			
			try:
				cursor.execute(query_string)
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
			
			self.query_output = cursor.fetchall()
			for i in range (len(self.query_output)):
				self.results_txt.insert(INSERT, self.query_output[i])
				self.results_txt.insert(INSERT, "\n")
		
		elif (self.choice == self.OPTIONS[1]): 
			temp_string = "SELECT * FROM ORGANIZER WHERE name = \"" + self.blank.get() + "\";"
			
			try:
				cursor.execute(temp_string)
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				
			temp_org = cursor.fetchone()[0]
			query_string = "SELECT * FROM EVENT WHERE organizer = " + str(temp_org) + ";"
			query_string2 = "SELECT * FROM TODO WHERE organizer = " + str(temp_org) + ";"
			
			try:
				cursor.execute(query_string)
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				
			self.query_output = cursor.fetchall()
			for i in range (len(self.query_output)):
				self.results_txt.insert(INSERT, self.query_output[i])
				self.results_txt.insert(INSERT, "\n")
			
			try:
				cursor.execute(query_string2)
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				
			self.query_output = cursor.fetchall()
			for i in range (len(self.query_output)):
				self.results_txt.insert(INSERT, self.query_output[i])
				self.results_txt.insert(INSERT, "\n")
			
		elif (self.choice == self.OPTIONS[2]):
			query_string = "SELECT * FROM EVENT WHERE location = \"" + self.blank.get() + "\";"
				
			try:
				cursor.execute(query_string)
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
					
			self.query_output = cursor.fetchall()
			for i in range (len(self.query_output)):
				self.results_txt.insert(INSERT, self.query_output)
				self.results_txt.insert(INSERT, "\n")
			self.results_txt.insert(INSERT, "Total of " + str(len(self.query_output)) + " events.")
			self.results_txt.insert(INSERT, "\n")
		
		elif (self.choice == self.OPTIONS[3]): 
			self.results_txt.insert(INSERT, "I'm Sorry, follow me on twitter @L1NDZO\n")
			self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
			self.results_txt.see(END)
			self.results_txt.config(state = DISABLED)
			
		elif (self.choice == self.OPTIONS[4]): 
			splitS = self.blank.get()
			splitS = splitS.split(",")
			try:
				num1 = int(splitS[0])
				num2 = int(splitS[1])
			except: 
				self.results_txt.insert(INSERT, "Error: This needs to be int,int\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				return
				
			if (int(splitS[0]) > int(splitS[1])):
				self.results_txt.insert(INSERT, "Error: First int is higher than second.\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				return
				
			for i in range (int(splitS[0]), int(splitS[1])+1):
				query_string = "SELECT * FROM TODO WHERE priority = " + str(i) + ";"
				
				try:
					cursor.execute(query_string)
				except (Error):
					self.results_txt.insert(INSERT, "Invalid\n")
					self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
					self.results_txt.see(END)
					self.results_txt.config(state = DISABLED)
				
				self.query_output = cursor.fetchall()
				for i in range (len(self.query_output)):
					self.results_txt.insert(INSERT, self.query_output[i])
					self.results_txt.insert(INSERT, "\n")
			
		elif (self.choice == self.OPTIONS[5]): 
			try:
				cursor.execute("""SELECT * FROM EVENT;""")
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
			
			self.query_output = cursor.fetchall()
			count = len(self.query_output)
			for i in range (count):
				if (self.blank.get() in self.query_output[i][1]):
					temp = self.query_output[i]
					self.results_txt.insert(INSERT, temp)
					self.results_txt.insert(INSERT, "\n")
			
			try:
				cursor.execute("""SELECT * FROM TODO;""")
			except (Error):
				self.results_txt.insert(INSERT, "Invalid\n")
				self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
				self.results_txt.see(END)
				self.results_txt.config(state = DISABLED)
				
			self.query_output = cursor.fetchall()
			count = len(self.query_output)
			for i in range (count):
				if (self.blank.get() in self.query_output[i][1]):
					temp = self.query_output[i]
					self.results_txt.insert(INSERT, temp)
					self.results_txt.insert(INSERT, "\n")
			
		self.results_txt.insert(INSERT, "----------------------------------------------------------------------------------------------------\n")
		self.results_txt.see(END)
		self.results_txt.config(state = DISABLED)
		 
	def show_option(self, value):
		self.choice = value
		self.query_txt.config(state = NORMAL)
		self.query_txt.delete('1.0', END)
		if (value == self.OPTIONS[0]):
			self.query_txt.config(state = NORMAL)
			self.query_txt.insert(INSERT, "SELECT ")
		elif (value == self.OPTIONS[1]):
			self.query_txt.insert(INSERT, "Fill in the blank above.")
			self.query_txt.config(state = DISABLED)
		elif (value == self.OPTIONS[2]):
			self.query_txt.insert(INSERT, "Fill in the blank above.")
			self.query_txt.config(state = DISABLED)
		elif (value == self.OPTIONS[3]):
			self.query_txt.insert(INSERT, "Fill in the blank above.")
			self.query_txt.config(state = DISABLED)
		elif (value == self.OPTIONS[4]):
			self.query_txt.insert(INSERT, "Fill in the blank above.")
			self.query_txt.config(state = DISABLED)
		elif (value == self.OPTIONS[5]):
			self.query_txt.insert(INSERT, "Fill in the blank above.")
			self.query_txt.config(state = DISABLED)
		 
	def query_close(self):
		self.query = 0
		self.query_msg.destroy()
	
	def query_btn(self):
		if (self.query == 1):
			return
			
		self.query = 1
		self.query_msg = Toplevel(width = 500, height = 475, background = "pink")
		self.query_msg.title("Query")
		self.query_msg.minsize(500, 450)
		
		self.query_msg.protocol('WM_DELETE_WINDOW', self.query_close)
		
		self.query_frame = Frame(self.query_msg, width = 500, height = 200, background = "pink")
		self.query_lbl = Label (self.query_frame, text = "Query", font = ("Helvetica Light", 12), background = "pink").pack(pady = 5)
		
		self.OPTIONS = ["Ad-hoc query.",
					"Display the items of organizer _______ (SQL wild card % is permitted).", 
					"How many events take place in _______ (location)?", 
					"Transaction 3", 
					"Display todos from priority ______ to ______ in order (integer, integer).", 
					"Display events and todos with _______ in summary.(string)"]
		
		var = StringVar()
		var.set(self.OPTIONS[0])
		
		self.query_opt = OptionMenu(self.query_frame, var, self.OPTIONS[0], self.OPTIONS[1], self.OPTIONS[2], self.OPTIONS[3], self.OPTIONS[4], self.OPTIONS[5], command = self.show_option)
		self.query_opt.pack(fill = X, padx = 10)
		
		self.blank = StringVar()
		self.query_entry = Entry(self.query_frame, width = 28, textvariable = self.blank).pack(pady = 15)

		self.query_txt = Text(self.query_frame, width = 65, height = 6)
		self.query_txt.pack(padx = 10, pady = 5, side = LEFT)
		self.query_txt.insert(INSERT, "SELECT * FROM ")
		self.submit_btn = Button(self.query_frame, text = "Submit", command = self.submit_btn).pack(side = TOP, fill = BOTH, padx = 5, expand = 1, pady = 20)
		self.help_btn = Button(self.query_frame, text = "Help", command = self.help_btn).pack(side = BOTTOM, fill = BOTH, expand = 1, padx = 5, pady = 20)
		
		self.results_frame = Frame(self.query_msg, width = 500, height = 300, background = "pink")
		self.results_lbl = Label(self.results_frame, text = "Results", font = ("Helvetica Light", 12), background = "pink").pack(pady = 5) 
		self.results_txt = ScrolledText(self.results_frame, width = 50, height = 10, font = ("Helvetica Light", 12), state = DISABLED)
		self.results_txt.pack(padx = 5, pady = 5)
		self.clearD_btn = Button(self.results_frame, text = "Clear", command = lambda: self.clear_data())
		self.clearD_btn.pack(fill = X, padx = 15, pady = 5)
		self.close_btn = Button(self.results_frame, text = "Close", command = self.query_close).pack(fill = X, padx = 15, pady = 5)
		
		self.query_frame.pack(fill = BOTH, expand = 1, padx = 5, pady = 5)
		self.results_frame.pack(fill = BOTH, expand = 1, padx = 5, pady = 5)
		
	def fileMenu(self,r): 
		self.m = Menu(r)
		self.menu_file = Menu(self.m, tearoff = 0)
		self.menu_file.add("command", label="Open...", command = lambda: self.open_btn("open"), accelerator = "Ctrl+o")
		self.menu_file.add("command", label="Save", command = lambda: self.save_btn("save"), accelerator = "Ctrl+s", state = DISABLED)
		self.menu_file.add("command", label="Save As...", command = lambda: self.saveas_btn("saveas"), state = DISABLED)
		self.menu_file.add("command", label="Combine", command = self.combine_btn, state = DISABLED)
		self.menu_file.add("command", label="Filter...", command = self.filter_lvl, state = DISABLED)
		self.menu_file.add("command", label="Exit", command = lambda: self.exit_btn("exit"), accelerator = "Ctrl+x")
		
		self.menu_todo = Menu(self.m, tearoff = 0)
		self.menu_todo.add("command", label="To-Do List...", command = lambda: self.todo_btn("todo"), accelerator = "Ctrl+t", state = DISABLED)
		self.menu_todo.add("command", label="Undo...", command = lambda: self.undo_btn("undo"), accelerator = "Ctrl+z", state = DISABLED)
		
		self.menu_help = Menu(self.m, tearoff = 0)
		self.menu_help.add("command", label="Date Mask...", command = self.datemask_btn) 
		self.menu_help.add("command", label="About xcal...",  command = self.about_btn)
		
		self.menu_data = Menu(self.m, tearoff = 0)
		self.menu_data.add("command", label="Store All", command = self.store_all, state = DISABLED)
		self.menu_data.add("command", label="Store Selected", command = self.store_selected, state = DISABLED)
		self.menu_data.add("command", label="Clear", command = self.truncate_table, state = NORMAL)
		self.menu_data.add("command", label="Status", command = self.status_btn, state = NORMAL)
		self.menu_data.add("command", label="Query...", command = self.query_btn, state = NORMAL)
		self.query = 0
		
		cursor.execute("""SELECT * FROM ORGANIZER;""")
		org_output = cursor.fetchall()
		org_num = len(org_output)
		
		cursor.execute("""SELECT * FROM EVENT;""")
		event_output = cursor.fetchall()
		event_num = len(event_output)
		
		cursor.execute("""SELECT * FROM TODO;""")
		todo_output = cursor.fetchall()
		todo_num = len(todo_output)
		
		if ((org_num != 0) or (event_num != 0) or (todo_num != 0)):
			self.menu_data.entryconfig("Clear", state = NORMAL)
		
		self.m.add("cascade", menu = self.menu_file, label="File")
		self.m.add("cascade", menu = self.menu_todo, label="Todo")
		self.m.add("cascade", menu = self.menu_help, label="Help") 
		self.m.add("cascade", menu = self.menu_data, label="Database")
		
		return self.m

	def buildMainScreen(self, root):
		m = self.fileMenu(root)
		root.configure(menu = m)
		self.fileView = Frame (width = 600, height = 400, background="pink")
		self.fileView.pack(fill = BOTH, padx = 10, pady = 5, expand = 1,side = TOP)
		
		self.prop_tree = ttk.Treeview(self.fileView, height = 15, selectmode = "browse")
		
		scrollY_tree = Scrollbar(self.prop_tree, orient=VERTICAL)
		scrollY_tree.pack(side = RIGHT, fill = Y)
		self.prop_tree.config(yscrollcommand=scrollY_tree.set)
		scrollY_tree.config(command = self.prop_tree.yview)
		
		scrollX_tree = Scrollbar(self.prop_tree, orient=HORIZONTAL)
		scrollX_tree.pack(side = BOTTOM, fill = X)
		self.prop_tree.config(xscrollcommand = scrollX_tree.set)
		scrollX_tree.config(command = self.prop_tree.xview)
		
		self.prop_tree["columns"] = ("0", "1", "2", "3")
		self.prop_tree.column("#0", width = 50)
		self.prop_tree.column("#1", width = 100)
		self.prop_tree.column("#2", width = 100)
		self.prop_tree.column("#3", width = 100)
		self.prop_tree.column("#4", width = 200) 
		self.prop_tree.heading("#0", text = "No.")
		self.prop_tree.heading("#1", text = "Name")
		self.prop_tree.heading("#2", text = "Props")
		self.prop_tree.heading("#3", text = "Subs")
		self.prop_tree.heading("#4", text = "Summary")
		self.prop_tree.pack(padx = 5, pady = 5, fill = BOTH, expand = 1)
		
		self.show_button = Button(self.fileView, text = "Show Selected", font = ("Helvetica Light", 11), state = DISABLED, command = lambda: self.show_btn())
		self.show_button.pack(padx = 5, pady = 5, side = LEFT, fill = BOTH, expand = 1)
		self.extract_e = Button(self.fileView, text = "Extract Events", font = ("Helvetica Light", 11), state = DISABLED, command = lambda: self.extract_e_btn())
		self.extract_e.pack(padx = 5, pady = 5, side = LEFT, fill = BOTH, expand = 1) 
		self.extract_x = Button(self.fileView, text = "Extract xProps", font = ("Helvetica Light", 11), state = DISABLED, command = lambda: self.extract_x_btn())
		self.extract_x.pack(padx = 5, pady = 5, side = LEFT, fill = BOTH, expand = 1)
		
		self.logPanel = Frame(width = 600, height = 200, bg="pink")
		self.logPanel.pack(fill = BOTH, padx = 10, pady = 5, expand = 0); 
		self.log_text = ScrolledText(self.logPanel, width = 70, height = 10, font = ("Helvetica Light", 11), state = DISABLED)
		self.log_text.insert(INSERT, "Hello from the G-U-I\n")
		self.log_text.insert(END, "hi\nhi\nhi\nhi\nhi\nhi\nhi\nhey\nyo\nhi\nhey\nlindsay\n")
		self.log_text.see(END)
		self.log_text.pack(padx = 5, pady = 5, fill = BOTH, expand = 1) 
		clear_button = Button(self.logPanel, text = "Clear", font = ("Helvetica Light", 11),  command = lambda: self.clear_btn())
		clear_button.pack(padx = 5, pady = 5, fill = X, expand = 1)
		
		return root 

root = Tk()

user_name = sys.argv[1]

if (len(sys.argv) == 2):
	host_name = "dursley.socs.uoguelph.ca"
else:
	host_name = sys.argv[2]
	
i = IntVar() 
i = 0 
while i != 3:
	try:
		password = getpass.getpass()
		connect = mysql.connector.connect(user=user_name,password=password,host=host_name, database=user_name)
		cursor = connect.cursor()
		i = 3
	except mysql.connector.Error as err:
		if err.errno != None:
			i = i + 1
			if (i==3):
				print("3 tries you're out")
				exit()
			else: 
				print ("Try Again")
					
org_str = """CREATE TABLE IF NOT EXISTS ORGANIZER (
			org_id INT AUTO_INCREMENT PRIMARY KEY,
			name VARCHAR(60) NOT NULL, 
			contact VARCHAR(60) NOT NULL);"""

event_str = """CREATE TABLE IF NOT EXISTS EVENT (
			event_id INT AUTO_INCREMENT PRIMARY KEY, 
			summary VARCHAR(60) NOT NULL, 
			start_time DATETIME NOT NULL, 
			location VARCHAR(60), 
			organizer INT, 
			FOREIGN KEY(organizer) REFERENCES ORGANIZER(org_id) ON DELETE CASCADE) ;"""

todo_str = """CREATE TABLE IF NOT EXISTS TODO (
			todo_id INT AUTO_INCREMENT PRIMARY KEY, 
			summary VARCHAR(60) NOT NULL, 
			priority SMALLINT, 
			organizer INT,
			FOREIGN KEY(organizer) REFERENCES ORGANIZER(org_id) ON DELETE CASCADE);"""

try:
	cursor.execute(org_str); 
	cursor.execute(event_str); 
	cursor.execute(todo_str); 
except (Error):
	print ("Error creating tables")
	exit()
				
connect.commit()

xcal = xcal_GUI(root)
root.mainloop()

cursor.close()
connect.close()


