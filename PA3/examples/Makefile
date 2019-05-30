all:
	gcc -o abba -g abba.c -pthread
	gcc -shared -fPIC -o ddetector.so ddetector.c -ldl
	gcc -o dinning_deadlock -g dinning_deadlock.c -pthread
	gcc -shared -fPIC -o dmonitor.so dmonitor.c -ldl
	gcc -o DPredictor -g DPredictor.c -pthread
	gcc -o comm_deadlock -g comm_deadlock.c -pthread
	gcc -o gateLock -g gateLock.c -pthread
	gcc -o thread_creation -g thread_creation.c -pthread
	gcc -o single_thread -g single_thread.c -pthread

ABBA:
	LD_PRELOAD=./ddetector.so ./abba

DIN:
	LD_PRELOAD=./ddetector.so ./dinning_deadlock

MONITOR_ABBA:
	LD_PRELOAD=./dmonitor.so ./abba

MONITOR_DIN:
	LD_PRELOAD=./dmonitor.so ./dinning_deadlock

clean:
	rm abba
	rm ddetector.so
	rm dinning_deadlock
	rm dmonitor.so
	rm DPredictor
	rm comm_deadlock
	rm gateLock
	rm thread_creation
	rm single_thread
