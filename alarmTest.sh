#!/bin/bash
numalarms=1000

#wait 3 seconds for the program to start running
sleep 3

#generate the alarms
for (( id = 1 ;id <= numalarms ; id++ ));do
	sleep 0.25;
	#generate another alarm
	change=$(( $id % 20 ))
	cancel=$(( $id % 51 ))
	view=$(( $id % 99 ))

	if [[ $change -eq 0 ]]; then 
	echo "Change_Alarm($(( $id - 12 ))): 40 GOODDAY World!!! How you doing?"
	sleep 0.25;
	echo "Change_Alarm($(( $id - 3 ))): 23 GOODDAY World!!! How you doing? Im doing GREAT!"
	sleep 0.25;
	echo "Change_Alarm($(( $id - 19 ))): 36 GOODDAY World!!! THE WEATHER is VERY NICE today ;)"
	elif  [[ $cancel -eq 0 ]]; then
	echo "Cancel_Alarm($(( $id - 30 )))"
	sleep 0.25;
	echo "Cancel_Alarm($(( $id - 35 )))"
	sleep 0.25;
	echo "Cancel_Alarm($(( $id - 15 )))"
	elif  [[ $view -eq 0 ]]; then
	echo "View_Alarms"
	fi
	sleep 0.25;
	echo "Start_Alarm(${id}): 60 Hello world"

done
sleep 0.5;
echo "Exit"
exit 0
