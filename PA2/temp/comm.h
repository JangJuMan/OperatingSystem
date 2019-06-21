#ifndef __COMM_H_
#define __COMM_H_

 

#define SEGSIZE        sizeof(_ST_SHM)

#define SHM_PATH        "."

#define SHM_KEY         'M'

#define SEM_KEY         'S'

 

typedef struct {



		int number_of_items;

		int ID[20];
/*
		char pw_1[40];
		char pw_2[40];
		char pw_3[40];
		char pw_4[40];
		char pw_5[40];
		char pw_6[40];
		char pw_7[40];
		char pw_8[40]; */
//		char* pw;
//		struct pw[40];
		char pw[20][9];	// ok	
		char file[20][4096];
} _ST_SHM;

/*
typedef struct {
		
		char password[10];

} pw;
*/
 

#endif  /* #ifndef __COMM_H_ */
