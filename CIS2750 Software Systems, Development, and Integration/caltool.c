
/********
caltool.c -- Public interface for iCalendar tools in caltool.c
Last updated:  March 19th 2016

Name: Lindsay Elliott
ID: 0875476
Email: lellio04

********/

#include "caltool.h"
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <time.h>
#include <assert.h>

/*  Description: traverses through the top layer of CalComps and counts them 
		(not recursive), traverses through the comps and sorts them into events, 
		todos, and other 
    Arguments: 
    	tempComp, current comp to be analyzed 
    Return value: void, does return (int)array of types 
    	types[5] = {totalSum, totalEvents, totalToDos, totalOther, totalSubComponents} */
void countComps(const CalComp * tempComp, int * types) {
	// do not include VCALENDAR 
	for (int i = 0; i < tempComp->ncomps; i++) {
		types[0]++; 
		if (strcmp(tempComp->comp[i]->name, "VEVENT") == 0) {
			types[1]++; 
		} else if (strcmp(tempComp->comp[i]->name, "VTODO") == 0) {
			types[2]++; 
		} else {
			types[3]++; 
		}

		types[4] = types[4] + tempComp->comp[i]->ncomps; 
	} 
}

/*  Description: recursively traverses through all props and finds the total number 
    Arguments: 
    	tempComp, main CalComp to be analyzed and broken down 
    	newProp, integer used to signal the function it needs to reset static variables 
    Return value: total number of props in a component (all layers) */
int countProps(const CalComp * tempComp, int newProp) {
	static int numProps = 0; 
	if (newProp == 0) {
		numProps = 0; 
	}
	numProps = numProps + tempComp->nprops; 

	for (int i = 0; i < tempComp->ncomps; i++) {
		countProps(tempComp->comp[i], 1); 
	}
	return numProps; 
}

/*  Description: writes a line to file and folds it if needed 
    Arguments: 
        ics, file to append the line to the end of 
        line, string to write to the file and fold if over specified FOLD_LEN guidelines 
    Return value: should return the number of lines written to the file */
int writeFoldedLine (FILE *const ics, char * line) {
    int size = 0; 

    if (strlen(line) > FOLD_LEN) {
        for (int i = 0; i < strlen(line); i++) {
            if ((fprintf (ics, "%c", line[i])) < 0) {
            	fprintf (stderr, "IOERR\n"); 
            } 
            if ((i%FOLD_LEN == 0)&&(i != 0)) {
                if ((fprintf (ics, "\r\n ")) < 0) {
                	fprintf (stderr, "IOERR\n"); 
                } 
                size++; 
            } 
        }
    } else {
        if ((fprintf(ics, "%s", line)) < 0) {
        	fprintf (stderr, "IOERR\n"); 
        } 
        size ++; 
    }
    if ((fprintf (ics, "\r\n")) < 0) {
    	fprintf (stderr, "IOERR\n"); 
    } 
    return size; 
}

/*  Description: concantonates any params and their values to pString 
    Arguments: 
        ics, file to write to
        param, current param to be formatted properly  
    Return value: void */
char * writeSubParams(FILE *const ics, CalParam * param) {
    CalParam * tempParam = param; 
    char * pString; 

    pString = malloc(sizeof(char)*BUFFER); 

    while (tempParam != NULL) { 
        strcpy(pString, ";"); 
        strcat(pString, tempParam->name); 
        strcat(pString, "="); 
        strcat(pString, tempParam->value[0]); 
        for (int i = 1; i < (tempParam->nvalues); i++) {
            strcat(pString, tempParam->value[i]); 
        }
        tempParam = tempParam->next; 
    }

    return pString; 
}

/*  Description: formats the current prop and its params, sends string to be written 
        to file and folded if needed 
    Arguments: 
        ics, file to be written to 
        prop, current prop to be formatter 
    Return value: number of lines written to file  */
