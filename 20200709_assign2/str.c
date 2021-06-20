#include <assert.h> /* to use assert() */
#include <stdio.h>
#include <stdlib.h> /* for strtol() */
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "str.h"

size_t StrGetLength(const char* pcSrc)
{
  const char *pcEnd;
  assert(pcSrc); /* NULL address, 0, and FALSE are identical. */
  pcEnd = pcSrc;
	
  while (*pcEnd) /* null character and FALSE are identical. */
    pcEnd++;

  return (size_t)(pcEnd - pcSrc);
}

char *StrCopy(char *pcDest, const char* pcSrc)
{
    assert(pcSrc);
    while(*pcDest){ /*increase pcDest until NULL*/
        pcDest++;
    }
    char *temp=pcDest;
    while( (*pcDest++=*pcSrc++)!='\0' ){} /*copying pcSrc including last NULL character*/
    return temp;
}

int StrCompare(const char* pcS1, const char* pcS2)
{
    assert(pcS1); assert(pcS2);
    /*inspect until one String is finished*/
    for(size_t i=0 ; i<StrGetLength(pcS1) && i<StrGetLength(pcS2) ;i++){
        if(pcS1[i]!=pcS2[i]){
            return pcS1[i]<pcS2[i] ? -1 : 1;
        }
    }

    /*determine two String is identical or not*/
    if(StrGetLength(pcS1)==StrGetLength(pcS2)) return 0;
    else return StrGetLength(pcS1)<StrGetLength(pcS2) ? -1:1;
}

char *StrFindChr(const char* pcHaystack, int c)
{
    assert(pcHaystack);
    /*iterating through pcHayStack*/
    for(;*pcHaystack!='\0';pcHaystack++){
        if(*pcHaystack==c) return (char *)pcHaystack;
    }
    /*if target character is NULL then return the terminal adress*/
    if(0==c) return (char *)pcHaystack;
    /*else return NULL*/
    else return NULL;
}

char *StrFindStr(const char* pcHaystack, const char *pcNeedle)
{
    assert(pcHaystack);
    /*if pcNeedle is "" then return initial adress */
    if(StrGetLength(pcNeedle)==0) return (char *)pcHaystack;

    for(;*pcHaystack!='\0';pcHaystack++){
        if(*pcHaystack==*pcNeedle){
            size_t j;
            for(j=0 ; j<StrGetLength(pcNeedle) ; j++){
                if(*(pcHaystack+j)!=*(pcNeedle+j)) break;
            }
            if(StrGetLength(pcNeedle)==j) return (char *)pcHaystack;
            /*if pcHaystack contains the whole string in pcNeedle then return*/
        }
    }
    /*doesn't contain the String in pcNeedle*/
    return NULL;
}

char *StrConcat(char *pcDest, const char* pcSrc)
{
    assert(pcSrc);
    char *temp=pcDest; /*current address*/
    while(*pcDest){
        pcDest++; /*iterating until NULL*/
    }
    while( (*pcDest++=*pcSrc++)!='\0' ){} /*copying the pcSrc including NULL at terminal*/
    /*return initial address*/
    return temp;
}

long int StrToLong(const char *nptr, char **endptr, int base)
{
    assert(nptr);
    if (base!=10) return 0;
    /*state indicates the givin nptr is either plus or minus */
    enum DFAState {MINUS, PLUS};
    /*defalut state is PLUS*/
    enum DFAState state=PLUS;
    /*String to Long data is contained in value*/
    /*temp contains the information of (char)-->(int) */
    long value=0; int temp;

    while(*nptr && isspace(*nptr)) nptr++; /*pass the empty character*/

    /*determine the sign*/
    if('-'==*nptr || '+'==*nptr){
        if(*nptr=='-') state=MINUS;
        nptr++;
    }

    while(*nptr && isdigit(*nptr)){

        /*case of overflow or underflow*/
        if(value<0 && PLUS==state) break;
        else if(value>0 && MINUS==state) break;

        /*converting String to Long*/
        temp=*nptr-'0';
        if(PLUS==state) {value=10*value+temp;}
        else if(MINUS==state) {value=10*value-temp;}
        nptr++;
    }
    /*set endpoint*/
    if(endptr) *endptr=(char *)nptr;

    if(value<0 && PLUS==state) return LONG_MAX; /*overflow*/
    if(value>0 && MINUS==state) return LONG_MIN; /*underflow*/
    return value;
}