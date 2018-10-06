#include"SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

int numThreads = 1;
int numIter = 1;
int numElements = 0;
int numLists = 1;
int opt_yield = 0;

SortedList_t *listArr;
SortedListElement_t *listElements;
int *listNum;

char syncArg = '\0';
pthread_mutex_t *mutexArr;
int *spinLockArr;

long long *threadLockTime;
int numLockOps = 1;

void signal_callback_handler(int signum){
  //caught signal
  if (signum == SIGSEGV) {
    fprintf(stderr, "Segmentation fault occurred.\n");
    exit(2);
  }
}

void lockAcquisitionTime (struct timespec startTime, struct timespec endTime, int threadNum) {
  long long timeElapsed = (endTime.tv_sec - startTime.tv_sec) * 1000000000;
  timeElapsed += endTime.tv_nsec;
  timeElapsed -= startTime.tv_nsec;
  threadLockTime[threadNum] += timeElapsed;

  numLockOps++;
}

int listLength (int threadNum) {
  int length = 0;
  struct timespec startTime;
  struct timespec endTime;
  for (int i = 0; i < numLists; i++) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    
    pthread_mutex_lock(mutexArr+listNum[i]);
    
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    lockAcquisitionTime(startTime, endTime, threadNum);
    
    length += SortedList_length(listArr+i);
    
    pthread_mutex_unlock(mutexArr+listNum[i]);
  }
  return length;
}

void lookupAndDelete (int i) {
  SortedListElement_t *tempElement = SortedList_lookup(listArr+listNum[i], listElements[i].key);
  if (tempElement == NULL) {
    fprintf(stderr, "List element not found; list corrupted.\n");
    exit(2);
  }
  if (SortedList_delete(tempElement) == 1) {
    fprintf(stderr, "Error while deleting element; list corrupted.\n");
    exit(2);
  }
}

void* threadFunc (void *index) {
  int threadNum = *((int*)index);
  threadLockTime[threadNum] = 0;

  struct timespec startTime;
  struct timespec endTime;
  for (int i = threadNum; i < numElements; i += numThreads) {
    switch (syncArg) {
      case 'm': {
	clock_gettime(CLOCK_MONOTONIC, &startTime);
	
        pthread_mutex_lock(mutexArr+listNum[i]);
	
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        lockAcquisitionTime(startTime, endTime, threadNum);
	
        SortedList_insert(listArr+listNum[i], &listElements[i]);
	
        pthread_mutex_unlock(mutexArr+listNum[i]);  
        break;
      }

      case 's': {
        clock_gettime(CLOCK_MONOTONIC, &startTime);
	
        while(__sync_lock_test_and_set(spinLockArr+listNum[i], 1));
	
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        lockAcquisitionTime(startTime, endTime, threadNum);
	
        SortedList_insert(listArr+listNum[i], &listElements[i]);
	
        __sync_lock_release(spinLockArr+listNum[i]);
	
        break;
      }

      default:
        SortedList_insert(listArr+listNum[i], &listElements[i]);
        break;
    }
  }

  int retValue = 0;
  switch (syncArg) {
    case 'm': {
      retValue = listLength(threadNum);
      break;
    }
      
    case 's': {
      retValue = listLength(threadNum);
      break;
    }
      
    default: {
      int length = 0;
      for (int i = 0; i < numLists; i++) {
	retValue = SortedList_length(listArr+i);
	if (retValue == -1) {
	  fprintf(stderr, "Error while fetching list length; list corrupted.\n");
	  exit(2);
	}
	length += retValue;
      }
      break;
    }
  }
  if (retValue == -1) {
    fprintf(stderr, "Error while fetching list length; list corrupted.\n");
    exit(2);
  }
  
  for (int i = threadNum; i < numElements; i += numThreads) {
    switch (syncArg) {
      case 'm': {
        clock_gettime(CLOCK_MONOTONIC, &startTime);
	
	pthread_mutex_lock(mutexArr+listNum[i]);
	
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        lockAcquisitionTime(startTime, endTime, threadNum);
	
	lookupAndDelete(i);
	
	pthread_mutex_unlock(mutexArr+listNum[i]);
	break;
      }

      case 's': {
        clock_gettime(CLOCK_MONOTONIC, &startTime);
	
	while(__sync_lock_test_and_set(spinLockArr+listNum[i], 1));
	
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        lockAcquisitionTime(startTime, endTime, threadNum);
	
	lookupAndDelete(i);
	
	__sync_lock_release(spinLockArr+listNum[i]);
	break;
      }

      default:
	lookupAndDelete(i);
	break;
    }
  }
  return NULL;
}