int writeSubProps(FILE *const ics, CalProp * prop, int numComp) {
    CalProp * tempProp = prop;
    char * line = NULL; 
    char pString[BUFFER]; 
    int lineto = 0; 
    int linesPerProp = 0; 

    while (tempProp != NULL) {
    	if ((numComp == 1)&&((strcmp(tempProp->name, "VERSION") == 0)||(strcmp(tempProp->name, "PRODID") == 0))) {
    		tempProp = tempProp->next; 
    	} else {

        line = malloc(sizeof(char)*(BUFFER)); 
        assert(line!=NULL); 
        strcpy(line, tempProp->name); 
        
        if (tempProp->nparams > 0) {
            strcpy(pString, writeSubParams(ics, tempProp->param)); 
            strcat(line, pString); 
        }
 
        strcat(line, ":"); 
        strcat(line, tempProp->value);

        linesPerProp = writeFoldedLine(ics, line); 
        lineto++; 
        lineto = lineto + linesPerProp; 
        tempProp = tempProp->next; 
    	}
    }
    return lineto; 
}

/*  Description: 
    Arguments: 
    Return value:  */
CalStatus writeSubComps (const CalComp * subComp, FILE * const icsfile) {
	static CalStatus current = {0,0,OK}; 

    for (int i = 0; i < subComp->ncomps; i++) { 
        if ((fprintf (icsfile, "BEGIN:%s\r\n", subComp->comp[i]->name)) < 0) {
        	fprintf (stderr, "IOERR\n"); 
        } 
        writeSubProps(icsfile, subComp->comp[i]->prop, 0); 
        current = writeSubComps(subComp->comp[i], icsfile);         
    }

    if (strcmp(subComp->name, "VCALENDAR") != 0) {
    	if ((fprintf (icsfile, "END:%s\r\n", subComp->name)) < 0) {
    		fprintf (stderr, "IOERR\n"); 
    	}
    }

	return current; 
}

/*  Description: builds an array of prop names that begin with 'X' which are 
		then sorted by calExtract 
    Arguments: 
    	prop, current prop to be added or not added 
    	propArray, array of propNames only including those that begin with "X"
    	size, current number of names in propArray 
    Return value: current number of names in propArray */
int getPropArray (CalProp * prop, char * propArray[], int size) {
    CalProp * tempPtr = prop; 
    int sameCheck = 0; 

    while (tempPtr != NULL) { // put into an array of props in order to sort 
        if (tempPtr->name[0] == 'X') {
        	for (int i = 0; i < size; i++) {
        		if (strcmp(propArray[i], tempPtr->name) == 0) {
        			sameCheck = 1; 
        		}
        	}

        	if (sameCheck == 0) {
				propArray[size] = tempPtr->name; 
				size ++; 
        	}
        }
        sameCheck = 0; 
        tempPtr = tempPtr->next; 
    }
    return size; 
}

/*  Description: works with getPropArray to traverse through props and 
		build an array of prop names 
    Arguments: 
    	tempComp, 
    	propArray, 
    Return value: current number of names in propArray */
int searchProps (const CalComp * tempComp, char * propArray[]) {
	static int top = 0; 
	static int size = 0; 

	if (top == 0) {
		size = getPropArray(tempComp->prop, propArray, size); 
		top = 1; 
	}

	for (int i = 0; i < tempComp->ncomps; i++) {
		size = getPropArray(tempComp->comp[i]->prop, propArray, size); 
		size = searchProps(tempComp->comp[i], propArray); 
	} 
	return size; 
}

/*  Description: parses a string and counts how many tokens it will contain 
		this is so that it is easy for main to malloc an accurate number of tokens 
    Arguments:  
    Return value:  */
int countTokens (char input[]) { 
	char * tokenCount; 
	int count = 0; 

	tokenCount = malloc(sizeof(char)*(strlen(input)+1)); 
	assert(tokenCount != NULL); 
	strcpy(tokenCount, input); 

	tokenCount = strtok(tokenCount, " \n");  /* count arguments before parsing */
	while (tokenCount != NULL) {
		count ++; 
		tokenCount = strtok(NULL, " \n"); 
	}

	return count; 
}



