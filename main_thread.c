#include <pthread.h>
#include <time.h>
#include "errors.h"
#include "alarm_threads.h"
#include "main_thread.h"//seperate the main thread code

extern threadtag_t *thread_list; //defined in alarm_threads.h


/** This function determines if we need to create another thread.  If each thread has MAX_THREAD_ALARMS alarms assigned (In our case 2 alarms)
 * then we need to add another thread.
*
*@param head        The pointer to the head of the thread list.
@return             Returns 1 if more threads are needed and 0 otherwise.
*/
int needCreateThread(threadtag_t* head)
{
    threadtag_t *current_tag=head;

    //go through the list of threadtags and check to see if they are all full
    while(current_tag!=NULL)
    {
        //we found a thread that has capacity for another alarm we don't need another thread
        if(current_tag->num_alarms < MAX_THREAD_ALARMS)
            return 0;
        //go to the next thread in the list
        current_tag=current_tag->next;
    }
    //all of the threads are full capacity so we need threads
    return 1;
}



/** This function removes an alarm from a list of alarms and frees its heap allocated memory.  This function will
* modify the head pointer if the first alarm in the list is being removed.
*
*@param head        The pointer to the head of the list
*@param alarm_id    The alarm's id number
@return         Returns 1 on success 0 on failure
*/
int removefromAlarmList(alarm_t **head,unsigned int alarm_id)
{

    alarm_t *prev_tag=NULL;
    alarm_t *current_tag = *head;

    //search for the tag while within the list
    while(current_tag != NULL)
    {
        //if we find this threads id in the list of alarms
        if(current_tag->alarm_id == alarm_id)
        {
            //CASE1: this is the first entry in the list
            if(prev_tag == NULL)
            {
               //remap the head pointer
               *head=current_tag->link;
            }
            //CASE2: remove from end of the list
            else if(current_tag->link == NULL)
            {
                //re-terminate the end
                prev_tag->link=NULL;
            }
            //CASE3: node is in the middle of the list
            else
                //assign the previous tag this tag's next tag
                prev_tag->link = current_tag->link;

            //free the memory of the current tag
            free(current_tag);
            return 1;
        }
        //save the previous tag before following
        prev_tag = current_tag;
        //follow to the next tag
        current_tag = current_tag->link;
    }
    return 0;


}

/** This function puts a new alarm into the list of alarms.
*@param head        The pointer to the head of the alarm_list
*@param alarm_id    The id of the alarm
*@param seconds     The number of seconds until the alarm goes off
*@param message     The message of the alarm

@return             returns 1 on success and 0 on failure
*/
int newAlarm(alarm_t **head,unsigned int alarm_id,int seconds,char* message)
{
     alarm_t *prev_tag=NULL;
    alarm_t *current_tag = *head;

    //if there is already an alarm in the list return
    if(getAlarm(*head,alarm_id)!=NULL)
        return 0;
    //create the new alarm tag
     alarm_t *newalaramtag = (alarm_t *)malloc (sizeof (alarm_t));
    if (newalaramtag == 0)
        errno_abort ("Couldn't allocate memory for a new thread tag");

    //assign default values to fields
    newalaramtag->link=NULL;
    newalaramtag->alarm_id=alarm_id;
    newalaramtag->time=seconds;
    newalaramtag->create_time = time(NULL);
    newalaramtag->status = ALARM_STATUS_UNASSIGNED;
    //make a true copy of the message and put into the struct
    strcpy(newalaramtag->message,message);

    //search for the tag while within the list
    while(current_tag != NULL)
    {
        //ascending insertion into list
        if(current_tag->link != NULL)
        {
            //if the next item in the list is larger then we have found a position in the list
            if(current_tag->link->alarm_id > alarm_id)
                break;
        }
        //save the previous tag before following
        prev_tag = current_tag;
        //follow to the next tag
        current_tag = current_tag->link;
    }


         //first thread tag in the list
        if(current_tag == NULL && prev_tag == NULL)
            //update the pointer to the head of the list because this is the first item in the list
            *(head) = newalaramtag;

        else if(current_tag == NULL)
            //point the last item in the list to this item because we are at the end of the list
            prev_tag->link = newalaramtag;
        else
        {
            /*middle of the list somewhere so assign the previous alarm tag to point to the new one
            and the new alarm tag to point to what the previous one had pointed to*/
            newalaramtag->link = current_tag->link;
            current_tag->link = newalaramtag;


        }
        return 1;

}
/** This function displays an alarm list by id starting from the tag* provided.
*@param tag    -The alarm tag to begin start displaying from
*

@return         No return value
*/
void showAlarmList(alarm_t *tag)
{
    //if the tag is initially null the list is empty so return NULL

    while(tag != NULL)
    {
        printf("->%d\n",tag->alarm_id);
        if(tag->status == ALARM_STATUS_ASSIGNED)
            printf("    ->Status: ASSIGNED\n");
        if(tag->status == ALARM_STATUS_UNASSIGNED)
            printf("    ->Status: NOT ASSIGNED\n");
        printf("    ->Message: %s\n",tag->message);
        //go to the next element
        tag = tag->link;
    }
}

