#include <stdio.h>
#include "smalloc.h"

int 
main()
{
	void *p1, *p2, *p3, *p4, *p5, *p6;

	print_sm_containers() ;
//	print_sm_uses();

	p1 = smalloc(2000) ; 
	printf("smalloc(2000)\n") ; 
	print_sm_containers() ;
//	print_sm_uses();

	p2 = smalloc(500) ; 
	printf("smalloc(500)\n") ; 
	print_sm_containers() ;
//	print_sm_uses();

	p3 = smalloc(300);
	printf("smalloc(300)\n");
	print_sm_containers();
//	print_sm_uses();

	p4 = smalloc(3000);
	printf("smalloc(3000)\n");
	print_sm_containers();
//	print_sm_uses();

	sfree(p1) ; 
	printf("sfree(%p)\n", p1) ; 
	print_sm_containers() ;
//	print_sm_uses();

	sfree(p3);
	printf("sfree(%p)\n", p3);
	print_sm_containers();
//	print_sm_uses();

	p5 = smalloc(250) ; 
	printf("smalloc(250)\n") ; 
	print_sm_containers() ;
//	print_sm_uses();

	p6 = smalloc(400) ; 
	printf("smalloc(400)\n") ; 
	print_sm_containers() ;
//	print_sm_uses();
}