/*  Description: 
    Arguments: 
    	prop, 
    	organizers, 
    	size, 
    Return value:  */
int getOrganizers (CalProp * prop, char * *organizers, int size) {
    CalProp * tempPtr = prop; 
    CalParam * tempParam; 
    char * commonName;
    int sameOrg = 0;  

    while (tempPtr != NULL) { // put into an array of props in order to sort 
        if (strcmp(tempPtr->name, "ORGANIZER") == 0) {
        	tempParam = tempPtr->param; 
    		while (tempParam != NULL) {
    			if (strcmp(tempParam->name, "CN") == 0) {
    				commonName = malloc(sizeof(char)*(strlen(tempParam->value[0])+1)); 
    				strcpy(commonName, tempParam->value[0]);  
    			} 
       			tempParam = tempParam->next; 
   			}

   			for (int i = 0; i < size; i++) {
   				if (strcmp(organizers[i], commonName) == 0) {
   					sameOrg = 1; 
   				}
   			}

   			if (sameOrg == 0) {
        		organizers[size] = malloc(sizeof(char)*(strlen(commonName))); 
        		strcpy(organizers[size], commonName); 
   				free(commonName); 
				size ++; 
        	} 
        	
        	sameOrg = 0; 
        	
        }
        tempPtr = tempPtr->next; 
    }
    return size; 
}

/*  Description: 
    Arguments: 
    tempComp, 
    organizers, 
    Return value:  */
int findOrganizers(const CalComp * tempComp, char ** organizers) {
	static int top = 0; 
	static int size = 0; 

	if (top == 0) {
		size = getOrganizers(tempComp->prop, organizers, size); 
		top = 1; 
	}

	for (int i = 0; i < tempComp->ncomps; i++) {
		size = getOrganizers(tempComp->comp[i]->prop, organizers, size); 
		size = findOrganizers(tempComp->comp[i], organizers); 
	} 
	return size; 
}

int getDates(CalProp * prop, struct tm * dateArray[], int size) {
	CalProp * tempPtr = prop; 
	
	while (tempPtr != NULL) {
		if ((strcmp(tempPtr->name, "COMPLETED") == 0)||
			(strcmp(tempPtr->name, "DTEND") == 0)||
			(strcmp(tempPtr->name, "DUE") == 0)||
			(strcmp(tempPtr->name, "DTSTART") == 0)||
			(strcmp(tempPtr->name, "CREATED") == 0)||
			(strcmp(tempPtr->name, "DTSTAMP") == 0)||
			(strcmp(tempPtr->name, "LAST-MODIFIED") == 0)) {
		
			dateArray[size] = malloc(sizeof(struct tm)); 
			assert(dateArray[size] != NULL); 
			
			strptime(tempPtr->value, "%Y%m%dT%H%M%S", dateArray[size]);
			size ++ ;
		}
			
		tempPtr = tempPtr->next; 
	}
	return size; 
}

int getDatesFilter(CalProp * prop, struct tm * dateArray[], int size) {
	CalProp * tempPtr = prop; 
	
	while (tempPtr != NULL) {
		if ((strcmp(tempPtr->name, "COMPLETED") == 0)||
			(strcmp(tempPtr->name, "DTEND") == 0)||
			(strcmp(tempPtr->name, "DUE") == 0)||
			(strcmp(tempPtr->name, "DTSTART") == 0)) {
		
			dateArray[size] = malloc(sizeof(struct tm)); 
			assert(dateArray[size] != NULL); 
			strptime(tempPtr->value, "%Y%m%dT%H%M%S", dateArray[size]);
			size ++ ;
		}
			
		tempPtr = tempPtr->next; 
	}
	return size; 
}

