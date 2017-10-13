/********
calutil.c -- Public interface for iCalendar utility functions
Last updated:  April 7th 2016

Name: Lindsay Elliott
ID: 0875476
Email: lellio04
********/

#include "calutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*  Description: recursively frees a CalParam pointer. 
    Arguments:
        param, pointer to a CalParam that needs to be freed.  
    Return value: void.  */
void freeParam (CalParam * param) { /********************************************/
	CalParam * hold = NULL; 
    while (param != NULL) {
		for (int i = 0; i < param->nvalues; i++){
			free(param->value[i]); 
		} 	
		hold = param->next;
		free(param->name); 
		param = hold; 
	}
    
    free(param); 
}

/*  Description: recursively frees a CalProp pointer. 
    Arguments: 
        prop, pointer to a CalProp that needs to be freed.  
    Return value: void.  */
void freeProp (CalProp * prop) { /*****************************************/
    CalProp * current;  
    while (prop != NULL) { 
		free(prop->name); 
		free(prop->value);
		current = prop; 
		prop = prop->next; 
		freeParam(current->param); 
		free(current); 
	}
}

/*  Description: traverses through a string and makes each character 
        an upper case letter.  
    Arguments: 
        string, char* that is potentially lowercase, but will be lowercase  
    Return value: void.  */
void upperString (char * string) { /*********************************************************/
    int i = 0; 
    while (string[i] != '\0') {
        string[i] = toupper(string[i]); 
        i++; 
    }
}

/*  Description: function traverses through first layer of CalProps in search not only 
        version, but the correct version specified in calutil.h 
    Arguments: 
        check, main CalComp (top layer) 
    Return value: OK calError if only 1 is found, BADVER calerror code otherwise */
CalError versionCheck (CalComp * check) {
    CalProp * tempProp = check->prop; 
    int count = 0; 

    while (tempProp != NULL) {
        if ((strcmp(tempProp->name, "VERSION") == 0)&&(strcmp(tempProp->value, VCAL_VER) == 0)) {
            count++; 
        }
        tempProp = tempProp->next; 
    }

    if (count == 0) {
        return BADVER; 
    } else if (count == 1) {
        return OK; // no error 
    }
    return BADVER; 
}

/*  Description: function traverses throught first layer of CalProps in search of 
        only one prodid 
    Arguments:  
        check, main CalComp (top layer)
    Return value: OK calErro if only 1 is found, NOPROD otherwise  */
CalError prodidCheck (CalComp * check) {
    CalProp * tempProp = check->prop; 
    int count = 0; 

    while (tempProp != NULL) {
        if (strcmp(tempProp->name, "PRODID") == 0) {
            count++; 
        }
        tempProp = tempProp->next; 
    }

    if (count == 0) {
        return NOPROD; 
    } else if (count == 1) {
        return OK; // no error 
    }
    return NOPROD; 
}

/*  Description: traverse through first layer of CalComps, checking if they begin with 
        a 'V' (VEVENT, VTODO... )
    Arguments: 
        comp, main CalComp (top layer) 
    Return value: the number of subcomps beginning with 'V' */
int vCheck(CalComp * comp) {
    CalComp * tempPtr = comp; 
    static int count = 0; 

    for (int i = 0; i < comp->ncomps; i++) {
        if ((tempPtr)->comp[i]->name[0] != 'V') {
            count ++; 
        }
        vCheck(tempPtr->comp[i]); 
    } 
    return count; 
}

/*  Description: writes a line to file and folds it if needed 
    Arguments: 
        ics, file to append the line to the end of 
        line, string to write to the file and fold if over specified FOLD_LEN guidelines 
    Return value: should return the number of lines written to the file */
int writeLine (FILE *const ics, char * line) { /*FIX THIS: increase return number of lines written */
    if (strlen(line) > FOLD_LEN) {
        for (int i = 0; i < strlen(line); i++) {
            fprintf (ics, "%c", line[i]); 
            if ((i%(FOLD_LEN-1) == 0)&&(i != 0)&&((i+1) != strlen(line))) {
                fprintf (ics, "\r\n "); 
            } 
        }
    } else {
        fprintf(ics, "%s", line); 
    }
    fprintf (ics, "\r\n"); 
    return 0; 
}

/*  Description: concantonates any params and their values to pString 
    Arguments: 
        ics, file to write to
        param, current param to be formatted properly  
    Return value: void */
