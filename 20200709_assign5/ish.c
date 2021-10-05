#define _GNU_SOURCE
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>



#define MAXARGV 10
#define MAXLINE 1024
#define MAXBUF 4096
#define MAXBG 10

void SIGQUIT_handler(int sig);
void SIGQUIT_handler2(int sig);
void SIGALRM_handler(int sig);
void SIGCHLD_handler(int sig);



pid_t bg[MAXBG];
int numbg=0;
enum DFARedir {isLeft, isRight, isPipe};
int redir[3];

int sepToken(char token){
    if('|'==token){
        redir[isPipe]+=1;; return 1;
    }
    if('<'==token){
        redir[isLeft]+=1; return 1;
    }
    if('>'==token){
        redir[isRight]+=1; return 1;
    }
    if('&'==token) return 1;

    return 0;
}

/*
return 1 if it is bg process
return 0 if it is fg process
*/
int lexical(char *command, char **argv){
    int isBg=0;
    static char buffer[MAXBUF];
    for(int i=0 ; i<MAXBUF ; i++){
        buffer[i]='\0';
    }

    enum DFAState {FALSE,TRUE};
    enum DFAState inQuote=FALSE, inSpace=FALSE;
    int j=0;

    for(int i=0 ; i<=strlen(command) ; i++){
        if('&'==command[i]) isBg=1;

        if(i==strlen(command)){
            if(FALSE==inQuote) break;
            else 
                fprintf(stderr,"unterminated quotation\n");
        }

        if(isspace(command[i])!=0 && FALSE==inQuote){
            inSpace=TRUE;
            continue;
        }

        if(TRUE==inSpace){
            inSpace=FALSE;
            buffer[j++]=',';
        }

        if('\"'==command[i] && FALSE==inQuote){
            inQuote=TRUE; continue;
        }

        if('\"'==command[i] && TRUE==inQuote){
            inQuote=FALSE; continue;
        }

        if(sepToken(command[i]) && FALSE==inQuote){
            if(','!=buffer[j-1]) buffer[j++]=',';
            if('&'!=command[i]) buffer[j++]=command[i];
            if(' '!=command[i+1]) buffer[j++]=',';
            continue;
        }

        buffer[j++]=command[i];        
    }
    buffer[j++]=',';
    buffer[j]='\0';

    char *token;
    if((token=strtok(buffer,","))==NULL) exit(-1);

    argv[0]=token;
    for(j=1 ; j<=MAXARGV ; j++){
        if(j==MAXARGV){
            fprintf(stderr,"Too many arguments\n");
            assert(0);
        }
        if((token=strtok(NULL,","))==NULL) break;
        argv[j]=token;
    }
    argv[j]=NULL;

    return isBg;
}

void unix_error(char *msg){
    fprintf(stderr,"%s\n",msg);
    assert(0);
}

int isBuiltin(char *cmd){
    if(0==strcmp("setenv",cmd)) return 1;
    if(0==strcmp("unsetenv",cmd)) return 1;
    if(0==strcmp("cd",cmd)) return 1;
    if(0==strcmp("exit",cmd)) return 1;
    if(0==strcmp("fg",cmd)) return 1;
    return 0;
}

void cmd_Builtin(char **argv, int isBg){
    if(0==strcmp("exit",argv[0])){
        exit(0);
    }

    if(0==strcmp("setenv",argv[0])){
        if(argv[2]!=NULL){
            if(setenv(argv[1],argv[2],1)<0)
                unix_error("setenv error");
        }
        else{
            if(setenv(argv[1],"",1)<0)
                unix_error("setenv error");
        }
    }

    if(0==strcmp("unsetenv",argv[0])){
        if(unsetenv(argv[1])<0)
            unix_error("unsetenv error");
    }

    if(0==strcmp("cd",argv[0])){
        const char *dir="/mnt/c/Users/sjhwa";
        if(argv[1]!=NULL) dir=argv[1];
        if(chdir(dir)<0){
            fprintf(stderr,"No such file or directory\n");
        }
    }

    if(0==strcmp("fg",argv[0])){
    if(numbg==0){
        fprintf(stderr,"fg: current: no such job");
        return;
    }

    printf("[%d] Latest background process is executing\n",bg[numbg-1]);
    pid_t pid;
    if((pid=waitpid(bg[numbg-1],NULL,0))<0) unix_error("waitpid error");
    printf("[%d] Done\n",pid);
    }

}