int traverseComps (const CalComp * tempComp, struct tm * dateArray[]) {
	static int top = 0; 
	static int size = 0; 
	
	if (top == 0) {
		size = getDates (tempComp->prop, dateArray, size); 
		top = 1; 
	}
	
	for (int i = 0; i < tempComp->ncomps; i++) {
		if (strcmp(tempComp->name, "VTIMEZONE") != 0) {
		size = getDates(tempComp->comp[i]->prop, dateArray, size);
		} 
		size = traverseComps(tempComp->comp[i], dateArray); 
	}
	return size; 
}


int compareAlpha (const void * element1, const void * element2) {
	return strcmp(*(char**)element1, *(char**)element2); 
}

int compareEventDates (const void * element1, const void * element2) {
	CalEvent * event1 = (CalEvent*)element1; 
	CalEvent * event2 = (CalEvent*)element2;

	if (event1->tm.tm_year > event2->tm.tm_year) { // compare years 
		return 1; 
	} else if (event1->tm.tm_year < event2->tm.tm_year) {
		return -1; 
	} else {
		if (event1->tm.tm_mon > event2->tm.tm_mon) { // compare months 
			return 1; 
		} else if (event1->tm.tm_mon < event2->tm.tm_mon) {
			return -1; 
		} else {
			if (event1->tm.tm_mday > event2->tm.tm_mday) { // compare days 
				return 1; 
			} else if (event1->tm.tm_mday < event2->tm.tm_mday) {
				return -1; 
			} else {
				if (event1->tm.tm_hour > event2->tm.tm_hour) { // compare hours 
					return 1; 
				} else if (event1->tm.tm_hour < event2->tm.tm_hour) {
					return -1; 
				} else {
					if (event1->tm.tm_min > event2->tm.tm_hour) { // compare minutes  
						return 1; 
					} else if (event1->tm.tm_min < event2->tm.tm_hour) {
						return -1; 
					} else {
						if (event1->tm.tm_sec > event2->tm.tm_sec) { // compare seconds 
							return 1; 
						} else if (event1->tm.tm_sec < event2->tm.tm_sec) {
							return -1; 
						} else {
							return 0; // they are exactly equal 
						}
					}
				}
			}
		}
	}
	return 0; 
}

int compareDates(const void * element1, const void * element2) {
	struct tm * time1 = (struct tm*)element1; 
	struct tm * time2 = (struct tm*)element2; 
	
	if (time1->tm_year > time2->tm_year) { // compare years 
		return 1; 
	} else if (time1->tm_year < time2->tm_year) {
		return -1; 
	} else {
		if (time1->tm_mon > time2->tm_mon) { // compare months 
			return 1; 
		} else if (time1->tm_mon < time2->tm_mon) {
			return -1; 
		} else {
			if (time1->tm_mday > time2->tm_mday) { // compare days 
				return 1; 
			} else if (time1->tm_mday < time2->tm_mday) {
				return -1; 
			} else {
				if (time1->tm_hour > time2->tm_hour) { // compare hours 
					return 1; 
				} else if (time1->tm_hour < time2->tm_hour) {
					return -1; 
				} else {
					if (time1->tm_min > time2->tm_hour) { // compare minutes  
						return 1; 
					} else if (time1->tm_min < time2->tm_hour) {
						return -1; 
					} else {
						if (time1->tm_sec > time2->tm_sec) { // compare seconds 
							return 1; 
						} else if (time1->tm_sec < time2->tm_sec) {
							return -1; 
						} else {
							return 0; // they are exactly equal 
						}
					}
				}
			}
		}
	}
	return 0; 
}

