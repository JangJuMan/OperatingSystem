all: submitter instagrapd worker

submitter: submitter.c
	gcc -o submitter submitter.c

instagrapd: instagrapd.c
	gcc -o instagrapd instagrapd.c

worker: worker.c
	gcc -o worker worker.c

clean:
	rm -rf submitter instagrapd worker
