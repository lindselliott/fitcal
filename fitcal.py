import json 
import pdb 
import sys
import csv 


data = {} 
headings = [] 
row_count = 0 

with open(sys.argv[1], 'rb') as fitbit_csv:
	fitbit_reader = csv.reader(fitbit_csv, delimiter=',', quotechar='"')
	
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
				if headings[element_count] == 'Date':
					element_count = element_count + 1 
					continue 

				data[row[0]][headings[element_count]] = element
				element_count = element_count + 1  

		row_count = row_count + 1 

print data 