char * writeParams(FILE *const ics, CalParam * param)  {
    CalParam * tempParam = param; 
    char * pString; 
    pString = malloc(sizeof(char)*BUFFER); 
    strcpy(pString, "\0"); 

    while (tempParam != NULL) {
        strcat(pString, ";"); 
        strcat(pString, tempParam->name); 
        strcat(pString, "="); 
        strcat(pString, tempParam->value[0]); 
        for (int i = 1; i < (tempParam->nvalues); i++) {
			strcat(pString, ","); 
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
int writeProps(FILE *const ics, CalProp * prop) {
    CalProp * tempProp = prop;
    char * line = NULL; 
    char pString[BUFFER]; 
    int lineto = 0; 
    int linesPerProp = 0; 

    while (tempProp != NULL) {
        line = malloc(sizeof(char)*(BUFFER)); 
        strcpy(line, tempProp->name); 
        
        if (tempProp->nparams > 0) {
            strcpy(pString, writeParams(ics, tempProp->param)); 
            //line = realloc(line, sizeof(char)*(strlen(line)+strlen(pString)));  
            strcat(line, pString); 
        }

        //line = realloc(line, sizeof(char)*(strlen(line)+strlen(tempProp->value)+1)); 
        strcat(line, ":"); 
        strcat(line, tempProp->value);

        linesPerProp = writeLine(ics, line); 
        lineto++; 
        lineto = lineto + linesPerProp; 
        tempProp = tempProp->next; 
    }
    return lineto; 
}

/*********************************************************************************************/

CalStatus readCalFile( FILE *const ics, CalComp **const pcomp ){ /**************************/
    CalStatus current; 
    CalError verCheck = 0; 
    CalError prodCheck = 0; 
    int vCount = 0; 
    int size; 

	//THIS WONT WORK WITH STDOUT OR STDIN  ..... FIX IT L8TR 
    fseek(ics, 0, SEEK_END);  // check if the file is empty 
    size = ftell(ics);
    fseek(ics, 0, SEEK_SET); 

    if (size == 0) {
        current.code = NOCAL; 
        current.lineto = 0; 
        current.linefrom = 0; 
        return current; 
    }


    current.code = OK; 
    current.linefrom = 0; 
    current.lineto = 0; 

    *pcomp = malloc(sizeof(CalComp)); 
    (*pcomp)->name = NULL; 
    (*pcomp)->nprops = 0;
    (*pcomp)->prop = NULL; 
    (*pcomp)->ncomps = 0;  

    if (ics == NULL) {
        current.code = NOCAL; 
        return current; 
    }
 
    current = readCalLine(NULL, NULL); /* initialize values */ 
    current = readCalComp(ics, pcomp); 


    if ((*pcomp)->name == NULL) {
        current.lineto = 1; 
        current.linefrom = 1;
        current.code = NOCAL; 
        // freee props 
    }
 
    vCount = vCheck(*pcomp); 
    if (vCount != 0) {
        current.code = NOCAL; 
    }


    if ((*pcomp)->ncomps == 0) {
        current.code = NOCAL; 
    }

    prodCheck = prodidCheck(*pcomp); 
    if (prodCheck != 0) {
        current.code = NOPROD; 
    }

    verCheck = versionCheck(*pcomp); 
    if (verCheck != 0) {
        current.code = BADVER; 
    }

    return current;         
}

CalStatus readCalComp( FILE *const ics, CalComp **const pcomp ){ /**************************/
    CalStatus current; 
    char * buffer; 
    static int count = 1; 
    CalProp* curProp; 
    static int nestNum = 1; 
    CalProp * head; 
    int tempLine; 

    current.code = OK; 
    current.linefrom = 0; 
    current.lineto = 0; 

    while (!feof(ics)){ //loop until end of file 
        current = readCalLine(ics, &buffer);
        //printf ("%s\n", buffer); 
        
        if (strcmp(buffer, "") != 0) {
            curProp = malloc(sizeof(CalProp)); 
            current.code = parseCalProp(buffer, curProp); 
            
            free (buffer); 
            
            if ((count == 1)&&(strcmp(curProp->name, "BEGIN") == 0)) {
                if (strcmp(curProp->value, "VCALENDAR") != 0) {
                    current.code = NOCAL; 
                    return current; 
                }

                (*pcomp)->name = malloc(sizeof(char)*(strlen(curProp->value)+1)); 
                strcpy((*pcomp)->name, curProp->value); 
                upperString((*pcomp)->name); 
                (*pcomp)->ncomps = 0; 
                count ++; 

            } else {
                
                if (strcmp(curProp->name, "BEGIN")==0) { // new comp, add to flexible array 
                    tempLine = current.linefrom; 
                    nestNum++; 

                    (*pcomp) = realloc((*pcomp), sizeof(CalComp) + (((*pcomp)->ncomps+2) * sizeof(CalComp*))); 
                    
                    (*pcomp)->comp[(*pcomp)->ncomps] = malloc(sizeof(CalComp));
                    (*pcomp)->comp[(*pcomp)->ncomps]->name = malloc(sizeof(char)*(strlen(curProp->value)+1)); 
                    strcpy((*pcomp)->comp[(*pcomp)->ncomps]->name, curProp->value);
                    upperString((*pcomp)->comp[(*pcomp)->ncomps]->name); 
                    (*pcomp)->comp[(*pcomp)->ncomps]->nprops = 0; 
                    (*pcomp)->comp[(*pcomp)->ncomps]->ncomps = 0; 
                    (*pcomp)->ncomps++; 

                    current = readCalComp(ics, &(*pcomp)->comp[(*pcomp)->ncomps-1]); 

                } else if (strcmp(curProp->name, "END")==0) {
                    upperString(curProp->value); 

                    if (nestNum > 3) {
                        current.code = SUBCOM; 
                        current.linefrom = tempLine; 
                        return current; 
                    }
                    nestNum --; 

                    if (((*pcomp)->ncomps == 0)&&((*pcomp)->nprops == 0)) {
                        current.code = NODATA; 
                    }

                    if (strcmp((*pcomp)->name, curProp->value) != 0) {
                        current.code = BEGEND; 
                    }

                    if (strcmp(curProp->value, "VCALENDAR") == 0) {
                        count = 1; // reset for next comp to be read 
                        nestNum = 1; 
                    } 

                    free(curProp->name); 
                    free(curProp); 
                    return (current); 

                } else { // new prop, add to linked list  

                    if ((*pcomp)->nprops == 0) { 
                        (*pcomp)->prop = malloc(sizeof(CalProp)); 
                        (*pcomp)->prop->name = malloc(sizeof(char)*(strlen(curProp->name)+1)); 
                        strcpy((*pcomp)->prop->name, curProp->name);
                        (*pcomp)->prop->value = malloc(sizeof(char)*(strlen(curProp->value)+1)); 
                        strcpy((*pcomp)->prop->value, curProp->value); 
                        (*pcomp)->prop->nparams = curProp->nparams; 
                        (*pcomp)->prop->param = curProp->param; 
                        (*pcomp)->prop->next = NULL; 
                        (*pcomp)->nprops++; 

                        head = (*pcomp)->prop; 

                    } else { 
                        (*pcomp)->prop = head; 
 
                        if((*pcomp)->prop != NULL) {
                            while ((*pcomp)->prop->next != NULL) {
                                (*pcomp)->prop = (*pcomp)->prop->next; 
                            }
                        }
                    
                        (*pcomp)->prop->next = malloc(sizeof(CalProp)); 
                        (*pcomp)->prop = (*pcomp)->prop->next;
                        (*pcomp)->prop->nparams = curProp->nparams; 
                        (*pcomp)->prop->param = curProp->param;
                        (*pcomp)->prop->next = NULL; 
                        (*pcomp)->prop->name = malloc(sizeof(char)*(strlen(curProp->name)+1)); 
                        strcpy((*pcomp)->prop->name, curProp->name); 
                        (*pcomp)->prop->value = malloc(sizeof(char)*(strlen(curProp->value)+1)); 
                        strcpy((*pcomp)->prop->value, curProp->value); 
                        (*pcomp)->nprops++;  

                        (*pcomp)->prop = head; 
                    }
                }
            }
        }
    }

    current.code = BEGEND; 
    return current; 
}

CalStatus readCalLine( FILE *const ics, char **const pbuff ){ /**************************/
    static CalStatus current = {OK, -1, -1};
    char tempRead[BUFFER]; 
    static char foldBuffer[BUFFER]; 
    static int tempLineFrom = 0; 
    static int resetFrom = 0; 

    if (resetFrom == 1) { /*coming back from folded line, sync up lineto and linefrom*/ 
        current.linefrom = tempLineFrom; 
        current.code = OK; 
        resetFrom = 0; 
    }
    
    if (ics == NULL) {
        current.code = OK; 
        current.linefrom = -1; 
        current.lineto = -1; 
        return current; 
    } 

    fgets(tempRead, BUFFER, ics);
    current.lineto++;  
    current.linefrom++; 
    
    /* proper end of line formatting check */ 
    if ((strchr(tempRead, '\n') == NULL)||(strchr(tempRead, '\r') == NULL)) {
       //printf ("NOCRNL\n"); 
    } 

    if (strchr(tempRead, '\n') != NULL) {
        tempRead[strlen(tempRead)-1] = '\0'; // remove end character  
    } 

    if (strchr(tempRead, '\r') != NULL) {
        tempRead[strlen(tempRead)-1] = '\0'; 
    }

    if ((tempRead[0] == ' ')||(tempRead[0] == '\t')||(foldBuffer[0] == '\0')) { /* UNFOLDING */
        if (foldBuffer[0] != '\0') {
            for (int i = 0; i < strlen(tempRead); i++) {
                tempRead[i] = tempRead[i+1]; 
            }
            current.linefrom--; 
        } 
        
        strcat(foldBuffer,tempRead); 
        *pbuff = malloc(sizeof(char)); 
        strcpy(*pbuff, ""); 

    } else {
        *pbuff = malloc(sizeof(char)*(strlen(foldBuffer)+1));
        strcpy(*pbuff, foldBuffer); 
        strcpy(foldBuffer, tempRead); 

        tempLineFrom = current.lineto; 
        resetFrom = 1;  
        return current; 
    }   

    return readCalLine(ics, pbuff); 
}

CalError parseCalProp( char *const buff, CalProp *const prop ){ /**************************/
    char * token; 
    char * tempToken; 
    char * tempDelim;
    char * tempValue;
    char * tempBuffer; 
    static CalParam * head;
    char curDelim;
    char curDelim2;  
    int inQuote = 0; 

    CalError error = OK; 

    tempBuffer = malloc(sizeof(char) * (strlen(buff) + 1));
    strcpy(tempBuffer, buff);

    (*prop).name = NULL; 
    (*prop).value = NULL; 
    (*prop).nparams = 0; 
    (*prop).param = NULL; 
    (*prop).next = NULL; 

    tempValue = malloc(sizeof(char)*(strlen(buff)+1));
    strcpy(tempValue, buff); 

    tempDelim = malloc(sizeof(char)*(strlen(buff)+1)); 
    strcpy(tempDelim, buff);

    tempToken = NULL; 

    token = strtok(tempBuffer, ":;"); /* find property name */
    (*prop).name = malloc(sizeof(char)*(strlen(token)+1)); 
    strcpy((*prop).name, token); 
    upperString((*prop).name); 
 

    curDelim = tempDelim[token-tempBuffer+(strlen(token))]; //find the delim used 
    if (curDelim == ';') { 
        token = strtok(NULL, "\n"); 
        int i = 0; 
        char * tempParams = token; 
        char * inQuote; 

        while ((token[i] != ':')&&(token[i] != '\"')) { //find first of " or : 
            i++; 
        }

        if (token[i] == ':') { // no quotes 
            if (strchr(token, ':') == NULL) {
                return SYNTAX; 
            }

            token = strtok(token, ":"); 
            tempToken = malloc(sizeof(char)*(strlen(token)+1)); 
            strcpy(tempToken, token); 
            token = strtok(NULL, "\n"); // get the property value 
        } else if (token[i] == '\"') {
            inQuote = &token[i]; 

            int j = 1; 
            while (inQuote[j] != '\"') { //find the second quote 
                j++; 
            }

            int n = 0; 
            while (inQuote[n+j] != ':') { //find the end of the params 
                n++; 
            }

            tempToken = malloc(sizeof(char)*(j+i+n)); 
            for (int k = 0; k < (j+i+n); k++) { // copy string of params to be parsed 
                tempToken[k] = tempParams[k];   
            }

            token = &inQuote[j]; 
            token = strtok(&inQuote[j], ":");
            token = strtok(NULL, "\n");  
        }

    } else if (curDelim == ':') {
        token = strtok(NULL, "\n"); 
        if (token == NULL) {
            return OK; 
        }
       
    } else {
        return SYNTAX; 
    }

    (*prop).value = malloc(sizeof(char)*(strlen(token)+1)); 
    strcpy((*prop).value, token); 

    if (tempToken != NULL) { // if there are no parameters 
		tempDelim = malloc(sizeof(char)*(strlen(tempToken)+1)); 
		strcpy(tempDelim, tempToken);
        token = strtok(tempToken, ";=,");

        (*prop).param = malloc(sizeof(CalParam) + sizeof(char)); 
        (*prop).param->name = malloc(sizeof(char)*(strlen(token)+1)); 
        strcpy((*prop).param->name, token); 
        upperString((*prop).param->name); 
        (*prop).param->next = NULL; 
        *(*prop).param->value = malloc(sizeof(char*)*50); 
        (*prop).param->nvalues = 0; 
        (*prop).nparams = 1; 
        
        head = (*prop).param; 

        while (token != NULL) { 
            curDelim = tempDelim[token-tempToken+(strlen(token))]; //find the delim used 
            curDelim2 = tempDelim[token-tempToken+(strlen(token))+1];
            if ((curDelim == '\"')||(curDelim2 == '\"')) {
                if (inQuote == 0) {
                    inQuote = 1; 
                } else {
                    inQuote = 0; 
                }
            } 
            
            token = strtok(NULL, ";=,"); 

             if ((curDelim == '=')||(curDelim == ',')) { // new param value, add to flexible array 
				head = (*prop).param; 
                while ((*prop).param->next != NULL) { // new param, find end of linked list to add on
                    (*prop).param = (*prop).param->next;
                }

				if ((*prop).param->nvalues > 0) {
                (*prop).param = realloc ((*prop).param, sizeof(CalParam) + ((((*prop).param->nvalues)+1) * sizeof(char*))); 
				} 	
                
                (*prop).param->value[(*prop).param->nvalues] = malloc(sizeof(char)*(strlen(token)+1)); 
                strcpy((*prop).param->value[(*prop).param->nvalues], token); 
                (*prop).param->nvalues ++; 

                (*prop).param = head; 

            } else if (curDelim == ';') { // new param, add to linked list 

                (*prop).param = head; 
                (*prop).nparams++; 
                    
                while ((*prop).param->next != NULL) { // new param, find end of linked list to add on
                    (*prop).param = (*prop).param->next;
                }
                
                (*prop).param->next = malloc(sizeof(CalParam)); 
                (*prop).param = (*prop).param->next; 
                (*prop).param->next = NULL; 
                (*prop).param->name = malloc(sizeof(char)*(strlen(token)+1)); 
                upperString(token); 
                strcpy((*prop).param->name, token); 
                *(*prop).param->value = malloc(sizeof(char*)*50); 
                (*prop).param->nvalues = 0; 
                
                (*prop).param = head;
            } else {
                return OK; 
            }
        } 
    }
    return error; 
}

CalStatus writeCalComp( FILE *const ics, const CalComp * comp ) { /********************* writeCalComp ***/
    const CalComp * tempPtr = comp; 
    static CalStatus current = {0, 0, OK}; 
    static int top = 0;  
    static char * tempName; 
    int propNum;  

    if (top == 0) {
        tempName = malloc(sizeof(char)*(strlen(comp->name))); 
        strcpy(tempName, comp->name); 
        if (fprintf (ics, "BEGIN:%s\r\n", comp->name) < 0) {
			current.code = IOERR; 
			return current; 
		}
        current.lineto = 1; 
        current.linefrom = 1; 
        current.lineto++; 
        current.lineto = current.lineto + writeProps(ics, comp->prop); 
        top = 1; 
    }

    for (int i = 0; i < comp->ncomps; i++) { 
        fprintf (ics, "BEGIN:%s\r\n", tempPtr->comp[i]->name); 
        current.lineto ++; 

        propNum = writeProps(ics, tempPtr->comp[i]->prop); 
        current.lineto = current.lineto + propNum; 

        current = writeCalComp(ics, tempPtr->comp[i]);
    } 

    if (fprintf(ics, "END:%s\r\n", comp->name) < 0) { 
		current.code = IOERR; 
		return current; 
	} 
    if (strcmp(tempName, comp->name) == 0) {
        top = 0; 
    }

    current.lineto++; 
    current.linefrom = current.lineto; // check if these should be equal for this fucntion @@@ 

    return current; 
}

void freeCalComp( CalComp *const comp ){ /**************************/
    /*freeCalComp calls freeProp which calls freeParam recursively */
	for (int i = 0; i < comp->ncomps; i++){
		free(comp->comp[i]->name); 
		freeProp(comp->comp[i]->prop); 
	} 
}
    
