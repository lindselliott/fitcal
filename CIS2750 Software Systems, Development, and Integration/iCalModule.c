/* 
 * iCalModule.c 
 * Lindsay Elliott 
 * CIS2750 - Last Updates: April 7th 2016
 * lelliott 
 * The purpose of this file is to translate C code to python code via wrapper functions 
 * The C functions are from calutil.c and xcal.py will use them 
*/

#include <Python.h>
#include "calutil.h"
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h>

int getFullOrganizers (CalProp * prop, CalProp ** organizers, int size) {
    CalProp * tempPtr = prop; 

    while (tempPtr != NULL) { // put into an array of props in order to sort 
        if (strcmp(tempPtr->name, "ORGANIZER") == 0) {
				organizers[size] = malloc(sizeof(CalParam)); 
				organizers[size] = tempPtr; 
				size ++; 
        }
        tempPtr = tempPtr->next; 
    }
    return size; 
}

int findFullOrganizers (const CalComp * tempComp, CalProp ** organizers, int callNum) {
	static int size = 0; 

	if (callNum == 0) {
		size = 0; 
		size = getFullOrganizers(tempComp->prop, organizers, size); 
	}

	for (int i = 0; i < tempComp->ncomps; i++) {
		size = getFullOrganizers(tempComp->comp[i]->prop, organizers, size); 
		size = findFullOrganizers(tempComp->comp[i], organizers, 1); 
	} 
	return size; 
		
}

static PyObject * hello(PyObject * self, PyObject * args) {
	const char * var; 
	if (!PyArg_ParseTuple(args, "s", &var)) {
		printf ("This doesnt work\n"); 
		return NULL; 
	}
	 
	printf ("HELLO THERE %s\n", var); 
	return Py_BuildValue ("s", var);
}

PyObject *Cal_organizers (PyObject * self, PyObject * args) {
	char * filename; 
	char * statusString = malloc(sizeof(char)*BUFFER); 
	FILE * openFile; 
	CalStatus status; 
	CalComp * comp; 
	PyObject * result; 
	PyObject * organizers; 
	PyObject * list; 
	CalProp ** orgArr = malloc(sizeof(CalProp)*BUFFER); 
	int orgArrSize = 0; 
	CalParam * temp; 
	char * tempCN = NULL; 
	char * tempContact = NULL; 
	
	if (!PyArg_ParseTuple(args, "sOO", &filename, &result, &organizers)){
		printf("Did not work (Cal_organizers)\n"); 
		return NULL; 
	}
	
	openFile = fopen(filename, "r"); 
	status = readCalFile(openFile, &comp); 
	fclose(openFile); 
	
	orgArrSize = findFullOrganizers(comp, orgArr, 0); 
	
	list = PyList_New(0); 
	for (int i = 0; i < orgArrSize; i++) {
		tempContact = malloc(sizeof(char)*(strlen(orgArr[i]->value)+1)); 
		strcpy(tempContact, orgArr[i]->value); 
		
		temp = orgArr[i]->param; 
		while (temp != NULL) {
			if (strcmp(temp->name, "CN") == 0) {
				tempCN = malloc(sizeof(char)*(strlen(temp->value[0])+1)); 
				strcpy(tempCN, temp->value[0]); 
			}
			temp = temp->next; 
		}

		PyList_Append (list, Py_BuildValue("ss", tempCN, tempContact)); 
	}
	 
	PyList_Append(organizers, list);
	
	if (status.code == OK) {
		sprintf (statusString, "%d lines were successfully written", status.linefrom);  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", status.linefrom, status.lineto); 
	} 
	
	return Py_BuildValue ("s", statusString); 
}