//first thing that came up when I googled C string hash function
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (0 != (c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % numLists;
}

int main (int argc, char *argv[]) {
  //setup signal handler
  signal(SIGSEGV, signal_callback_handler);
  
  char testName1[6] = "-none";
  char testName2[6] = "-none";
  
  //check arguments
  static struct option long_options[] = {
      {"threads",     required_argument, 0, 't'},
      {"iterations",  required_argument, 0, 'i'},
      {"yield",  required_argument, 0, 'y'},
      {"sync",  required_argument, 0, 's'},
      {"lists",  required_argument, 0, 'l'},
      {0, 0, 0, 0}
  };

  int option_index = 0;
  int ret = getopt_long(argc, argv, "", long_options, &option_index);
  while (ret != -1) {
    switch (ret) {
      case 't':
        numThreads = atoi(optarg);
        break;

      case 'i':
	numIter = atoi(optarg);
        break;

      case 'y':
	//Format output
	testName1[1] = '\0';
	int iFlag = 0;
	int dFlag = 0;
	int lFlag = 0;

	for (size_t i = 0; i < strlen(optarg); i++) {
          if (i >= 3) {
            fprintf(stderr, "Usage: %s [--threads=#] [--iterations=#] [--yield<idl>] [--sync==<ms>]\n", argv[0]);
            exit(1);
          }
          if (optarg[i] == 'i') {
            opt_yield |= INSERT_YIELD;
            iFlag = 1;
          } else if (optarg[i] == 'd') {
            opt_yield |= DELETE_YIELD;
            dFlag = 1;
          } else if (optarg[i] == 'l') {
            opt_yield |= LOOKUP_YIELD;
            lFlag = 1;
          } else {
            fprintf(stderr, "Usage: %s [--threads=#] [--iterations=#] [--yield<idl>] [--sync==<ms>]\n", argv[0]);
            exit(1);
          }
        }

	if (iFlag == 1) {
	  strcat(testName1, "i");
	}
	if (dFlag == 1) {
          strcat(testName1, "d");
	}
	if (lFlag == 1) {
          strcat(testName1, "l");
	}
	break;

      case 's':
	syncArg = optarg[0];
        testName2[1] = syncArg;
        testName2[2] = '\0';
	numLockOps = 0;
	break;

      case 'l':
        numLists = atoi(optarg);
        break;

      default:
        fprintf(stderr, "Usage: %s [--threads=#] [--iterations=#] [--yield<idl>] [--sync==<ms>]\n", argv[0]);
        exit(1);
    }
    ret = getopt_long(argc, argv, "", long_options, &option_index);
  }

  if (numThreads < 0 || numIter < 0 || (syncArg != '\0' && syncArg != 'm' && syncArg != 's')) {
    fprintf(stderr, "Usage: %s [--threads=#] [--iterations=#] [--yield<idl>] [--sync==<ms>]\n", argv[0]);
    exit(1);
  }

  //Main execution
  //initialize list
  listArr = malloc(numLists * sizeof(SortedList_t));
  mutexArr = malloc(numLists * sizeof(pthread_mutex_t));
  spinLockArr = malloc(numLists * sizeof(int));
  for (int i = 0; i < numLists; i++) {
    listArr[i].key = NULL;
    listArr[i].prev = &listArr[i];
    listArr[i].next = &listArr[i];
    spinLockArr[i] = 0;
  }
  
  numElements = numThreads * numIter;
  listElements = malloc(sizeof(SortedListElement_t)*numElements);
  listNum = malloc(numElements * sizeof(int));

  //Fill elements with random keys
  srand(time(NULL));   //only called once
  for (int i = 0; i < numElements; i++) {
    int randNum = rand() % 26;
    unsigned char *randKey = malloc(sizeof(char)*2);
    randKey[0] = 'A' + randNum;
    randKey[1] = '\0';
    listElements[i].key = (char*)randKey;
    listNum[i] = hash(randKey);
    //printf("key: %s\n", randKey);
  }
  
  struct timespec startTime;
  clock_gettime(CLOCK_MONOTONIC, &startTime);

  pthread_t *threads = malloc(numThreads * sizeof(pthread_t));
  threadLockTime = malloc(numThreads * sizeof(long long));
  
  //Create threads
  for (int i = 0; i < numThreads; i++) {
    int *index = malloc(sizeof(int));
    *index = i;
    if (pthread_create(threads+i, NULL, threadFunc, index)) {
      fprintf(stderr, "Error occurred while creating thread %d: %s.\n", i, strerror(errno));
      exit(2);
    }
  }
  
  //Wait for threads
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(threads[i], NULL)) {
      fprintf(stderr, "Error occurred while joining thread %d: %s.\n", i, strerror(errno));
      exit(2);
    }
  }
  
  struct timespec endTime;
  clock_gettime(CLOCK_MONOTONIC, &endTime);
  long long timeElapsed = (endTime.tv_sec - startTime.tv_sec) * 1000000000;
  timeElapsed += endTime.tv_nsec;
  timeElapsed -= startTime.tv_nsec;

  long long totalLockTime = 0;
  for (int i = 0; i < numThreads; i++) {
    totalLockTime += threadLockTime[i];
  }
  
  long long numOps = (numThreads*numIter*3);
  printf("list%s%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", testName1, testName2, numThreads, numIter, numLists, numOps, timeElapsed, (timeElapsed/numOps), (totalLockTime/numLockOps));

  free(listElements);
  free(listArr);
  free(threads);
  
  return 0;
}