void rangeOfDates (struct tm * dateArray[], int size, struct tm * min, struct tm * max) {
	int compareMin;
	int compareMax; 
	
	*min = *dateArray[0]; 
	*max = *dateArray[0]; 
	for (int i = 0; i < size; i++) {
		compareMin = compareDates (min, dateArray[i]);  
		if (compareMin == 1){
			*min = *dateArray[i];  
		}
		
		compareMax = compareDates(max, dateArray[i]); 
		if (compareMax == -1) {
			*max = *dateArray[i]; 
		}
	}

 }


/***************************************************************************************************/
/***************************************************************************************************/


/***************************************************************************************************/

CalStatus calInfo( const CalComp *comp, int lines, FILE *const txtfile ) {
	CalStatus current;
	int types[] = {0, 0, 0, 0, 0, 0};  //{comps, events, todos, other, subcomps, propCount}  
	char ** orgArray = malloc(sizeof(char*)*BUFFER) ; 
	int sizeOrgArray = 0; 
	struct tm * dateArray[BUFFER]; 
	int dateSize = 0;  
	char minToString[BUFFER]; 
	char maxToString[BUFFER]; 
	struct tm min; 
	struct tm max;

	current.code = OK; 
	current.lineto = 0; 
	current.linefrom = 0; 

	if ((fprintf (txtfile, "%d lines\n", lines)) < 0) {// total number of lines in the file 
		fprintf (stderr, "IOERR\n"); 
	}

	countComps(comp, types); // number of components (broken out by events, to-do items, and other)
	if (types[0] == 1) { // print components 
		fprintf (txtfile, "%d component: ", types[0]); 
	} else {
		fprintf (txtfile, "%d components: ", types[0]); 
	}
	
	if (types[1] == 1) { // print number of events 
		fprintf (txtfile, "%d event, ", types[1]);  
	} else {
		fprintf (txtfile, "%d events, ", types[1]); 
	}
	
	if (types[2] == 1) { // printf number of todo's 
		fprintf (txtfile, "%d todo, ", types[2]); 
	} else {
		fprintf (txtfile, "%d todos, ", types[2]); 
	}
	
	if (types[3] == 1) { // printf number of others 
		fprintf (txtfile, "%d other\n", types[3]); 
	} else {
		fprintf (txtfile, "%d others\n", types[3]); 
	}
	

	if (types[4] == 1) { // print the number of subcomponents 
		fprintf (txtfile, "%d subcomponent\n", types[4]); 
	} else {
		fprintf (txtfile, "%d subcomponents\n", types[4]); 
	}
	

	types[5] = countProps(comp, 0); 
	if (types[5] == 1) { // print the number of properties 
		fprintf (txtfile, "%d property\n", types[5]); 
	} else {
		fprintf (txtfile, "%d properties\n", types[5]); 
	}

	// range of dates 
	dateSize = traverseComps(comp, dateArray); 
	if (dateSize == 0) {
		fprintf (txtfile, "No dates\n"); 
	} else {
		rangeOfDates (dateArray, dateSize, &min, &max); 
		strftime(minToString, sizeof(minToString), "%Y-%b-%d", &min); // reformat date 
		strftime(maxToString, sizeof(maxToString), "%Y-%b-%d", &max); // reformat date 
		if ((fprintf (txtfile, "From %s to %s\n", minToString, maxToString)) < 0) {
			fprintf (stderr, "IOERR\n"); 
		}
	}
	
	// sorted list of organizers 
	sizeOrgArray = findOrganizers(comp, orgArray); 
	if (sizeOrgArray == 0) {
		fprintf (txtfile, "No organizers\n"); 
	} else {
		fprintf (txtfile, "Organizers:\n"); 
		qsort(orgArray, sizeOrgArray, sizeof(char*), compareAlpha);
		for (int i = 0; i < sizeOrgArray; i++) {
			fprintf (txtfile, "%s\n", orgArray[i]); 
		}
	}

	return current; 
}

