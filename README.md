# AlarmSystem
This repository contains code for an alarm system project coded in C programming language. The alarm system works by multi threaded programming tactics to ensure concurrent execution of multiple alarm systems at one time.   

To compile and run the program:

1) Copy all source files into a folder on your computer
2) From the terminal change your current directory into the folder with the source files 
3) type 'make' and press enter to compile the program
4) Run the executable with the command :
./NewAlarmThread


To test the program:
1) Run program with the 'alarmTest.sh' script as the input, for example:
 ./alarmTest.sh | ./NewAlarmThread > test.txt
2) To edit the number of alarms, edit the 'numalarms' variable in the 'alarmTest.sh' script

