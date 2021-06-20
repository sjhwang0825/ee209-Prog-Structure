/*20200709 황성준
assignment 3
customer_manager2.c*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"


#define UNIT_ARRAY_SIZE 1024

/*Success is 0 fail is -1*/
#define SUCCESS 0
#define FAIL -1

/*hash function*/
enum {HASH_MULTIPLIER = 65599};
static int hash_function(const char *pcKey, int iBucketCount){
   int i;
   unsigned int uiHash = 0U;
   for (i = 0; pcKey[i] != '\0'; i++)
      uiHash = uiHash * (unsigned int)HASH_MULTIPLIER
               + (unsigned int)pcKey[i];
   return (int)(uiHash % (unsigned int)iBucketCount);
}

struct UserInfo {
    char *name;                // customer name
    char *id;                  // customer id
    int purchase;              // purchase amount (> 0)

    /*IdNext means the following ID node*/
    struct UserInfo *IdNext;
    /*NameNext means the following Name node*/
    struct UserInfo *NameNext;
};

struct DB {
    struct UserInfo *pIdHash[UNIT_ARRAY_SIZE];   // pointer to the Hash for ID search
    struct UserInfo *pNameHash[UNIT_ARRAY_SIZE]; // pointer to the Hash for Name search
};

/*--------------------------------------------------------------------*/
DB_T CreateCustomerDB(void)
{
    DB_T d;
    /*memory allocation and initialization*/
    d = (DB_T) calloc(1, sizeof(struct DB));
    /*if memory allocation fails return NULL*/
    if (d == NULL) {
        fprintf(stderr, "Can't allocate a memory for DB_T\n");
        return NULL;
    }

    return d;
}


/*--------------------------------------------------------------------*/

/*free Customer DB*/
void DestroyCustomerDB(DB_T d)
{/*does nothing when d is NULL*/
    if(d==NULL) return;

    struct UserInfo *p,*next;
    for(int i=0 ; i<UNIT_ARRAY_SIZE ; i++){
        if(d->pIdHash[i]!=0){
            for(p=d->pIdHash[i] ; p!=NULL ; p=next){
                next=p->IdNext;
                free(p);
            }
        }
    }
    free(d);
}

/*--------------------------------------------------------------------*/
/*search that given id or name is already exist or not*/
int IsExist(DB_T d, const char *id, const char *name){
    /*error control*/
    assert(d||id||name);
    /*if given ID exists then checkid=1 if not checkid=0*/
    /*if given Name exists then checkname=1 if not checkid=0*/
    int checkid=0, checkname=0;
    int IdHash=hash_function(id,UNIT_ARRAY_SIZE);
    int NameHash=hash_function(name, UNIT_ARRAY_SIZE);
    /*iterate ID*/
    for(struct UserInfo *p=d->pIdHash[IdHash] ; p!=NULL ; p=p->IdNext){
        if( strcmp(p->id, id)==0 ){
            checkid=1; break;
        }
    }
    /*iterate Name*/
    for(struct UserInfo *p=d->pNameHash[NameHash] ; p!=NULL ; p=p->NameNext){
        if( strcmp(p->name, name)==0 ){
            checkname=1; break;
        }
    }
    /*if ID or Name exists then return 1*/
    if(checkid || checkname) return 1;
    /*else return 0*/
    return 0;
}