/** This function shows the alarms and their associated threads
*@param tag    -The thread tag to dislay from.  Normally the head of the list
*

@return         No return value
*/
void viewAlarms(threadtag_t *tag,alarm_t* alarm_list_head)
{
    int thread_counter=1;
    printf("View Alarms at %d\n",(int)time(NULL));
    while(tag != NULL)
    {
        printf("%d. Display Thread %u Assigned:\n",thread_counter,tag->thread_id);

       int i;
        for(i=0;i<MAX_THREAD_ALARMS;i++)
        {
            alarm_t* current_alarm = getAlarm(alarm_list_head,tag->assigned_alarms[i]);

            if(current_alarm != NULL && tag->assigned_alarms[i] != ALARM_STATUS_UNASSIGNED)
                printf("%d%c. Alarm(%d): Created at %ld Assigned at %ld %d %s\n",i+1,97+i,tag->assigned_alarms[i],
                current_alarm->create_time,current_alarm->assign_time,(int)current_alarm->time,current_alarm->message);
        }

        //go to the next element
        tag = tag->next;
        thread_counter++;
    }
}
/** This function displays an alarm list by id starting from the tag* provided.
@param tag          The tag to start searching from in the list.
*@param alarm_id    The alarm's alarm id
*

@return        The alarm's address if found and NULL otherwise.
*/
alarm_t* getAlarm(alarm_t *tag, int alarm_id)
{
    //if the tag is initially null the list is empty so return NULL

    while(tag != NULL)
    {
        if(tag->alarm_id == alarm_id)
            return tag;
        //go to the next element
        tag = tag->link;
    }
    return NULL;
}

/** This function counts the number of instances of c in str.  The function expects the string
 * to be null terminated.
@param str          The string to search in.
@param str          The character to search for.
*
@return        The number of occurances of c in str.
*/
int countChar(const char* str,char c)
{
    int count=0,pos=0;
    while(str[pos] != '\0')
    {
        //if we found an occurance increase the count
        if(str[pos] == c)
            count++;
        //go to the next position
        pos++;
    }
    return count;
}

/** This function determines if the string contains only numeric characters.  The characters include
 * {0,1,2,3,4,5,6,7,8,9}.
* @param str          The string to search in.
*
@return             1 if the string is contains only numberic characters 0 otherwise
*/
int strIsNumeric(const char* str)
{
    int pos=0;
    while(str[pos] != '\0')
    {
        if(!isdigit(str[pos]))
            return 0;
        pos++;
    }
    return 1;
}

/** This function removes any trailing spaces from the string.
 *
* @param line         The input string to parse.
* @param type         Specifies where to put the resulting command type code.
* @param args         Specifies where to put each of the command's arguments
*
* @return             1 if the command is invalid and 0 if the command is not valid.
*/
void strRTrim(char* str)
{
    int pos = strlen(str)-1;
    while(str[pos] == ' ')
        pos--;
    //NULL terminate the string at the new position
    str[pos+1]='\0';

}



