/************************************
* Michael Ames
* Grep Simulator - Final Project
* CIS 361: System Programming
* Summer 2017 - GVSU
* Instructor: Dr. Vijay Bhuse
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <regex.h>
#include <sys/stat.h>

#define SIZE 1024
#define INVALID_INPUT 0
#define FILE_NOT_FOUND 1
#define NULL_DIRECTORY 2
#define REGEX_COMPILE_ERROR 3
#define NO_FLAG 'z'

void checkFile(char *path, char *query, char flag);
void readStdIn();
void parseArgs(int argc, char *argv[]);
void handleError(int errorCode);
void wholeWord(char *path, char *query);
void wholeLine(char *path, char *query);
bool isWCC(char c);

static char flag;
static char query[SIZE] = "";
static char *files[SIZE] = {NULL};

int main(int argc, char *argv[]) {
   
    parseArgs(argc, argv);
   
	int i;
	i = 0;
    while(files[i] != 0) {
    if (flag == 'w')
        wholeWord(files[i], query);
    else if (flag == 'x')
        wholeLine(files[i], query);
    else
    	checkFile(files[i], query, flag);
    ++i;    
   }
    return 0;
}

void parseArgs(int argc, char *argv[]) {
    int i,j = 0;
    char *ptr;

    if (argc == 1)
        handleError(INVALID_INPUT);

    if(argv[argc - 1][0] == '-') //if the last argument is a flag display an error message and exit
        handleError(INVALID_INPUT);

    if (argc == 2)
        readStdIn(argv[1]);//if user enters only a query search for that query in stdin

    if(argv[argc - 1][0] == '-' || argv[argc - 2][0] == '-') //if either of the last two arguments are flags display an error message and exit
        handleError(INVALID_INPUT);

    if (argc == 3) { //no flags, only query and one filename or directory
        strcpy(query, argv[1]);
        files[0] = argv[2];
        flag = NO_FLAG;
        return;
    }

    if (argc == 4) {
        if (argv[1][0] == '-') { //if first arg is a flag the second is query and third is fileName or directory
            if (strlen(argv[1]) > 2)
                handleError(INVALID_INPUT);
            flag = argv[1][1];
            strcpy(query, argv[2]);
            files[0] = argv[3];
            files[1] = 0;
        return;
        }
        else {
            flag = NO_FLAG;
            strcpy(query, argv[1]);
            files[0] = argv[2];
            files[1] = argv[3];
            files[2] = 0;
            return;
        }
    }

     if (argv[1][0] == '-') { //if the first arg is a flag, the second arg is the query and the rest are the files to search
        if (strlen(argv[1]) > 2)
            handleError(INVALID_INPUT);
        flag = argv[1][1];
        strcpy(query, argv[2]);
        for (i = 3; i < argc; ++i) {
            files[j] = argv[i];
            ++j;
        }
        files[j+1] = 0;
        return;
    } else {
         flag = NO_FLAG;
         strcpy(query, argv[1]);
         for (i = 2; i < argc; ++i) {
            files[j] = argv[i];
            ++j;
         }
         files[j+1] = 0;
    }

    return;
}

void checkFile(char *path, char *query, char flag) {

    regex_t patternBuffer;
    int regCompileError;
    int regSearchResult;
    size_t numMatches;	

    if(flag == 'i')
        regCompileError = regcomp(&patternBuffer, query, REG_ICASE);
    else
        regCompileError = regcomp(&patternBuffer, query, 0);

    if(regCompileError)
        handleError(REGEX_COMPILE_ERROR);

    int count = 0;
    int lineNum = 1;

    FILE* f = fopen(path, "r");
    char *searchPtr;

    if (f == NULL)
        handleError(FILE_NOT_FOUND);

    char line[SIZE];

    fgets(line, SIZE, f);

    while(!feof(f)) {
    	regSearchResult = regexec(&patternBuffer, line, 0, NULL, 0);

        if (flag != 'v') {
            if (!regSearchResult) {
                if (flag == 'l') {
                    printf("%s\n", path);
                    break;
                }
                ++count;
                if (flag == 'n')
                    printf("%s:%d:%s", path, lineNum, line);
                if (flag == 'z' || flag == 'i')
                    printf("%s:%s\n", path, line);
            }
        }
        else {
            if(regSearchResult)
                printf("%s:%s", path, line);
        }
        fgets(line, SIZE, f);
        ++lineNum;
    }

    if (count > 0 && flag == 'c')
        printf("%s:%d\n", path, count);

    regfree(&patternBuffer);
    fclose(f);

    return;
}

void wholeWord(char *path, char *query) {

	regmatch_t matches[SIZE]; // pointer to array to store matches found by regexec
	
	int regCompileError;
	regex_t patternBuffer;
	
	regCompileError = regcomp(&patternBuffer, query, 0);
	if (regCompileError)
		handleError(REGEX_COMPILE_ERROR);

	FILE* f = fopen(path, "r");
	if (f == NULL)
		handleError(FILE_NOT_FOUND);
	
	char line[SIZE];
	fgets(line, SIZE, f);

	char *lineCursor;
	char *endOfLine;
	int offset;

	lineCursor = line;
    endOfLine = line + strlen(line) - 1;
    offset = (int)(lineCursor - line);

	int regSearchResult;

	while (!feof(f)) {           
        
        while (lineCursor <= endOfLine) {
            regSearchResult = regexec(&patternBuffer, lineCursor, 10, matches, 0);
            if (!regSearchResult) {
                regoff_t firstChar = matches[0].rm_so; //location of 1st char of match in line
			    regoff_t lastChar = matches[0].rm_eo; //location of 1 + last char of match in line
            
                if ( !( isWCC(line[offset + firstChar - 1]) ) || ( (line - firstChar - 1) == 0 ) ){
                    if ( !( isWCC(line[offset + lastChar]) ) || ( (endOfLine - lastChar) == 0)) {
                        printf("%s:%s", path, line);
                        break;
                    }   
                }

                ++lineCursor;
                offset = (int)(lineCursor - line);
            } 
            else {
                break;
            }
        }

		fgets(line, SIZE, f);
        lineCursor = line;
        endOfLine = line + strlen(line) - 1;
        offset = (int)(lineCursor - line);
	}
	
	regfree(&patternBuffer);
	fclose(f);
    return;
}

void wholeLine(char *path, char *query) {

	regmatch_t matches[SIZE]; // pointer to array to store matches found by regexec

	int regCompileError;
	regex_t patternBuffer;

	regCompileError = regcomp(&patternBuffer, query, REG_NEWLINE);
	if (regCompileError)
		handleError(REGEX_COMPILE_ERROR);

	FILE* f = fopen(path, "r");
	if (f == NULL)
		handleError(FILE_NOT_FOUND);

	char line[SIZE];
	fgets(line, SIZE, f);

	int regSearchResult;

	while (!feof(f)) {

			regSearchResult = regexec(&patternBuffer, line, 2, matches, 0);
			if (!regSearchResult) {
				regoff_t firstChar = matches[0].rm_so; //location of 1st char of match in line
				regoff_t lastChar = matches[0].rm_eo; //location of 1 + last char of match in line

				char match[SIZE] = {"\0"};
				strncpy(match, line + firstChar, lastChar - firstChar);
				
				line[strcspn(line, "\n")] = 0; //remove trailing newline character
				
				if (strcmp(line, match) == 0) {
					printf("%s:%s\n", path, line);								
				}
			}
		
		fgets(line, SIZE, f);
	}

	regfree(&patternBuffer);
	fclose(f);
	return;
}

void readStdIn(char *query) {

    char input[SIZE];
    char *searchPtr;

    while(!feof(stdin)) {
       fgets(input, SIZE - 1, stdin);
       searchPtr = strstr(input, query);
        if (searchPtr) {
            puts(input);
        }
    }
}

void handleError(int errorCode) {

    switch(errorCode) {
        case 0:
            puts("Usage: grep [OPTIONS] . . . PATTERN [FILE] . . ." );
            break;
        case 1:
            puts("No such file or directory.");
            break;
        case 2:
            puts("Current working directory is null");
            break;
        case 3:
            puts("Error Compiling Regular Expression");
            break;
    }
    exit(0);
}

bool isWCC(char c) {
    if (isalnum(c))
        return true;
    if (c == '_')
        return true;
    return false;
}