/***************************************************************************************************/
CalStatus calExtract( const CalComp *comp, CalOpt kind, FILE *const txtfile ) {
	CalStatus current; 
	char * propArray[BUFFER]; 
	int sizeOfArray = 0; 
	CalProp * tempProp; 
	CalEvent eventArray[comp->ncomps];  
	char format[BUFFER]; 
	char * tempSummary = NULL; 
	int count = 0; 

	current.code = OK; 
	current.linefrom = 0; 
	current.lineto = 0; 

	if (kind == OEVENT) {// e - events, sorted in date order 
		// a textfile, also sort them in order of date 
		for (int i = 0; i < comp->ncomps; i++) {
			if (strcmp(comp->comp[i]->name, "VEVENT") == 0) {
				tempProp = comp->comp[i]->prop; 
    			
    			while (tempProp != NULL) { // put into an array of props in order to sort 
       				if (strcmp(tempProp->name, "DTSTART") == 0) { 
       					strptime(tempProp->value, "%Y%m%dT%H%M%S", &eventArray[count].tm); 
       					// fix the leading zero for hours 
       					strftime(format, sizeof(format), "%Y-%b-%d %-I:%M %p: ", &eventArray[count].tm); // reformat date 
        			
        			} else if (strcmp(tempProp->name, "SUMMARY") == 0) {
        				tempSummary = malloc(sizeof(char)*(strlen(tempProp->value))); 
        				strcpy(tempSummary, tempProp->value); 
        				eventArray[count].summary = malloc(sizeof(char)*(strlen(tempProp->value))); 
        				assert(eventArray[count].summary != NULL); 
        				strcpy(eventArray[count].summary, tempSummary); 
        			}
        			tempProp = tempProp->next; 
    			}

    			if (strlen(tempSummary) == 0) { // if this event does not have a summary 
    				tempSummary = malloc(sizeof(char)*(strlen("(na)"))); 
    				assert(tempSummary != NULL); 
    				strcpy(tempSummary, "(na)"); 
    				eventArray[count].summary = malloc(sizeof(char)*(strlen("(na)"))); 
    				assert(eventArray[count].summary != NULL); 
        			strcpy(eventArray[count].summary, "(na)"); 
    			}

    			eventArray[count].toString = malloc(sizeof(char)*((strlen(format)+strlen(tempSummary)))); 
    			assert(eventArray[count].toString != NULL); 
    			strcpy(eventArray[count].toString, format); 
    			strcat(eventArray[count].toString, tempSummary); 
    			strcpy(tempSummary, "");  
				count ++; 
			}
		}

		qsort(eventArray, count, sizeof(CalEvent), compareEventDates); // sort array by dates 
		for (int j = 0; j < count; j++) {
			if ((fprintf (txtfile, "%s\n", eventArray[j].toString)) < 0 ) {
				fprintf (stderr, "IOERR\n"); 
			}
		}

	} else if (kind == OPROP) { // x - properties, printed alphabetically (qsort) 
		sizeOfArray = searchProps(comp, propArray); 
		qsort(propArray, sizeOfArray, sizeof(char*), compareAlpha); //sort the prop names alpha
		for (int i = 0; i < sizeOfArray; i++) { // still needs to be sorted 
			if ((fprintf (txtfile, "%s\n", propArray[i])) < 0) {
				fprintf (stderr, "IOERR\n"); 
			} 
		}
		
	} 

	return current; 
}


