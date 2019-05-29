all:
	gcc -o abba -g abba.c -pthread
	gcc -shared -fPIC -o ddetector.so ddetector.c -ldl
	gcc -o dinning_deadlock -g dinning_deadlock.c -pthread
