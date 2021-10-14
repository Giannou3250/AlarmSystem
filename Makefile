NewAlarmthread: New_Alarm_Mutex.c main_thread.c alarm_threads.c
	cc New_Alarm_Mutex.c main_thread.c alarm_threads.c -o NewAlarmThread -lpthread -DPOSIX_PTHREAD_SEMANTICS