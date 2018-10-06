#include "SortedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
  if (list == NULL || element == NULL || list->key != NULL) {
    return;
  }
  
  //critical section
  SortedListElement_t *curr = list->next;
  
  while (curr != list) {
    //printf("1\n");
    //printf("element key: %s\n", element->key);
    //printf("curr key: %s\n", curr->key);
    //printf("next key: %s\n", curr->next->key);
    if (strcmp(element->key, curr->key) <= 0) {
      break;
    }
    curr = curr->next;
  }

  if(opt_yield & INSERT_YIELD) {
    sched_yield();
  }
  
  curr->prev->next = element;
  element->prev = curr->prev;
  element->next = curr;
  curr->prev = element;
}

int SortedList_delete( SortedListElement_t *element) {
  if (element == NULL || element->key == NULL) {
    return 1;
  }
  /*
  printf("delete this key: %s\n", element->key);
  printf("prev key: %s\n", element->prev->key);
  printf("prev->next key: %s\n", element->prev->next->key);
  printf("next key: %s\n", element->next->key);
  printf("next->prev key: %s\n", element->next->prev->key);
  */
  //interface specification asks, make sure list isn't corrupted
  if (element->next->prev != element || element->prev->next != element) {
    return 1;
  }

  //critical section
  if(opt_yield & DELETE_YIELD) {
    sched_yield();
  }
  
  element->prev->next = element->next;
  element->next->prev = element->prev;
  
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
  if (list == NULL || key == NULL || list->key != NULL) {
    return NULL;
  }

  //critical section
  SortedListElement_t *curr = list->next;

  while (curr != list) {
    if (strcmp(key, curr->key) == 0) {
      return curr;
    }
    if(opt_yield & LOOKUP_YIELD) {
      sched_yield();
    }
    //printf("2");
    curr = curr->next;
  }

  return NULL;
}

int SortedList_length(SortedList_t *list) {
  if (list == NULL || list->key != NULL) {
    return -1;
  }

  //critical section
  int count = 0;
  SortedListElement_t *curr = list->next;

  while (curr != list) {
    count++;
    if(opt_yield & LOOKUP_YIELD) {
      sched_yield();
    }
    //printf("3");
    curr = curr->next;
  }

  return count;
}
