/**
 * @file main_thread.h
 * This header file, paired with main_thread.c, contains all of the functions related to the main 
 * thread's processing.
 */

#ifndef __mainthread_h
#define __mainthread_h

#include <pthread.h>
#include <time.h>
#include <string.h>

#include <ctype.h>
#include "errors.h"
#include "alarm_threads.h"

#define ALARM_STATUS_UNASSIGNED 0
#define ALARM_STATUS_ASSIGNED 1
#define ALARM_STATUS_MODIFIED 2

#define STR_BUFFER_SIZE 256 //size of any temporary allocated char buffers
#define STR_MAX_MESSAGE_SIZE 128 //maximum size of a command 
#define INPUT_BUFFER_SIZE 512 //256 byte input buffer
#define INPUT_MAX_ARGS 3

#define START_ALARM 0
#define CHANGE_ALARM 1
#define CANCEL_ALARM 2
#define VIEW_ALARMS 3


//forward declaration 
typedef struct thread_tag threadtag_t;

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
typedef struct alarm_tag {
    struct alarm_tag    *link;
               
    time_t              time;   /* seconds from EPOCH.  Holds the absolute time the alarm will expire*/
    char                message[128]; //increased from 64 to 128 characters
    
    unsigned int        alarm_id; //alarm id stored as a positive integer
    time_t              create_time; //holds the time that the alarm was created
    time_t              assign_time; //holds the time the alarm was assigned to a thread
    char                status;  //0=unassigned 1=assigned 2=modified (see define at top of this file ALARM_STATUS_ASSIGNED AND ALARM_STATUS_UNASSIGNED ect..)
} alarm_t;

struct thread_args
{
    alarm_t** alarm_list_head;
    threadtag_t** thread_list_head;
    pthread_mutex_t* mutex;
};

int removefromAlarmList(alarm_t **head,unsigned int alarm_id);
int newAlarm(alarm_t **head,unsigned int alarm_id,int time,char* message);
void showAlarmList(alarm_t *tag);
void viewAlarms(threadtag_t *tag,alarm_t* alarm_list_head);
alarm_t* getAlarm(alarm_t *tag, int alarm_id);
int needCreateThread(threadtag_t* head);
int countChar(const char* str,char c);
int strIsNumeric(const char* str);
char interpret(const char* line,int* type,int* alarmid,int* time,char* message);
void strRTrim(char* str);
int getAlarmId(char* str);
int changeAlarm(alarm_t* head,int alarm_id, unsigned int time, char* message);
void clearbuffer();


#endif 
