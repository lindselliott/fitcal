# ***************************************
# Programmer: Lindsay Elliott
# Title: Fitcal 
# Description: A CSV to ICS converter for 
#    raw exported Fitbit data.  
# ***************************************

from datetime import datetime 
import json 
import sys
import csv 


def read_fitbit_csv(fitbit_reader):
	data = {} 
	headings = [] 
	row_count = 0 

	for row in fitbit_reader:
		if row_count == 0:
			row_count = row_count + 1 
			continue

		if len(row) == 0: 
			continue

		if row_count == 1: 
			for element in row: 
				headings.append(element)
		else: 
			data[row[0]] = {}

			element_count = 0
			for element in row:
				data[row[0]][headings[element_count]] = element
				element_count = element_count + 1  

		row_count = row_count + 1 

	return data 

def write_event_property(ics_file, date): 
	ics_file.write("BEGIN:VEVENT\n")
	
	prop_date = datetime.strptime(date['Date'], "%Y-%m-%d")
	date_str = prop_date.strftime('%Y%m%d')

	summary = 'SUMMARY:Fitbit Activity for %s\n' % date['Date']
	ics_file.write(summary)

	uid = 'UID:Fitbit-Activity-%s\n' % date_str
	ics_file.write(uid)

	dtstart = 'DTSTART;TZID=America/Toronto:%sT000000\n' % date_str
	ics_file.write(dtstart)

	dtend = 'DTEND;TZID=America/Toronto:%sT010000\n' % date_str
	ics_file.write(dtend)

	ics_file.write("DESCRIPTION:")
	for element in date: 
		if element == 'Date':
			continue 

		description = '\t%s: %s\\n\n' % (element, date[element])
		ics_file.write(description)

	ics_file.write("END:VEVENT\n")

def write_calendar_property(ics_file, data):
	ics_file.write("BEGIN:VCALENDAR\n")
	ics_file.write("PRODID:-//Mozilla.org/NONSGML Mozilla Calendar V1.1//EN\n")
	ics_file.write("VERSION:2.0\n")

	for date in data: 
		write_event_property(ics_file, data[date])

	ics_file.write("END:VCALENDAR\n")

def write_ics_file(data):
	file = open("testing.ics", "w")
	write_calendar_property(file, data)
	file.close()

def main():
	data = {} 
	
	with open(sys.argv[1], 'rb') as fitbit_csv:
		fitbit_reader = csv.reader(fitbit_csv, delimiter=',', quotechar='"')
			
		data = read_fitbit_csv(fitbit_reader)

	write_ics_file(data)

main()