int RegisterCustomer(DB_T d, const char *id, const char *name, const int purchase)
{
    /*error control*/
    if(d==NULL || id==NULL || name==NULL || purchase<=0){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    /*if given id or name already exists then return -1*/
    if(IsExist(d, id, name)){
        return FAIL;
    }

    /*construct new node*/
    struct UserInfo *node=(struct UserInfo *)malloc(sizeof(struct UserInfo));
    /*initialize the value of node*/
    node->purchase=purchase;
    /*checking that string copy is well done or not*/
    if( (node->name=strdup(name))==NULL){
        fprintf(stderr,"Can't allocate a memory for Id\n");
        return FAIL;
    }
    if( (node->id=strdup(id))==NULL ){
        fprintf(stderr,"Can't allocate a memory for Name\n");
        if(node->name!=NULL) free(node->name);
        return FAIL;
    }

    /*returned value by hash_function*/
    int IdHash=hash_function(id,UNIT_ARRAY_SIZE);
    int NameHash=hash_function(name,UNIT_ARRAY_SIZE);

    /*pointers with respect to ID*/
    node->IdNext=d->pIdHash[IdHash];
    d->pIdHash[IdHash]=node;

    /*pointers with respect to Name*/
    node->NameNext=d->pNameHash[NameHash];
    d->pNameHash[NameHash]=node;

    return SUCCESS;
}

/*--------------------------------------------------------------------*/

/*used in UnregisterCustomerById function*/
/*used in UnregisterCustmoerByName function*/
/*eliminate a node whose Id and Name is same as given parameter*/
/*and finally it returns 0(remove SUCCESS)*/
int FreeNode(DB_T d, const char *id, const char *name){
    /*error control*/
    assert(d||id||name);

    struct UserInfo *p, *prev=NULL;
    int IdHash=hash_function(id, UNIT_ARRAY_SIZE);
    int NameHash=hash_function(name, UNIT_ARRAY_SIZE);
    /*changing pointer related to ID*/
    for(p=d->pIdHash[IdHash] ; p!=NULL ; prev=p, p=p->IdNext){
        if(strcmp(p->id, id)==0){
            if(prev==NULL) d->pIdHash[IdHash]=p->IdNext;
            else prev->IdNext=p->IdNext;
        }
    }
    /*changing pointer related to Name*/
    for(p=d->pNameHash[NameHash],prev=NULL ; p!=NULL ; prev=p, p=p->NameNext){
        if(strcmp(p->name,name)==0){
            if(prev==NULL) d->pNameHash[NameHash]=p->NameNext;
            else prev->NameNext=p->NameNext;
            free(p);
            return SUCCESS;
        }
    }
    /*it should not occur*/
    assert(0);
    return FAIL;
}

int UnregisterCustomerByID(DB_T d, const char *id)
{
    /*error control*/
    if(d==NULL || id==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }
    /*Find the name corresponding to the ID*/
    char *name;
    struct UserInfo *p;
    int IdHash=hash_function(id, UNIT_ARRAY_SIZE);
    /*Find the name*/
    /*iteration*/
    for(p=d->pIdHash[IdHash] ; p!=NULL ; p=p->IdNext){
        if(strcmp(p->id, id)==0){
            name=p->name;
            break;
        }
    }
    /*if above execution find the name corresponding 
    to the given ID then remove a node and return 0*/
    if(p!=NULL) return FreeNode(d, id, name);
    return FAIL;
}

/*--------------------------------------------------------------------*/
int UnregisterCustomerByName(DB_T d, const char *name)
{
    
    /*error control*/
    if(d==NULL || name==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }
    char *id;
    struct UserInfo *p;
    int NameHash=hash_function(name, UNIT_ARRAY_SIZE);
    for(p=d->pNameHash[NameHash] ; p!=NULL ; p=p->NameNext){
        if(strcmp(p->name, name)==0){
            id=p->id;
            break;
        }
    }
    /*if above execution find the ID corresponding 
    to the given Name then remove a node and return 0*/
    if(p!=NULL) return FreeNode(d, id, name);
    return FAIL;
}
/*--------------------------------------------------------------------*/

int GetPurchaseByID(DB_T d, const char* id)
{
    /*error control*/
    if(d==NULL||id==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    int IdHash=hash_function(id, UNIT_ARRAY_SIZE);
    for(struct UserInfo *p=d->pIdHash[IdHash] ; p!=NULL ; p=p->IdNext){
        if(strcmp(p->id, id)==0) return p->purchase;
    }

    return FAIL;
}

/*--------------------------------------------------------------------*/
int GetPurchaseByName (DB_T d, const char* name)
{
    if(d==NULL||name==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }   

    int NameHash=hash_function(name, UNIT_ARRAY_SIZE);
    for(struct UserInfo *p=d->pNameHash[NameHash] ; p!=NULL ; p=p->NameNext){
        if(strcmp(p->name, name)==0) return p->purchase;
    }
    return FAIL;
}
/*--------------------------------------------------------------------*/

int GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
    if(d==NULL || fp==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    int sum=0;
    struct UserInfo *p;

    /*HashTable iteration*/
    for(int i=0 ; i<UNIT_ARRAY_SIZE ; i++){
        if(d->pIdHash[i]!=NULL){
            /*LinkedList iteration*/
            for(p=d->pIdHash[i]; p!=NULL ; p=p->IdNext){
                sum+=fp(p->id, p->name, p->purchase);
            }
        }
    }
    return sum;
}