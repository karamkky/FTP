#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include <unistd.h>
#include <errno.h>

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

typedef union _semun
{

	int val;
	struct semid_ds *buf;
	ushort *array;

}semun;

