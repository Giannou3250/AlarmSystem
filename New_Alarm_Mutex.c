/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include "errors.h"
#include "alarm_threads.h" //seperate the alarm_threads code 
#include "main_thread.h"//seperate the main thread code

threadtag_t *thread_list=NULL; //defined in alarm_threads.h
alarm_t *alarm_list=NULL; //defined in main_thread.h

pthread_mutex_t alarm_mutex;
pthread_t thread;

//#define DEBUG //uncomment this line if you want debug messages

//macro to display prompt for convenience 
#define prompt_message() printf("Alarm>") 


int main (int argc, char *argv[])
{

    thread_list = NULL;
    alarm_list = NULL;
    int status;
   
    char inputbuffer[INPUT_BUFFER_SIZE];
    int commandtype,alarmid,timeval;
    char message[STR_MAX_MESSAGE_SIZE]; //make a multi-dimensional array for the arguments
    int commandvalid=0;
    int retval;
    alarm_t* tmp_alarm;

    //create the structure to pass aruments to the newly created thread 
    struct thread_args threadarguments;
    threadarguments.alarm_list_head=&alarm_list;
    threadarguments.thread_list_head=&thread_list;
    threadarguments.mutex=&alarm_mutex;
    
    printf("\n");
    while(1)
    {
        //show prompt
        prompt_message();

        //wait for user input 
        fgets(inputbuffer,128,stdin);

        //if the line doesn't have the newline character ensure the input buffer is emptied
       if(!strchr(inputbuffer,'\n'))
            clearbuffer();
      

        //interpret command
        if(retval = interpret(inputbuffer,&commandtype,&alarmid,&timeval,message))
        {
            //try and lock the mutex
            status = pthread_mutex_lock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Lock mutex");
            #ifdef DEBUG
                printf("Valid: %d\n",retval);
                printf("Command type is: %d\n",commandtype);
                printf("alarmid:%d\n",alarmid);
                printf("timeval:%d\n",timeval);
                printf("message:%s\n",message);
            #endif
            switch(commandtype)
            {
                case START_ALARM:
                    if(needCreateThread(thread_list))
                    {
                          status = pthread_create (&thread, NULL, alarm_thread, &threadarguments);
                        if (status != 0)
                            err_abort (status, "Create alarm thread");
                    }
                    if(!newAlarm(&alarm_list,alarmid,timeval,message))
                        printf("An alarm with  id %d already exists. You cannot create an alarm with the same id number.\n",alarmid);
                    
                break;
                case CHANGE_ALARM:
                    changeAlarm(alarm_list,alarmid,timeval,message);
                break;
                case VIEW_ALARMS:
                    viewAlarms(thread_list,alarm_list);
                break;
                case CANCEL_ALARM:

                    //cache a copy of the alarm's message and time for printing to the console after the entry is removed 
                    tmp_alarm = getAlarm(alarm_list,alarmid);
                    if(tmp_alarm != NULL)
                    {
                        int tmp_time = tmp_alarm->time;
                        char tmp_message[STR_MAX_MESSAGE_SIZE + 1]; //add one elemnt for including the NULL character
                        strcpy(tmp_message,tmp_alarm->message);

                        if(removefromAlarmList(&alarm_list,alarmid))
                        {   
                            //better for removefromAlarmList not to print inside of the function since it is used
                            //by the main thread for cancelling alarms and the alarm thread for expired alarms.  Each aformentioned action
                            //requires a different message to be displayed depending on the context.  
                            
                            //print the cancelled message
                            printf("Alarm %u Cancelled at %d: %d %s\n",alarmid,(int)time(NULL),tmp_time, tmp_message);
                        }
                    }
                break;
            }
            //prompt_message();

            //unlock the mutex
            status = pthread_mutex_unlock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");           
        }
        else
        {
            printf("The command is invalid.  Your syntax is probably incorrect.\n");
        }
    }

    return 0;
}







