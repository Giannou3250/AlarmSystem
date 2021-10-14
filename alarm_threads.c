#include <pthread.h>
#include <time.h>
#include "errors.h"
#include "alarm_threads.h"
#include "main_thread.h"//seperate the main thread code


/** This function attempts to assign this thread to an alarm

*@param head            -The pointer to the head of the list
*@param thisThreadTag   -The pointer to this thread's thread_tag

@return         No return value
*/
void tryAssignAlarm(alarm_t *head,threadtag_t *thisThreadTag)
{

    alarm_t *prev_tag=NULL;
    alarm_t *current_tag = head;

    if(thisThreadTag->num_alarms == MAX_THREAD_ALARMS)
        return;
    //search for the tag while within the list
    while(current_tag != NULL)
    {
        //if we find an unassigned alarm then assign it
        if(current_tag->status == ALARM_STATUS_UNASSIGNED)
        {
          int i;
            for(i=0;i<MAX_THREAD_ALARMS;i++)
            {
                //find the empty slot for an assigned alarm
                if(thisThreadTag->assigned_alarms[i]==0)
                {
                    //update the empty slot
                    thisThreadTag->assigned_alarms[i]=current_tag->alarm_id;
                    break;
                }
            }

            //increment the number of alarms
            thisThreadTag->num_alarms++;

            //update assign time
            current_tag->assign_time=time(NULL);

            //update the assigned status
            current_tag->status= ALARM_STATUS_ASSIGNED;

            //NOT YET DEFINED
             //getTimeString(mytime,current_tab->assign_time());

            //display assigned message
            printf("Display Thread %u Assigned Alarm (%d) at %d: %d %s\n",thisThreadTag->thread_id,
            current_tag->alarm_id,(int)current_tag->assign_time,
            (int)current_tag->time,current_tag->message);


            return;
        }
        //save the previous tag before following
        prev_tag = current_tag;
        //follow to the next tag
        current_tag = current_tag->link;
    }

}




/** This function removes a thread from a list of threads and frees its heap allocated memory. This function will
* modify the head pointer if the first thread in the list is being removed.
* @param head       The pointer to the head of the list
* @param threadid   The thread's id number
*
@return         -Returns 1 on success 0 on failure
*/

int removefromThreadList(threadtag_t **head,unsigned int threadid)
{

    threadtag_t *prev_tag=NULL;
    threadtag_t *current_tag = *head;

    //search for the tag while within the list
    while(current_tag != NULL)
    {
        //if we find this threads id in the list
        if(current_tag->thread_id == threadid)
        {
            //CASE1: This is the head of the list
            if(prev_tag == NULL)
                //map the head to the next element
                *(head)=current_tag->next;
            //CASE2: This node is the end of the list
            else if(current_tag->next == NULL)
                prev_tag->next=NULL;
            //CASE3: node is in the middle of the list
            else
                //assign the previous tag this tag's next tag
                prev_tag->next = current_tag->next;
            //free the memory of the current tag
            free(current_tag);
            return 1;
        }
        //save the previous tag before following
        prev_tag = current_tag;
        //follow to the next tag
        current_tag = current_tag->next;
    }
    return 0;
}

/** This function searches a list of thread_tag by thread id number
*and returns the address to the thread_tag if found and NULL otherwise.
*@param tag    -The thread tag to begin the search from
*@param threadid -The id of the thread to search for in the list

@return         -A reference to the found thread_tag in the list.  NULL if no thread is found
*/

threadtag_t* isinThreadList(threadtag_t *tag,pthread_t threadid)
{
    //if the tag is initially null the list is empty so return NULL

    while(tag != NULL)
    {
        if(tag->thread_id == threadid)
            return tag;
        //go to the next element
        tag = tag->next;
    }
    return NULL;


}
/** This function displays a thread list by id starting from the tag* provided.
*@param tag    -The thread tag to begin the search from

@return         -A reference to the found thread_tag in the list.  NULL if no thread is found
*/

void showThreadList(threadtag_t *tag)
{
    //if the tag is initially null the list is empty so return NULL

    while(tag != NULL)
    {
        printf("Thread id->%d\n",tag->thread_id);
        printf("    Assigned Alarms->");
        int i;
        for(i=0;i<tag->num_alarms;i++)
        {
            printf("%d",tag->assigned_alarms[i]);
            //print the delimiter
            if (i < tag->num_alarms-1 )
                printf(",");
        }
        printf("\n");

        //go to the next element
        tag = tag->next;
    }
}

/** This function adds a new thread_tag into a list.  It may also change the value of the head tag pointer passed in if the list is empty.  Note that
* the ordering of the list does not change.
*
* @param thread_id  The thread id to insert into this list
* @param head       The reference to the list's head pointer
* @return (void)
*/

