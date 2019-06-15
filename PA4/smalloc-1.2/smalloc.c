#include <unistd.h>
#include <stdio.h>
#include "smalloc.h" 

sm_container_ptr sm_first = 0x0 ;
sm_container_ptr sm_last = 0x0 ;
sm_container_ptr sm_unused_containers = 0x0 ;

void sm_container_split(sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = hole->data + size ;

	remainder->data = ((void *)remainder) + sizeof(sm_container_t) ;
	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;
	remainder->status = Unused ;
	remainder->next = hole->next ;
	hole->next = remainder ;

	if (hole == sm_last)
		sm_last = remainder ;
}

void * sm_retain_more_memory(int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;

	hole->data = ((void *) hole) + sizeof(sm_container_t) ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	hole->status = Unused ;

	return hole ;
}

void * smalloc(size_t size) 
{

	sm_container_ptr hole = 0x0 ;
	sm_container_ptr first_unused = 0x0;
	int count = 0;

	sm_container_ptr itr = 0x0 ;
	if(sm_first != 0x0){
		sm_unused_containers = sm_first;
	}

	for (itr = sm_first ; itr != 0x0 ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;
		
		if(first_unused == 0x0){
			first_unused = itr;
		}

		// calculate unused space as 'next_unused'
		for(sm_unused_containers; sm_unused_containers != itr; sm_unused_containers = sm_unused_containers->next){
			if(sm_unused_containers == 0x0){
				break;
			}
			sm_unused_containers->next_unused = itr;
		}
	}

	for(sm_unused_containers = first_unused; sm_unused_containers != 0x0; sm_unused_containers = sm_unused_containers->next_unused){
		printf("\n>> this is un used space [%d]<<\n", (int)sm_unused_containers->dsize);
		if(size == sm_unused_containers->dsize){
			// a hole of the exact size
			sm_unused_containers->status = Busy;
			return sm_unused_containers;
		}
		// best fit
		else if (size + sizeof(sm_container_t) < sm_unused_containers->dsize){
			if(count == 0){
				hole = sm_unused_containers;
				count++;
			}
			if(hole->dsize > sm_unused_containers->dsize){
				hole = sm_unused_containers;
			}
		}
	}

	if (hole == 0x0) {
		hole = sm_retain_more_memory(size) ;

		if (hole == 0x0)
			return 0x0 ;

		if (sm_first == 0x0) {
			sm_first = hole ;
			sm_last = hole ;
			hole->next = 0x0 ;
		}
		else {
			sm_last->next = hole ;
			sm_last = hole ;
			hole->next = 0x0 ;
		}
	}
	sm_container_split(hole, size) ;
	hole->dsize = size ;
	hole->status = Busy ;
	return hole->data ;
}



void sfree(void * p)
{
	sm_container_ptr itr ;
	for (itr = sm_first ; itr->next != 0x0 ; itr = itr->next) {
		if (itr->data == p) {
			itr->status = Unused ;
			break ;
		}
	}
	
	sm_container_ptr first_unused = 0x0;
	sm_unused_containers = sm_first;
	// update
	for (itr = sm_first ; itr != 0x0 ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;

		if(first_unused == 0x0){
			first_unused = itr;
		}

		for(sm_unused_containers; sm_unused_containers != itr; sm_unused_containers = sm_unused_containers->next){
			if(sm_unused_containers == 0x0){
				break;
			}
			sm_unused_containers->next_unused = itr;
		}
	}
	
	for(itr = sm_first; itr->next != 0x0 && itr->next_unused != 0x0; itr = itr->next_unused){
		printf("itr[%d] >>\n", (int)itr->dsize);

		if(itr->status == Busy){
			printf("ggg\n");
			continue;
		}

		if(itr->next == itr->next_unused){
			// do merge
			itr->dsize = itr->dsize + sizeof(sm_container_t) + itr->next_unused->dsize;
			if(itr->next_unused->next != 0x0){
				itr->next = itr->next_unused->next;
			}
			else{
				itr->next = 0x0;
			}

			if(itr->next_unused->next_unused != 0x0){
				itr->next_unused = itr->next_unused->next_unused;
			}
			else{
				itr->next_unused = 0x0;
			}
		}
	}
}

void print_sm_containers()
{
	sm_container_ptr itr ;
	int i = 0 ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_first ; itr != 0x0 ; itr = itr->next, i++) {
		char * s ;
		printf("%3d:%p:%s:", i, itr->data, itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		for (s = (char *) itr->data ;
			 s < (char *) itr->data + (itr->dsize > 8 ? 8 : itr->dsize) ;
			 s++) 
			printf("%02x ", *s) ;
		printf("\n") ;
	}
	printf("=======================================================\n") ;

}

void print_sm_uses(){
	sm_container_ptr itr;
	int i=0;
	int unused = 0;
	int busy = 0;
	int allocated = 0;

	if(sm_first == 0x0){
		fprintf(stderr, "-- print_sm_uses --\n");
		fprintf(stderr, "[ Allocated : %d / Unused : %d / Busy : %d ]\n\n", allocated, unused, busy);
		return;
	}

	for(itr = sm_first; itr != 0x0; itr = itr->next, i++){
		char* s;
		if(itr->status == Unused){
			unused = unused + (int)itr->dsize;
		}
		else if(itr->status == Busy){
			busy = busy + (int)itr->dsize;
		}
		allocated = allocated + sizeof(sm_container_t);
	}

	allocated = allocated + busy + unused;

	fprintf(stderr, "-- print_sm_uses --\n");
	fprintf(stderr, "[ Allocated : %d / Unused : %d / Busy : %d ]\n\n", allocated, unused, busy);

}
