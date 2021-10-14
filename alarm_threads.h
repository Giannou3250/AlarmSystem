#ifndef __alarmthreads_h
#define __alarmthreads_h
#include "main_thread.h"



//define the number of alarms that can be assigned to a single thread
#define MAX_THREAD_ALARMS 2
#define ALARM_THREAD_DISPLAY_INTERVAL 5 //number of seconds between alarm thread display messages

//forward declaration
typedef struct alarm_tag alarm_t;
struct thread_args;

/*This struct is used to create a list of active threads and their assigned alarms
* The structure is used by the alarm threads for tracking.  As well the list can be used by 
* 'View_Alarms' command in the main thread.  The alarm_mutex will be used to keep reading and 
* writing to this list protected from other threads*/ 

typedef struct thread_tag
{
    int                 num_alarms; //the number of alarms assigned to the thread
    unsigned int        thread_id; //the thread's id number 
    struct thread_tag*  next; //The next element in the thread tag linked list
    unsigned int        assigned_alarms[MAX_THREAD_ALARMS];  //an array of alarm ids that are assigned

}threadtag_t;



//thread related functions declarations
void *alarm_thread (void *arg);
int removefromThreadList(threadtag_t **head,unsigned int threadid);
threadtag_t* isinThreadList(threadtag_t *tag,pthread_t threadid);
void showThreadList(threadtag_t *tag);
threadtag_t* newThreadListEntry(threadtag_t **head,pthread_t thread_id);
void tryAssignAlarm(alarm_t *head,threadtag_t *thisThreadTag);
void *alarm_thread (void *arg);



#endif
