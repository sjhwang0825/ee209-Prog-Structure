/*20200709 Sungjoon Hwang, Assignment#1 decomment.c*/
#include<stdio.h>
#include<stdlib.h>

/*
inQuote and inComment indicate that current position is in (Comment, Quotation) or not
checkToIn is used to verify that '/' is followed by * or not
checkToOut is used to confirm '*' is followed by '/' or not 
*/
enum DFAState {OUT, IN};
enum DFAState inComment=OUT, inQuote=OUT, checkToIn=OUT, checkToOut=OUT;
/* saving line information */
int cur_line=1, error_line=1;


/*  determine which one to print, input value or \n   */
void print(int input){
	if(inComment==IN){
		if(input=='\n') printf("\n");
	}
	else{
		printf("%c",input);
	}
}

int main(void){
	/*1. input : current input value obtained by getchar.
	  2. Quote : indicates whether the start of quotation marks is " or '.
	  3. prev : the value obtained from a previous execution(loop).
	*/
	int input, Quote, prev;
	/*iterate the source file*/
	while((input=getchar())!=EOF){
		/*  checking 2subsequent character can initate the Comment state  */
		if(checkToIn==IN && inComment==OUT && inQuote==OUT){
			if(input=='*'){
				error_line=cur_line;
				checkToIn=OUT; inComment=IN; printf(" "); continue;
			}
			else{
				checkToIn=OUT; printf("%c",'/');
			}
		}
		/*  checking 2subsequent character can terminate the Comment State  or not */
		else if(checkToOut==IN && inComment==IN && inQuote==OUT){
			if(input=='/'){
				checkToOut=OUT; inComment=OUT; continue;
			}
			else checkToOut=OUT;
		}
		
		
		/* checking state Outside --> Comment or not*/
		if(input=='/' && inComment==OUT && inQuote==OUT){
			checkToIn=IN; continue;
		}
		/* checking state Outside --> Quotation or not*/
		else if((input=='\"' || input=='\'') && inComment==OUT && inQuote==OUT){
			Quote=input;
			inQuote=IN;
		}
		/* checking state Comment --> Outside or not*/
		else if(input=='*' && inComment==IN && inQuote==OUT){
			checkToOut=IN; continue;
		}
		/* checking state Quotation --> Outside or not */
		else if(input==Quote && prev!='\\' && inComment==OUT && inQuote==IN){
			inQuote=OUT;
		}
		
		prev=input; /* save current input value to use it later */
		if(input=='\n')cur_line++; /* saving current line */
		print(input);
	}
	
	/* print error message when comment isn't terminated */
	if(inComment==IN){
		fprintf(stderr,"Error: line %d: unterminated comment\n",error_line);
		return EXIT_FAILURE;
	}
	else return EXIT_SUCCESS;
}
