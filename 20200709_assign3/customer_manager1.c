/*20200709 황성준
assignment 3
customer_manager1.c*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"


#define UNIT_ARRAY_SIZE 1024

/*Success is 0 fail is -1*/
#define SUCCESS 0
#define FAIL -1

struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
};

struct DB {
  struct UserInfo *pArray;   // pointer to the array
  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // # whether the array should be expanded
			     // # or not
};

/*--------------------------------------------------------------------*/
DB_T CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T) calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 1024 elements
  d->pArray = (struct UserInfo *)calloc(d->curArrSize, sizeof(struct UserInfo));
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;

  return NULL;
}

/*--------------------------------------------------------------------*/

/*free Customer DB*/
void DestroyCustomerDB(DB_T d)
{
    if(d==NULL) return;

    struct UserInfo *temp=d->pArray;
    while(temp->purchase!=0){
        free(temp->id);
        free(temp->name);
        temp++;
    }
    free(d->pArray);
    free(d);
}
/*--------------------------------------------------------------------*/
int RegisterCustomer(DB_T d, const char *id, const char *name, const int purchase)
{
    /*error control*/
    if(d==NULL || id==NULL || name==NULL || purchase<=0){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    /*dynamic array implemention*/
    /*if array is full increase size of array by 1024
    if there is more than 2048 empty space then reduce size of array by 1024 
    */
    if(d->curArrSize==d->numItems || d->numItems < (d->curArrSize)-2*UNIT_ARRAY_SIZE){
        /*current ArraySize is stored in size*/
        int size=d->curArrSize;
        struct UserInfo *temp;
        if(d->curArrSize==d->numItems){
            temp = (struct UserInfo *)calloc(d->curArrSize+UNIT_ARRAY_SIZE, sizeof(struct UserInfo));
            d->curArrSize+=UNIT_ARRAY_SIZE;
        }
        else {
            temp = (struct UserInfo *)calloc(d->curArrSize-UNIT_ARRAY_SIZE, sizeof(struct UserInfo));
            d->curArrSize-=UNIT_ARRAY_SIZE;
        }
        /*fail to execute dynamic array implementation*/
        if(temp==NULL){
            fprintf(stderr,"Can't allocate a memory for array of size %d\n",
            d->curArrSize+UNIT_ARRAY_SIZE);
            return FAIL;
        }

        /*copy data to new array*/
        for(size_t i=0 ; i<size ; i++){
            *(temp+i)=*(d->pArray+i);
        }
        /*delete current array*/
        free(d->pArray);
        /*replace it by new array*/
        d->pArray=temp;
    }

    /*Registering step*/

    struct UserInfo *current=d->pArray;
    /*iteration*/
    while(current->purchase!=0){
        if(strcmp(current->id, id)==0 || strcmp(current->name, name)==0 ) return FAIL;
        current++;
    }

    /*Changing id, name and purchase by given value*/
    /*if strdup fails then return NULL*/
    current->purchase=purchase;
    if( (current->id=strdup(id))==NULL ){
        fprintf(stderr,"Can't allocate a memory for ID\n");
        return FAIL;
    }

    if( (current->name=strdup(name))==NULL ){
        fprintf(stderr,"Can't allocate a memory for Name\n");
        return FAIL;
    }

    d->numItems++;
    return SUCCESS;
    
}
/*--------------------------------------------------------------------*/
int
UnregisterCustomerByID(DB_T d, const char *id)
{
    /*error control*/
    if(d==NULL || id==NULL ){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    struct UserInfo *temp=d->pArray;
    /*iteration*/
    while(temp->purchase!=0){
        /*find the given id*/
        if(strcmp(temp->id, id)==0 ){
            /*delete allocated memory*/
            free(temp->id);
            free(temp->name);
            /*move the data foward one by one*/
            while(temp->purchase!=0){
                *(temp)=*(temp+1);
                temp++;
            }
            return SUCCESS;
        }
        temp++;
    }
    return FAIL;
}

/*--------------------------------------------------------------------*/
int
UnregisterCustomerByName(DB_T d, const char *name)
{
    /*error control*/
    if(d==NULL || name==NULL ){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    struct UserInfo *temp=d->pArray;
    /*iteration*/
    while(temp->purchase!=0){
        /*find the give name*/
        if(strcmp(temp->name, name)==0){
            free(temp->id);
            free(temp->name);
            /*move the data foward one by one*/
            while(temp->purchase!=0){
                *(temp)=*(temp+1);
                temp++;
            }
            return SUCCESS;
        }
        temp++;
    }

    return FAIL;
}
/*--------------------------------------------------------------------*/

int GetPurchaseByID(DB_T d, const char* id)
{
    /*error control*/
    if(d==NULL || id==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    struct UserInfo *temp=d->pArray;
    /*iteration*/
    while(temp->purchase!=0){
        /*find the given ID*/
        if(strcmp(temp->id, id)==0){
            /*return the purchase correspond to ID*/
            return temp->purchase;
        }
        temp++;
    }
    return FAIL;
}

/*--------------------------------------------------------------------*/
int GetPurchaseByName (DB_T d, const char* name)
{
    /*error control*/
    if(d==NULL || name==NULL){
        fprintf(stderr,"Unacceptable inputs");
        return FAIL;
        assert(0);
    }

    struct UserInfo *temp=d->pArray;
    /*iteration*/
    while(temp->purchase!=0){
        /*find the given name*/
        if(strcmp(temp->name, name)==0){
            /*if search success return purchase*/
            return temp->purchase;
        }
        temp++;
    }
    return FAIL;
}
/*--------------------------------------------------------------------*/

int GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
    /*error control*/
    if(d==NULL || fp==NULL){
        fprintf(stderr,"wrong input value");
        return FAIL;
        assert(0);
    }

    /*saving the sum*/
    int sum=0;
    struct UserInfo *temp=d->pArray;
    /*iteration*/
    while(temp->purchase!=0){
        sum+=fp(temp->id, temp->name, temp->purchase);
        temp++;
    }
    return sum;
}