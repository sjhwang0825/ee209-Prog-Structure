#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for skeleton code */
#include <unistd.h> /* for getopt */
#include "str.h"

#define MAX_STR_LEN 1023

#define FALSE 0
#define TRUE  1

/*
 * Fill out your own functions here (If you need) 
 */

/*--------------------------------------------------------------------*/
/* PrintUsage()
   print out the usage of the Simple Grep Program                     */
/*--------------------------------------------------------------------*/
void 
PrintUsage(const char* argv0) 
{
  const static char *fmt = 
	  "Simple Grep (sgrep) Usage:\n"
	  "%s pattern [stdin]\n";

  printf(fmt, argv0);
}
/*-------------------------------------------------------------------*/


/*checking pattern contains '*' or not*/
int isContainStar(char *pattern){
    if(StrFindChr(pattern, '*')==NULL) return FALSE;
    else return TRUE;
}

 
// used when pattern contains '*' 
int check(char *buf, char *pattern){
    size_t i; int state=TRUE;
    /*base case*/
    if(*pattern=='\0') return TRUE;
    // skip when current character pointed by pattern is '*'
    if(*pattern=='*') return check(buf, pattern+1);

    /*
    find character pointed by pattern
    if search fails return false
    */
    char *temp=StrFindChr(buf, *pattern);
    if(temp==NULL) return FALSE;

    /*
    comparing between temp and pattern
    when pattern points '*' then search is halted.
    checking that all characters are same before pattern encounters '*'
    if yes state==TRUE
    if not state==FALSE

    */
    for(i=0 ; *(pattern+i) && *(pattern+i)!='*' ; i++){
        if(*(temp+i)!=*(pattern+i) ){
            state=FALSE;
            break;
        }
    }

    /*search again*/
    if(state==TRUE) {return check(temp,pattern+i);}

    /*if search fails find next address that pointed by pattern 
    if search fails return FALSE
    */
    if ( (temp=StrFindChr(temp+1, *pattern)) ==NULL ) return FALSE;
    return check(temp, pattern);

    
}

/* SearchPattern()
   Your task:
   1. Do argument validation 
   - String or file argument length is no more than 1023
   - If you encounter a command-line argument that's too long, 
   print out "Error: argument is too long"
   
   2. Read the each line from standard input (stdin)
   - If you encounter a line larger than 1023 bytes, 
   print out "Error: input line is too long" 
   - Error message should be printed out to standard error (stderr)
   
   3. Check & print out the line contains a given string (search-string)
      
   Tips:
   - fgets() is an useful function to read characters from file. Note 
   that the fget() reads until newline or the end-of-file is reached. 
   - fprintf(sderr, ...) should be useful for printing out error
   message to standard error

   NOTE: If there is any problem, return FALSE; if not, return TRUE  */
/*-------------------------------------------------------------------*/

int SearchPattern(const char *pattern)
{
    /*isStar indicates pattern contains(TRUE) '*' or not(FALSE) */
  char buf[MAX_STR_LEN + 2]; 
  int len, isStar;

  /* 
   *  TODO: check if pattern is too long
   */

  /*print error if pattern is too long*/
  if(StrGetLength(pattern) > MAX_STR_LEN){
      fprintf(stderr,"Error: pattern is too long\n");
      return FALSE;
  }

  
  /* Read one line at a time from stdin, and process each line */
  while (fgets(buf, sizeof(buf), stdin)) {
	  
    /* check the length of an input line */
    if ((len = StrGetLength(buf)) > MAX_STR_LEN) {
      fprintf(stderr, "Error: input line is too long\n");
      return FALSE;
    }
	
    /* TODO: fill out this function */
    /*check pattern contains '*' or not 
    if it isn't then use StrFindStr
    if it is then use check function(recursive use of StrFindChr)
    */
    isStar=isContainStar((char*)pattern);
    if(FALSE==isStar && (StrFindStr(buf, (char *)pattern))!=NULL ){printf("%s",buf);}
    if(TRUE==isStar && check(buf, (char *)pattern) ){printf("%s",buf);}
  }
   
  return TRUE;
}

/*-------------------------------------------------------------------*/
int 
main(const int argc, const char *argv[]) 
{
  /* Do argument check and parsing */
  if (argc < 2) {
	  fprintf(stderr, "Error: argument parsing error\n");
	  PrintUsage(argv[0]);
	  return (EXIT_FAILURE);
  }

  return SearchPattern(argv[1]) ? EXIT_SUCCESS:EXIT_FAILURE;
}