void cmd_nonBuiltin(char **argv, int isBg){
    pid_t pid;
    fflush(NULL);
    if((pid=fork())<0) unix_error("fork error");
    /***************************************************
    exceed the number of bg process allowed.
    ***************************************************/
    if(numbg==MAXBG && isBg==1){
        fprintf(stderr,"Too many bg processes. ignore command\n");
        return;
    }

    /*child process*/
    if(pid==0){
        /*****************************************
        recover default action for SIGINT and SIGQUIT and SIGCHLD
        *****************************************/
        void (*pfRet)(int);
        if((pfRet=signal(SIGINT,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if((pfRet=signal(SIGQUIT,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if((pfRet=signal(SIGCHLD,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if(execvp(argv[0],argv)<0)
            fprintf(stderr,"%s: No such file or directory\n",argv[0]);
        exit(0);
    }
    /*parent process*/
    else{
        /*fg process*/
        if(!isBg){
            if((waitpid(pid,NULL,0))<0) unix_error("waitpid error");
        }
        /*bg process*/
        else{
            bg[numbg++]=pid;
            if((waitpid(pid,NULL,WNOHANG))<0) unix_error("waitpid error");
        }


    }
}

void SIGQUIT_handler2(int sig){
    exit(0);
}

void SIGQUIT_handler(int sig){
    void (*pfRet)(int);
    if((pfRet=signal(SIGALRM, SIGALRM_handler))==SIG_ERR)
        unix_error("SIGALRM handler error");


    printf("Type Ctrl-\\ again within 5 secones to exit\n");

    
    if((pfRet=signal(SIGQUIT, SIGQUIT_handler2))==SIG_ERR)
        unix_error("SIGQUIT handler error");

    alarm(5);  
}

void SIGALRM_handler(int sig){

    void (*pfRet)(int);
    if((pfRet=signal(SIGQUIT, SIGQUIT_handler))==SIG_ERR)
        unix_error("SIGQUIT handler error");

    if((pfRet=signal(SIGALRM, SIG_DFL))==SIG_ERR)
        unix_error("SIGALRM handler error");
}

void SIGCHLD_handler(int sig){
    if(0==numbg) return;

    pid_t pid;
    int reaped=0;
    int i;

    for(i=0 ; i<numbg ; i++){
        pid=waitpid(bg[i], NULL, WNOHANG);
        if(pid>0){
            reaped=1; break;
        }
    }

    if(reaped){
        for(;i<numbg-1;i++){
            bg[i]=bg[i+1];
        }
        bg[i]=0;
        numbg--;
        printf("child %d terminated normally\n",pid);
    }
    
}


void redirect2(char **argv, int isBg, int walk1, int walk2, int *list){
    pid_t pid;
    fflush(NULL);
    if((pid=fork())<0) unix_error("fork error");
    /***************************************************
    exceed the number of bg process allowed.
    ***************************************************/
    if(numbg==MAXBG && isBg==1){
        fprintf(stderr,"Too many bg processes. ignore command\n");
        return;
    }

    /*child process*/
    if(pid==0){
        /*****************************************
        recover default action for SIGINT and SIGQUIT and SIGCHLD
        *****************************************/
        void (*pfRet)(int);
        if((pfRet=signal(SIGINT,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if((pfRet=signal(SIGQUIT,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if((pfRet=signal(SIGCHLD,SIG_DFL))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        
        if(walk2<0){
            if(*(list+walk1)==isRight){
                int fd;
                for(int i=walk1+1 ; i<MAXARGV && *(argv+i)!=NULL ; i++){
                    fd=creat(*(argv+i),0600);
                    close(1);
                    dup(fd);
                    close(fd);
                    execvp(argv[0], argv);
                    exit(0);
                }
            }

            if(*(list+walk1)==isLeft){
                
            }
        }
        


    }
    /*parent process*/
    else{
        /*fg process*/
        if(!isBg){
            if((waitpid(pid,NULL,0))<0) unix_error("waitpid error");
        }
        /*bg process*/
        else{
            bg[numbg++]=pid;
            if((waitpid(pid,NULL,WNOHANG))<0) unix_error("waitpid error");
        }


    }
}

void redirect(char **argv, int isBg){
    int num=redir[isLeft]+redir[isRight]+redir[isPipe];
    static int list[MAXARGV];
    for(int i=0 ; i<MAXARGV ; i++){
        list[i]=-1;
    }

    for(int i=0; argv[i]!=NULL ; i++){
        if(0==strcmp("<",argv[i])){
            argv[i]=NULL;
            list[i]=isLeft;
            continue;
        }
        else if(0==strcmp(">",argv[i])){
            argv[i]=NULL;
            list[i]=isRight;
            continue;
        }
        else if(0==strcmp("|",argv[i])){
            argv[i]=NULL;
            list[i]=isPipe;
            continue;
        }
    }

    if(0==redir[isPipe]){
        int walk1, walk2=-1, temp;

        for(walk1=0 ; walk1<MAXARGV ; walk1++){
            if(-1!=list[walk1]) break;
        }

        if(num>1){
            for(temp=walk1+1 ; temp<MAXARGV ; temp++){
                if(-1!=list[temp]){
                    walk2=temp;
                    break;
                }
            }
        }
        redirect2(argv, isBg, walk1, walk2, list);
    }

    /*    
    printf("%d\n",num);
    for(int i=0, j=0 ; j<num || argv[i]!=NULL ; i++){
        if(argv[i]==NULL){
            j++;
            printf("NULL ");
            continue;
        }
        printf("%s ",argv[i]);
    }
    printf("\n");

    for(int i=0 ; i<MAXARGV ; i++){
        printf("%d ",list[i]);
    }
    printf("\n");*/
    
}

int mypipe(char **argv){
    /*pipe token is located at the most beginning of the array*/
    if(0==strcmp("|",argv[0])) return -1;

    for(int i=0 ; argv[i]!=NULL ; i++){
        if(0==strcmp("|",argv[i])){
            /*pipe token is located at the end of the array*/
            if(i==MAXARGV-1) return -1;
            /*no command after pipe command*/
            if(NULL==argv[i+1]) return -1;
            /*successive pipe token*/
            if(0==strcmp("|",argv[i+1])) return -1;
        }
    }

    for(int i=0 ; argv[i]!=NULL ; i++){
        if(0==strcmp("|",argv[i])){
            argv[i]=NULL;
        }
    }

    /*
    for(int i=0, j=0 ; j<redir[isPipe] || argv[i]!=NULL ;i++){
        if(argv[i]==NULL){
            printf("NULL\n");
            j++;
            continue;
        }
        printf("%s\n",argv[i]);
    }*/
    

    return redir[isPipe];
}



int main(void){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGCHLD);
    void (*pfRet)(int);

    int isBg,numpipe;
    char *fgets_state, command[MAXLINE], *argv[MAXARGV];

    /******************************************************
    ******************************************************/
    while(1){
        /*signal preference for parent process*/
        if((pfRet=signal(SIGINT,SIG_IGN))==SIG_ERR)
            unix_error("SIGINT_handler error");

        if((pfRet=signal(SIGQUIT,SIGQUIT_handler))==SIG_ERR)
            unix_error("SIGQUIT_handler error");

        if((pfRet=signal(SIGCHLD,SIGCHLD_handler))==SIG_ERR)
            unix_error("SIGCHLD_handler error");

        printf("%% ");
        fgets_state=fgets(command, MAXLINE, stdin);
        /*read function is failed*/
        if(NULL==fgets_state){printf("\n"); break;}
        /*input line is just a single new line character*/
        if('\n'==*command) continue;


        redir[isLeft]=0; redir[isRight]=0; redir[isPipe]=0;
        isBg=lexical(command, argv);

        /*bg and pipe can't be together*/
        if(isBg && redir[isPipe]){
            fprintf(stderr,"| and & are incompatible\n");
            continue;
        }

        /*if(redir[isPipe]){
            if((numpipe=mypipe(argv))==-1){
                fprintf(stderr,"Invalid: Missing command name\n");
                continue;
            }
        }*/
        
        if(redir[isLeft] || redir[isRight] || redir[isPipe]) redirect(argv, isBg);
        else if(isBuiltin(argv[0])) cmd_Builtin(argv,isBg);
        else cmd_nonBuiltin(argv,isBg);
    }

    return 0;
}