/** This function interprets a command and checks to see if it is valid.  If the command is valid
 * type is set to the command type and any passed arguments are set in args.  If the command is invalid
 * the function returns 0.  The command codes are as follows:
 * Start_Alarm=0
 * Change_Alarm=1
 * Cancel_Alarm=2
 * View_Alarms=3
 *
* @param line         The input string to parse.
* @param type         Specifies where to put the resulting command type code.
* @param args         Specifies where to put each of the command's arguments
*
* @return             1 if the command is invalid and 0 if the command is not valid.
*/
char interpret(const char* line,int* type,int* alarmid,int* time,char* message)
{
    static const  char* STR_CHANGE_ALARM = "Change_Alarm(";
    static const  char* STR_START_ALARM = "Start_Alarm(";
    static const  char* STR_CANCEL_ALARM = "Cancel_Alarm(";
    static const  char* STR_VIEW_ALARMS = "View_Alarms";
    static const char* STR_EXIT = "Exit";

    char words[INPUT_MAX_ARGS][STR_BUFFER_SIZE];
    char  trimmed_line[INPUT_BUFFER_SIZE];

    //remove any trailing whitespace from the input
    strcpy(trimmed_line,line);
    strRTrim(trimmed_line);

    int wordcount = sscanf(trimmed_line,"%s %s %128[^\n]",words[0],words[1],words[2]);
    if(wordcount >= 3)
    {
        //second string (time) is not positive integer
        if(!strIsNumeric(words[1]))
            return 0;

            //if the first word contains one '(',')' and the last character is ':'
        if(countChar(words[0],'(') == 1 && countChar(words[0],')') == 1 && words[0][strlen(words[0])-1] == ':' && words[0][strlen(words[0])-2] == ')')
        {
                //get the position of '('
            int openbracketpos = (int)(strchr(words[0],'(') - &words[0][0]);
            //get the position of ')'
            int closebracketpos = (int)(strchr(words[0],')') - &words[0][0]);
            //make sure the brackets appear in the correct order and the closing bracket
            if(closebracketpos - openbracketpos > 0)
            {
                //flag to see if a type was detected
                char type_is_found=0;

                //identify the command type
                if(strncmp(STR_CHANGE_ALARM,words[0],strlen(STR_CHANGE_ALARM)) == 0)
                {
                     *(type) = CHANGE_ALARM;
                     type_is_found=1;
                }
                else if(strncmp(STR_START_ALARM,words[0],strlen(STR_START_ALARM)) == 0)
                {
                     *(type) = START_ALARM;
                     type_is_found=1;
                }

                if(type_is_found)
                {
                    int id= getAlarmId(words[0]);
                    if (id > 0)
                    {
                        *(alarmid) = id;
                        *(time)= atoi(words[1]);
                        strncpy(message,words[2],128);
                        return 1;
                    }
                }
            }
            else
                //command is invalid
                return 0;
        }
        else
            //command is invalid
            return 0;



    }
    else if(wordcount == 1)
    {
        //if the command string has one opening and one closing bracket and the command
        //starts with 'Cancel_Alarm'
        if(countChar(words[0],'(') == 1 && countChar(words[0],')') == 1
        && strncmp(STR_CANCEL_ALARM,words[0],strlen(STR_CANCEL_ALARM)) == 0 )
        {

            //get the position of '('
            int openbracketpos = (int)(strchr(words[0],'(') - &words[0][0]);
            //get the position of ')'
            int closebracketpos = (int)(strchr(words[0],')') - &words[0][0]);
            //make sure the brackets appear in the correct order and the closing bracket
            //terminates the string
            if(closebracketpos - openbracketpos > 0 && words[0][closebracketpos + 1] == '\0')
            {
                int id = getAlarmId(&words[0][0]);
                if(id > 0)
                {
                     *(type) = CANCEL_ALARM;
                     *(alarmid) = id;
                     *(time)=0;
                     //*(message) = NULL;
                     //Change_Alarm command detected
                     return 1;
                }
                else
                    //not a valid command
                    return 0;

            }
            else
                //command is invalid
                return 0;


        }
        else if(strcmp(STR_VIEW_ALARMS,words[0]) == 0)
        {
            *(type) = VIEW_ALARMS;
            *(alarmid) = 0;
            *(time)=0;
            //*(message) = NULL;
            //Change_Alarm command detected
            return 1;

        }
        else if(strcmp(STR_EXIT,words[0]) == 0)
        {
            //terminate the program
            exit(0);
        }

    }
    //invalid command
    return 0;

}

/** This function extracts first positive integer contained within the first set of round braces.
 * Ex: hello(234) gets 234 as an integer value.

 * @param str               The string to search
 * @return                  A positive integer contained within the braces.  Returns 0 if invalid
 */
int getAlarmId(char* strAlarmId)
{
    char tmp[STR_BUFFER_SIZE];
    char itmp=0;
    char pushchar = 0;
    int pos;
    for(pos=0;pos<strlen(strAlarmId);pos++)
    {
        if(strAlarmId[pos]=='(')
            pushchar=1;
        else if(strAlarmId[pos]==')')
        {
            pushchar=0;
            tmp[itmp]='\0'; //null terminate the character
        }
        else if(pushchar)
        {
            //the current value is a not a digit alarmid is invalid
            if(!isdigit(strAlarmId[pos]))
                return 0;
            else
            {
                //copy the character into the temporary buffer
                tmp[itmp]=strAlarmId[pos];
                itmp++;
            }

        }
    }
    return atoi(tmp);
}


/** Changed an existing alarm by id number.  The function assumes that the time field is numeric and the alarm_id passed is
 * numeric.

 * @param alarm_id               The alarm's id number.
 * @param time                   The new time interval of the alarm.
 * @param message                The string to search
 * @return                       Returns 1 on success and 0 on failure.
 */
int changeAlarm(alarm_t* head,int alarm_id, unsigned int newtime, char* message)
{
    //get the reference to the alarm
    alarm_t* alarm = getAlarm(head,alarm_id);
    //if alarm is valid and matches current alarm id
    if(alarm != NULL)
    {
        //update the time and message fields
        alarm->time=newtime;
        strcpy(alarm->message,message);
        //flag the alarm status
        alarm->status = ALARM_STATUS_MODIFIED;
        return 1;
        //print the changed message
        printf("Alarm %u Changed at %d: %d %s\n",alarm_id,(int)time(NULL),(int)alarm->time, alarm->message);
    }
    //no reference found so the remove is a failure
    return 0;

}

void clearbuffer()
{
  while ((getchar()) != '\n');

}