/***************************************************************************************************/
CalStatus calFilter( const CalComp *comp, CalOpt content, time_t datefrom, time_t dateto, FILE *const icsfile ){
	CalStatus current; 
	time_t tempInt; 
	int sizeOfArray = 0; 
	int valid = 0; 
	int count = 0; 
	int validCount = 0; 
	CalProp * tempProp; 

	current.code = OK; 
	fprintf (icsfile, "BEGIN:VCALENDAR\r\n"); 
	writeSubProps(icsfile, comp->prop, 0);  

	if (content == OEVENT) {
		for (int i = 0; i < comp->ncomps; i++) {
			if (strcmp(comp->comp[i]->name, "VEVENT") == 0) {
				count++; 
				struct tm * dateArray[BUFFER]; // local array 
				sizeOfArray = 0; 
				valid = 0; 
				tempProp = comp->comp[i]->prop; 
				
				sizeOfArray = getDatesFilter(tempProp, dateArray, sizeOfArray); //get array of dates within that event 
				for (int i = 0; i < sizeOfArray; i++) {
					tempInt = mktime(dateArray[i]); 
					if ((tempInt > datefrom)&&(tempInt < dateto)) {
						valid = 1; 
					}
				}

				if ((valid == 1)||(datefrom == 0)) {
					validCount++; 
					current = writeCalComp(icsfile, comp->comp[i]); 
				}  
			}
		}
		
		if ((count == 0)||(validCount == 0)) {
			current.code = NOCAL; 
			return current; 
		}
	} else if (content == OTODO) {
		for (int i = 0; i < comp->ncomps; i++) {
			if (strcmp(comp->comp[i]->name, "VTODO") == 0) {
				count++; 
				struct tm * dateArray[BUFFER]; // local array 
				sizeOfArray = 0; 
				valid = 0; 
				
				sizeOfArray = getDatesFilter(comp->comp[i]->prop, dateArray, sizeOfArray); //get array of dates within that event 
				for (int j = 0; j < sizeOfArray; j++) {
					tempInt = mktime(dateArray[j]); 
					if ((tempInt > datefrom) && (tempInt < dateto)) {
						valid =1; 
					}
				}
				
				if ((valid == 1)||(datefrom == 0)) {
					validCount++; 
					current = writeCalComp(icsfile, comp->comp[i]); 
				}
			}
		}
		
		if ((count == 0)||(validCount == 0)) {
			current.code = NOCAL; 
			return current; 
		}
	}

	fprintf (icsfile, "END:VCALENDAR\r\n"); 
	return current; 
}

/***************************************************************************************************/
CalStatus calCombine(const CalComp *comp1, const CalComp *comp2, FILE *const icsfile ){
    static CalStatus current = {0, 0, OK}; 
	// can only have a single prodid and version 
    if ((fprintf (icsfile, "BEGIN:%s\r\n", comp1->name)) < 0) {
    	fprintf (stderr, "IOERR\n"); 
    } 

    writeSubProps(icsfile, comp1->prop, 0); 
    current = writeSubComps(comp1, icsfile); 
    current = writeSubComps(comp2, icsfile); 

    if ((fprintf(icsfile, "END:%s\r\n", comp1->name)) < 0) {
    	fprintf (stderr, "IOERR\n"); 
    } 

	return current; 
}

