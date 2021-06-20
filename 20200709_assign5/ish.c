#define _GNU_SOURCE
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<assert.h>
#include"dynarray.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<fcntl.h>

#define argument_size 100

/*lexical analysis*/
DynArray_T lexical(char command[], DynArray_T arguments){
    /*Show status of being inside a particular string*/
    enum DFAState {OUT, IN};
    /*quotation mark*/
    /*prev step indicate space or not*/
    enum DFAState inQuote=OUT, inSkip=OUT;
    char buffer[strlen(command)];
    int j=0;

    for(int i=0 ; i<=strlen(command); i++){
        /*reach end of string*/
        if(i==strlen(command)){
            if(IN!=inQuote) break;
            /*quotation mark doesn't match*/
            fprintf(stderr,"ERROR - unmatched quote\n"); break;
        }

        /*encounter finds space and it's not inside the quotation*/
        if(isspace(command[i])!=0 && OUT==inQuote){
            inSkip=IN; continue;
        }

        /*previous character was string*/
        /*change inskip to out and 
        put comma to distinguish*/
        if(IN==inSkip){
            inSkip=OUT;
            buffer[j++]=',';
        }

        if('\"'==command[i] && IN==inQuote){
            inQuote=OUT; continue;
        }

        if('\"'==command[i] && OUT==inQuote){
            inQuote=IN; continue;
        }

        if(IN==inQuote){
            buffer[j++]=command[i]; continue;
        }

        if(OUT==inQuote && '|'==command[i]){
            buffer[j++]=command[i]; continue;
        }

        if(OUT==inQuote && '<'==command[i]){
            buffer[j++]=command[i]; continue;
        }

        if(OUT==inQuote && '>'==command[i]){
            buffer[j++]=command[i]; continue;
        }

        if(OUT==inQuote && '&'==command[i]){
            buffer[j++]=command[i]; continue;
        }

        buffer[j++]=command[i];
    }
    buffer[j++]=',';
    buffer[j]='\0';

    /*token stores the output of strtok*/
    /*temp stores token*/
    /*use static variable to maintain*/
    static char *token, *temp[argument_size];
    /*initialize static variable*/
    token=NULL;
    for(int i=0 ; i<argument_size ; i++){
        temp[i]=NULL;
    }

    token=strtok(buffer,",");
    temp[0]=token;
    int k;
    for(k=1 ; k<argument_size ; k++){
        token=strtok(NULL, ",");
        if(token==NULL) break;
        temp[k]=token;
    }
    temp[k]=NULL;

    /*store tokens to DynArray*/
    for(k=0 ; temp[k]!=NULL;k++){
        int add_state=DynArray_add(arguments, temp[k]);
        if(add_state==0) fprintf(stderr, "DynArray_add fails");
    }
    /*return DynArray*/
    return arguments;
}

//format of output, that is "% argv1 argv2 argv3" 
void format(DynArray_T arguments){
    printf("%% ");
    for(int i=0 ; i<DynArray_getLength(arguments);i++){
        char *value=DynArray_get(arguments,i);
        printf("%s ",value);
    }
    printf("\n");
}

/*execute buildin command*/
/*exit, fg, setenv, unsetenv, cd*/
void execute_Builtin(DynArray_T arguments){
    /*store first argument: fg, exit, etc.*/
    char *first=DynArray_get(arguments,0);
    char *argv[argument_size];
    int i,size=DynArray_getLength(arguments);
    for(i=0 ; i<DynArray_getLength(arguments);i++){
        char *value=DynArray_get(arguments,i);
        argv[i]=value;
    }
    argv[i]=NULL;
    /*making argument array*/

    /*exit*/
    if(0==strcmp(first,"exit")){
        exit(0);
    }

    /*fg*/
    if(0==strcmp(first,"fg")){
    }

    /*setenv*/
    if(0==strcmp(first,"setenv")){
        /*the case only setenv and varname is given*/
        if(size==2){
            int state=setenv(argv[1],"",1);
            assert(-1!=state);
        }
        /*the case when setenv, varname and value is given*/
        else{
            int state=setenv(argv[1],argv[2],1);
            assert(-1!=state);
        }
    }

    /*unsetenv*/
    if(0==strcmp(first,"unsetenv")){
        int state=unsetenv(argv[1]);
        /*unsetenv failed*/
        assert(-1!=state);
    }

    /*cd*/
    if(0==strcmp(first,"cd")){
        /*home directory*/
        const char *dir="/mnt/home/20200709";
        /*both cd and directory is given*/
        if(size!=1){
            dir=argv[1];
        }
        int state=chdir(dir);
        if(-1==state) fprintf(stderr,"./ish: No such file or directory\n");
    }
}

/*background process*/
void bg(DynArray_T arguments){
    char *argv[argument_size];
    int i;
    for(i=0 ; i<DynArray_getLength(arguments);i++){
        char *value=DynArray_get(arguments,i);
        argv[i]=value;
    }
    argv[i]=NULL;

    int pid;
    pid=fork();
    if(pid!=0){
        wait(NULL);
    }
    else{
        execvp(argv[0], argv);
        exit(0);
    } 
}