PyObject *Cal_events (PyObject * self, PyObject * args) {
	char * filename; 
	PyObject * result; 
	PyObject * events; 
	char * statusString = malloc(sizeof(char)*BUFFER); 
	FILE * openFile; 
	PyObject * list; 
	char * tempSummary; 
	char * tempStart; 
	char * tempLocation; 
	char * tempOrganizer; 
	CalProp * temp; 
	CalComp * comp; 
	CalParam * tempParam; 
	CalStatus status; 
	
	if (!PyArg_ParseTuple(args, "sOO", &filename, &result, &events)){
		printf ("Did not work (Cal_events)\n"); 
		return NULL; 
	}
	
	openFile = fopen(filename, "r"); 
	status = readCalFile(openFile, &comp); 
	fclose(openFile); 
	
	list = PyList_New(0); 
	
	tempSummary = ""; 
	tempStart = ""; 
	tempLocation= ""; 
	tempOrganizer = ""; 
		
	if (strcmp(comp->name, "VEVENT") == 0) {
		temp = comp->prop; 
		for (int j = 0; j < comp->nprops; j++) {
			if (strcmp(temp->name, "SUMMARY") == 0) {
				tempSummary = temp->value; 
			} else if (strcmp(temp->name, "DTSTART") == 0) {
				tempStart = temp->value; 
			} else if (strcmp(temp->name, "LOCATION") == 0) {
				tempLocation = temp->value; 
			} else if (strcmp(temp->name, "ORGANIZER") == 0) {
				tempParam = temp->param; 
				for (int k = 0; k < temp->nparams; k++) {
					if (strcmp(tempParam->name, "CN") == 0) {
						tempOrganizer = tempParam->value[0]; 
					}
					tempParam = tempParam->next; 
				}
			}			
			temp = temp->next; 
		}
			
		PyList_Append(list, Py_BuildValue("ssss", tempSummary, tempStart, tempLocation, tempOrganizer));
	}
	
	for (int i = 0; i < comp->ncomps; i++) {
		tempSummary = ""; 
		tempStart = ""; 
		tempLocation= ""; 
		tempOrganizer = ""; 
		
		if (strcmp(comp->comp[i]->name, "VEVENT") == 0) {
			temp = comp->comp[i]->prop; 
			for (int j = 0; j < comp->comp[i]->nprops; j++) {
				if (strcmp(temp->name, "SUMMARY") == 0) {
					tempSummary = temp->value; 
				} else if (strcmp(temp->name, "DTSTART") == 0) {
					tempStart = temp->value; 
				} else if (strcmp(temp->name, "LOCATION") == 0) {
					tempLocation = temp->value; 
				} else if (strcmp(temp->name, "ORGANIZER") == 0) {
					tempParam = temp->param; 
					for (int k = 0; k < temp->nparams; k++) {
						if (strcmp(tempParam->name, "CN") == 0) {
							tempOrganizer = tempParam->value[0]; 
						}
						tempParam = tempParam->next; 
					}
				}			
				temp = temp->next; 
			}
			
			PyList_Append(list, Py_BuildValue("ssss", tempSummary, tempStart, tempLocation, tempOrganizer)); 
			//printf ("%s - %s - %s - %s\n", tempSummary, tempStart, tempLocation, tempOrganizer); 
		}	
	}
	
	PyList_Append(events, list); 
	
	if (status.code == OK) {
		sprintf (statusString, "%d lines were successfully written", status.linefrom);  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", status.linefrom, status.lineto); 
	}
	
	return Py_BuildValue ("s", statusString); 
}

PyObject *Cal_todos (PyObject * self, PyObject * args) {
	char * filename; 
	PyObject * result; 
	PyObject * todos; 
	char * statusString = malloc(sizeof(char)*BUFFER); 
	FILE * openFile; 
	PyObject * list; 
	char * tempSummary; 
	char * tempPriority;  
	char * tempOrganizer; 
	CalProp * temp; 
	CalComp * comp; 
	CalParam * tempParam; 
	CalStatus status; 
	
	if (!PyArg_ParseTuple(args, "sOO", &filename, &result, &todos)){
		printf ("Did not work (Cal_events)\n"); 
		return NULL; 
	}
	
	openFile = fopen(filename, "r"); 
	status = readCalFile(openFile, &comp); 
	fclose(openFile); 
	
	list = PyList_New(0); 
	
	tempSummary = ""; 
	tempPriority = ""; 
	tempOrganizer = ""; 
		
	if (strcmp(comp->name, "VTODO") == 0) {
		temp = comp->prop; 
		for (int j = 0; j < comp->nprops; j++) {
			if (strcmp(temp->name, "SUMMARY") == 0) {
				tempSummary = temp->value; 
			} else if (strcmp(temp->name, "PRIORITY") == 0) {
				tempPriority = temp->value; 
			} else if (strcmp(temp->name, "ORGANIZER") == 0) {
				tempParam = temp->param; 
				for (int k = 0; k < temp->nparams; k++) {
					if (strcmp(tempParam->name, "CN") == 0) { 
						tempOrganizer = tempParam->value[0]; 
					}
					tempParam = tempParam->next; 
				}
			}			
			temp = temp->next; 
		}
			
		PyList_Append(list, Py_BuildValue("sss", tempSummary, tempPriority, tempOrganizer)); 
	}
	
	for (int i = 0; i < comp->ncomps; i++) {
		tempSummary = ""; 
		tempPriority = ""; 
		tempOrganizer = ""; 
		
		if (strcmp(comp->comp[i]->name, "VTODO") == 0) {
			temp = comp->comp[i]->prop; 
			for (int j = 0; j < comp->comp[i]->nprops; j++) {
				if (strcmp(temp->name, "SUMMARY") == 0) {
					tempSummary = temp->value; 
				} else if (strcmp(temp->name, "PRIORITY") == 0) {
					tempPriority = temp->value; 
				} else if (strcmp(temp->name, "ORGANIZER") == 0) {
					tempParam = temp->param; 
					for (int k = 0; k < temp->nparams; k++) {
						if (strcmp(tempParam->name, "CN") == 0) { 
							tempOrganizer = tempParam->value[0]; 
						}
						tempParam = tempParam->next; 
					}
				}			
				temp = temp->next; 
			}
			
			PyList_Append(list, Py_BuildValue("sss", tempSummary, tempPriority, tempOrganizer)); 
		}	
	}
	
	PyList_Append(todos, list);
	
	if (status.code == OK) {
		sprintf (statusString, "%d lines were successfully written", status.linefrom);  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", status.linefrom, status.lineto); 
	}
	 
	return Py_BuildValue ("s", statusString); 
}