/***************************************************************************************************/
int main (int argc, char*argv[]) {
	FILE * comp1File; 
	CalComp * tComp; 
	CalStatus tStatus; 
	CalComp * comp1; 
	CalComp * comp2; 
	CalStatus status1; 
	CalStatus status2; 
	struct tm datefrom;
	struct tm dateto; 
	struct tm * todayfrom;
	struct tm * todayto;  
	time_t datefromInt = 0; 
	time_t datetoInt = 0; 
	
	if (argc == 1) {
		return EXIT_FAILURE; 
	}
	 
	if (argc > 1) {
		if (strcmp(argv[1], "-info") == 0) {
			if (argc != 2) {
				return EXIT_FAILURE; 
			}
			tStatus = readCalFile(stdin, &tComp); 
			tStatus = calInfo(tComp, tStatus.lineto, stdout); 
			//freeCalComp(tComp); 
	
		} else if (strcmp(argv[1], "-extract") == 0) {
			if (argc != 3) {
				return EXIT_FAILURE; 
			}
			
			tStatus = readCalFile(stdin, &tComp);
			if (strcmp(argv[2], "e") == 0) {
				tStatus = calExtract(tComp, OEVENT, stdout); 
			} else if (strcmp(argv[2], "x") == 0) {
				tStatus = calExtract(tComp, OPROP, stdout); 
			} else {
				tStatus.code = IOERR; 
			}
			//freeCalComp(tComp); 
	
		} else if (strcmp(argv[1], "-filter") == 0) {
			
			tStatus = readCalFile(stdin, &tComp); 
			// get current time: time (&datefrom);  
			for (int i = 0; i < argc; i++) {
				
				if (strcmp(argv[i], "from") == 0) { // get datefrom 
					if (strcmp (argv[i+1], "today") == 0) {
						
						time (&datefromInt); 
						todayfrom = localtime(&datefromInt); 
						todayfrom->tm_hour  = 0; 
						todayfrom->tm_min = 0; 
						todayfrom->tm_sec = 0; 
						datefromInt = mktime(todayfrom); 
		
					} else {
						if ((getdate_r(argv[i+1], &datefrom)) == 0) {
						//if (!(strptime(argv[i+1], "%B %d, %Y", &datefrom))){
							return EXIT_FAILURE; 
						}
						
						datefrom.tm_hour = 0; 
						datefrom.tm_min = 0; 
						datefrom.tm_sec = 0; 
						
						datefromInt = mktime(&datefrom); 

					}
				} 

				if (strcmp(argv[i], "to") == 0) { // get dateto 
					if (datefromInt == 0) { // from has not been set yet, this is an error 
						return EXIT_FAILURE; 
					}
				
					if (strcmp (argv[i+1], "today") == 0) {
						time (&datetoInt); 
						todayto = localtime(&datetoInt); 
						todayto->tm_hour = 23; 
						todayto->tm_min = 59;
						todayto->tm_sec = 0; 
						datetoInt = mktime(todayto);  
						
					} else {
						if (getdate_r(argv[i+1], &dateto) == 0) {
						//if (!(strptime(argv[i+1], "%B %d, %Y", &dateto))){
							return EXIT_FAILURE; 
						}
						
						dateto.tm_hour = 23; 
						dateto.tm_min = 59; 
						dateto.tm_sec = 0; 
						
						datetoInt = mktime(&dateto); 
					} 
				}
			}

			if ((datefromInt != 0)&&(datetoInt == 0)){
				return EXIT_FAILURE; 
			}
			
			if (datefromInt > datetoInt) {
				return EXIT_FAILURE; 
			}

			if (strcmp(argv[2], "t") == 0) {
				tStatus = calFilter(tComp, OTODO, datefromInt, datetoInt, stdout); 
			} else if (strcmp(argv[2], "e") == 0) {
				tStatus = calFilter(tComp, OEVENT, datefromInt, datetoInt, stdout); 
			} else {
				return EXIT_FAILURE; 
			}
	
		} else if (strcmp(argv[1], "-combine") == 0) {
			status1.code = 0; 
			if (argc != 3) { // check if there are the right number of arguments 
				return EXIT_FAILURE; 
			}
			
			if (!(comp1File = fopen(argv[2], "r"))){ // check if a file exists 
				return EXIT_FAILURE; 
			} 
			status1 = readCalFile(comp1File, &comp1); 
			status2 = readCalFile(stdin, &comp2); 
			if ((status1.code != OK)||(status2.code != OK)) {
				printf ("ERRRRORORORORO  %d \n", status1.code);
				return EXIT_FAILURE; 
			}
			
			tStatus = calCombine(comp1, comp2, stdout); 
			//printf ("combine is shit\n"); 
			fclose (comp1File); 
			
		} else {
			tStatus.code = IOERR; 
		}
	} else {
		//comp1File = fopen("small2.ics", "r"); 
		//tStatus = readCalFile (comp1File, &tComp); 
		//printf ("HI\n"); 
		//tStatus = writeCalComp(stdout, tComp); 
	}
	if (tStatus.code != OK) {
		return EXIT_FAILURE; 
	} 
	return EXIT_SUCCESS; 
}