/*execute non buildin process*/
void execute_nonBuiltin(DynArray_T arguments){
    char *argv[argument_size];
    int i;
    for(i=0 ; i<DynArray_getLength(arguments);i++){
        char *value=DynArray_get(arguments,i);
        if(0==strcmp("&",value)){
            bg(arguments);
        }
        argv[i]=value;
    }
    argv[i]=NULL;
    /*convert DynArray to array*/


    int pid;
    pid=fork();
    if(pid!=0){
        wait(NULL);
    }
    else{
        int state=execvp(argv[0], argv);
        /*failed to execute*/
        if(state==-1)fprintf(stderr,"%s: No such file or directory\n",
        (char *)DynArray_get(arguments,0));
    }

}

/*SIGINT handler*/
static void SIGINT_handler(int isig){
    pid_t ipid=getpid();
    if(ipid!=0){return;}

}

/*used for control re-entered SIGQUIT*/
static void exit_handler(int isig){
    exit(0);
}


/*SIGQUIT handler*/
static void SIGQUIT_handler(int isig){
    pid_t ipid=getpid();
    if(ipid!=0){
        printf("Type Ctrl-\\ again within 5 seconds to exit.\n");
        void (*pfRet)(int);
        /*change SIGQUIT handler*/
        pfRet=signal(SIGQUIT,exit_handler);
        assert(pfRet!=SIG_ERR);
        /*SIGALRM is created after 5sec*/
        alarm(5);
    }
}

/*SIGALRM handler*/
static void SIGALRM_handler(int isig){
    pid_t ipid=getpid();
    if(ipid!=0){
        void (*pfRet)(int);
        /*change SIGQUIT handler to SIGQUIT_handler not exit_handler*/
        pfRet=signal(SIGQUIT, SIGQUIT_handler);
        assert(pfRet!=SIG_ERR);
        return;
    }
}

/*SIGCHLD chandler*/
static void SIGCHLD_handler(int isig){
    int status;
    waitpid(-1, &status, 0);
}

int main(void){
    /*Signal Blocker*/
    sigset_t sSet;
    sigemptyset(&sSet);
    sigaddset(&sSet, SIGINT);
    sigaddset(&sSet, SIGQUIT);
    sigaddset(&sSet, SIGALRM);
    sigaddset(&sSet,SIGCHLD);
    
    /*block signals until all signal handler is modified*/
    sigprocmask(SIG_BLOCK, &sSet, NULL);

    /*signalHandler*/
    void (*pfRet)(int);
    pfRet=signal(SIGINT,SIGINT_handler);
    assert(pfRet!=SIG_ERR);
    pfRet=signal(SIGQUIT,exit_handler);
    assert(pfRet!=SIG_ERR);
    pfRet=signal(SIGQUIT,SIGQUIT_handler);
    assert(pfRet!=SIG_ERR);
    pfRet=signal(SIGALRM, SIGALRM_handler);
    assert(pfRet!=SIG_ERR);
    pfRet=signal(SIGCHLD,SIGCHLD_handler);
    assert(pfRet!=SIG_ERR);

    /*all SIGhandler has benn changed*/
    /*unblock SIG*/
    sigprocmask(SIG_UNBLOCK, &sSet, NULL);


    const char *file_address="/mnt/home/20200709/.ishrc";
    /*used to store file stream*/
    char command[1024];
    /*open file*/
    FILE *ishrc=fopen(file_address,"r");

    /*DynArray to store arguments*/
    /*stdin from ishrc*/
    /*iteration by ./ishrc*/
    while(1){
        /*doesn't exist or not readable*/
        if(NULL==ishrc) break;
        /*file stream*/
        char *fgets_state=fgets(command, 1024, ishrc);
        /*reached EOF*/
        if(NULL==fgets_state) break;

        DynArray_T arguments=DynArray_new(0);
        arguments=lexical(command, arguments);

        /*classify builtin or not*/
        char *value=DynArray_get(arguments,0);
        if(0==strcmp("setenv",value) || 0==strcmp("unsetenv",value) || 
        0==strcmp("cd",value) || 0==strcmp("exit",value) || 0==strcmp("fg",value)){
            format(arguments);
            execute_Builtin(arguments);
        }
        else{
            /*clear I/O buffer*/
            fflush(NULL);
            format(arguments);
            execute_nonBuiltin(arguments);
        }
        DynArray_free(arguments);
    }

    /*iteration by stdin*/
    while(1){
        /*output format*/
        printf("%% ");
        /*input from standard input*/
        char *fgets_state=fgets(command, 1024, stdin);
        /*fails to read*/
        if(NULL==fgets_state){printf("\n"); break;}
        /*If there is no input*/
        if('\n'==*command) continue;

        DynArray_T arguments=DynArray_new(0);
        arguments=lexical(command, arguments);
        /*classify builtin or not*/
        char *value=DynArray_get(arguments,0);
        if(0==strcmp("setenv",value) || 0==strcmp("unsetenv",value) || 
        0==strcmp("cd",value) || 0==strcmp("exit",value) || 0==strcmp("fg",value)){
            execute_Builtin(arguments);
        }
        else{
            /*clear I/O buffer*/
            fflush(NULL);
            execute_nonBuiltin(arguments);
        }
        DynArray_free(arguments);

    }

    return 0;
}