PyObject *Cal_readFile (PyObject * self, PyObject * args) {
	char * filename; 
	PyObject * result; 
	PyObject * list; 
	FILE * openFile; 
	CalStatus status;
	CalComp * comp;  
	CalProp * temp;
	char * tempSummary; 
	char * statusString = malloc(sizeof(char)*BUFFER);
	
	if (!PyArg_ParseTuple(args, "sO", &filename, &result)){
		printf ("Did not work (Cal_readFile)\n"); 
		return NULL; 
	}
	
	openFile = fopen(filename, "r"); 
	status = readCalFile(openFile, &comp);
	fclose(openFile); 
	
	list = PyList_New(0); 
	PyList_Append (list, Py_BuildValue("siis", comp->name, comp->nprops, comp->ncomps, "")); 
	
	for (int i = 0; i < comp->ncomps; i++) {
		tempSummary = ""; 
		
		if ((strcmp(comp->comp[i]->name, "VEVENT") == 0)||(strcmp(comp->comp[i]->name, "VTODO") == 0)) {
			temp = comp->comp[i]->prop; 
			for (int j = 0; j < comp->comp[i]->nprops; j++){
				if (strcmp(temp->name, "SUMMARY") == 0) {
					tempSummary = temp->value; 
				}
				temp = temp->next; 
			}
		}
	
		PyList_Append (list, Py_BuildValue("siis", comp->comp[i]->name, comp->comp[i]->nprops, comp->comp[i]->ncomps, tempSummary)); 
	}
	
	PyList_Append(result, Py_BuildValue ("k", (unsigned int*)comp));
	PyList_Append(result, list); 
	
	if (status.code == OK) {
		sprintf (statusString, "OK");  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", status.linefrom, status.lineto); 
	}
	
	return Py_BuildValue ("s", statusString); 
}

PyObject *Cal_writeFile (PyObject *self, PyObject * args) {
	char * filename;
	CalComp * pcal; 
	PyObject * complist; 
	FILE * writeFile; 
	CalStatus curStatus; 
	char * statusString = malloc(sizeof(char)*BUFFER);
	
	if (!PyArg_ParseTuple(args, "skO", &filename, (unsigned long*)&pcal, &complist)){
		printf ("This did not parse right\n"); 
		return NULL; 
	}
	
	writeFile = fopen(filename, "w+"); 
	curStatus = writeCalComp(writeFile, pcal);
	
	fclose(writeFile); 
	
	if (curStatus.code == OK) {
		sprintf (statusString, "%d lines were successfully written", curStatus.linefrom);  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", curStatus.linefrom, curStatus.lineto); 
	}
	
	return Py_BuildValue ("s", statusString); 
}

PyObject *Cal_writeSelected (PyObject * self, PyObject * args) {
	char * filename;
	CalComp * pcal; 
	int index; 
	FILE * writeFile; 
	CalStatus curStatus; 
	char * statusString = malloc(sizeof(char)*BUFFER);
	
	if (!PyArg_ParseTuple(args, "ski", &filename, (unsigned long*)&pcal, &index)){
		printf ("This did not parse right\n"); 
		return NULL; 
	}
	
	writeFile = fopen(filename, "w+"); 
	fprintf(writeFile, "BEGIN:VCALENDAR\r\n");
	curStatus = writeCalComp(writeFile, pcal->comp[index-2]);
	fprintf(writeFile, "END:VCALENDAR\r\n");
	fclose(writeFile); 
	
	if (curStatus.code == OK) {
		sprintf (statusString, "%d lines were successfully written", curStatus.linefrom);  
	} else {
		sprintf (statusString, "Error: from line %d to line %d", curStatus.linefrom, curStatus.lineto); 
	}
	
	return Py_BuildValue ("s", statusString); 
}

//PyObject *Cal_freeFile (PyObject * self, PyObject * args) {

//}

static PyMethodDef CalMethods [] = {
	{"hello", hello, METH_VARARGS, "says hello"},
	{"readFile", Cal_readFile, METH_VARARGS},
	{"writeFile", Cal_writeFile, METH_VARARGS},
	{"getOrganizers", Cal_organizers, METH_VARARGS},
	{"getEvents", Cal_events, METH_VARARGS}, 
	{"getTodos", Cal_todos, METH_VARARGS},
	{"writeSelected", Cal_writeSelected, METH_VARARGS},
	{ NULL, NULL, 0, NULL}
}; 

struct PyModuleDef calModuleDef = {PyModuleDef_HEAD_INIT, "Cal", NULL, -1, CalMethods}; 

PyMODINIT_FUNC PyInit_cal (){
	return PyModule_Create (&calModuleDef); 
}