threadtag_t* newThreadListEntry(threadtag_t **head,pthread_t thread_id)
{
    threadtag_t *prev_tag=NULL;
    threadtag_t *current_tag = *head;
    threadtag_t * newthreadtag;

    //create the new thread tag
     newthreadtag = (threadtag_t*)malloc (sizeof (threadtag_t));
        if (newthreadtag == 0)
            errno_abort ("Couldn't allocate memory for a new thread tag");
    //assign default values to fields
    newthreadtag->next=NULL;
    newthreadtag->num_alarms=0;
    newthreadtag->thread_id = thread_id;
    //give all assigned alarms a default value of 0 to represent unassigned
    int i;
    for(i=0;i<MAX_THREAD_ALARMS;i++)
        newthreadtag->assigned_alarms[i]=ALARM_STATUS_UNASSIGNED;


    //walk through the list until we end up at the end of the list
    while (current_tag != NULL)
    {
        //save the previous tag before following
        prev_tag = current_tag;
        //follow to the next tag
        current_tag = current_tag->next;
    }

    //first thread tag in the list
    if(prev_tag == NULL)
        //update the pointer to the head of the list
        *(head) = newthreadtag;
    else
        //point the last item in the list to this item
        prev_tag->next = newthreadtag;
    return newthreadtag;
}

/*
 * The alarm thread's start routine.
 */
void *alarm_thread (void *arg)
{
    alarm_t *alarm;
    int sleep_time;
    time_t last_display_time = time(NULL);
    int status;

    struct thread_args* passed_arguments = (struct thread_args*) arg;

    alarm_t** alarm_list = passed_arguments->alarm_list_head;
    threadtag_t** thread_list = passed_arguments->thread_list_head;
    pthread_mutex_t* alarm_mutex = passed_arguments->mutex;

    status = pthread_mutex_lock (alarm_mutex);
    if (status != 0)
        err_abort (status, "Lock mutex");

    //put this thread into the list of threads and save the reference
    threadtag_t* mythreadtag = newThreadListEntry(thread_list,pthread_self());

     status = pthread_mutex_unlock (alarm_mutex);
    if (status != 0)
        err_abort (status, "Unlock mutex");

    /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */
    char exitcode = 0;

    while (exitcode==0)
    {
        status = pthread_mutex_lock (alarm_mutex);
        if (status != 0)
            err_abort (status, "Lock mutex");

        //if this thread needs to assign alarms to it still
        if(mythreadtag->num_alarms < MAX_THREAD_ALARMS)
        {
            //try to assign this thread to an alarm
            tryAssignAlarm(*alarm_list,mythreadtag);
        }
        //iterate through all of the assigned alarms
        int i;
        for(i=0;i<MAX_THREAD_ALARMS;i++)
        {
            //alarm id 0 is reserved for alarms that are not assigned
            if(mythreadtag->assigned_alarms[i]!=ALARM_STATUS_UNASSIGNED)
            {
                //get the current alarm
                alarm=getAlarm(*alarm_list,mythreadtag->assigned_alarms[i]);
                //if alarm is expired or alarm is not in the alarm list (cancelled)
                if(alarm == NULL ||  time(NULL) >= alarm->create_time + alarm->time)
                {
                    //remove the alarm from the alarm list if it is in the alarm list
                    if(alarm!=NULL)
                    {

                        //display removed alarm message
                        printf("Display Alarm Thread %u Removed Alarm %d at %d: %d %s\n",
                        mythreadtag->thread_id,mythreadtag->assigned_alarms[i],(int)time(NULL),(int)alarm->time, alarm->message);
                        removefromAlarmList(alarm_list,mythreadtag->assigned_alarms[i]);
                    }
                    else
                    {
                        //display removed alarm message
                        printf("Display Alarm Thread %u Removed Alarm %d at %d\n",
                        mythreadtag->thread_id,mythreadtag->assigned_alarms[i],(int)time(NULL));
                    }

                    //remove alarm from this thread's list
                    mythreadtag->assigned_alarms[i]=0;
                    mythreadtag->num_alarms--;

                }
            }
        }
        //display time has expired (5 seconds )
        if(time(NULL) >= last_display_time + ALARM_THREAD_DISPLAY_INTERVAL)
        {
            //print a message for each assigned alarm for this thread
            int i;
            for(i=0;i<MAX_THREAD_ALARMS;i++)
            {
                //if the assigned alarm is valid
                if(mythreadtag->assigned_alarms[i] != 0)
                {
                    alarm_t* thisalarm = getAlarm(*alarm_list,mythreadtag->assigned_alarms[i]);
                    if(thisalarm->status == ALARM_STATUS_MODIFIED)
                    {
                        printf("Display Thread %u Starts to Print Changed Message at %d: %d %s\n"
                                ,mythreadtag->thread_id,(int)time(NULL),(int)thisalarm->time,thisalarm->message);
                        thisalarm->status = ALARM_STATUS_ASSIGNED;
                    }
                    printf("Alarm %d Printed by Alarm Display Thread %u at %d: %d %s\n",
                    mythreadtag->assigned_alarms[i],mythreadtag->thread_id,(int)time(NULL),(int)thisalarm->time,thisalarm->message);
                }
            }
            //update last display time to the current time
            last_display_time=time(NULL);
        }

        //if the number of assigned alarms is 0
        if(mythreadtag->num_alarms == 0)
        {
            //display thread exit message
            printf("Display Alarm Thread %u Exiting at %d\n",mythreadtag->thread_id,(int)time(NULL));

            //remove thread tag from the thread list and free allocated memory
            removefromThreadList(thread_list,mythreadtag->thread_id);

            //unlock mutex
            status = pthread_mutex_unlock (alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");
            //set loop exit status
            exitcode=1;
        }
        else
        {
            /*unlock the mutex*/
            status = pthread_mutex_unlock (alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");

        }
        //give up this threads priority in the process queue
        sched_yield();
        //allow main thread to run 
        sleep(1);


    }

}
