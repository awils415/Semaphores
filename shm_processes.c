#include <semaphore.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>


void deposit(int* BankAccount);
void withdraw(int* BankAccount);

int  main(int  argc, char *argv[])
{
		int ShmID;
		int *ShmPTR;
		pid_t pid;
		int status;

		int fd, i, nloop=10, zero=0, *localBalance;

		sem_t *mutex;

    //open a file and map it into memory this is to hold the shared counter

		fd = open("log.txt",O_RDWR|O_CREAT,S_IRWXU);
		write(fd,&zero,sizeof(int));
		localBalance = mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
		close(fd);

		/* create, initialize semaphore */
		if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
				perror("semaphore initilization");
				exit(1);
		}
	
		pid = fork();
		if (pid > 0) { /* child process*/
				for (i = 0; i < nloop; i++) {
						sleep(rand() % 6);
						printf("Dear Old Dad: Attempting to Check Balance\n");
						sem_wait(mutex);
						int rand_num = rand() % 101;
						if (rand_num % 2 == 0) {
								if (*localBalance < 100) {
										deposit(localBalance);
								}
								else {
										printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", *localBalance);
								}
						}
						else {
								printf("Dear Old Dad: Last Checking Balance = $%d\n", *localBalance);
						}
						sem_post(mutex);
				}
				printf("Exiting...\n");
				exit(1);
		}
		else if (pid == 0) {
				for (i = 0; i > -1; i++) {
						sleep(rand() % 6);
						printf("Poor Student: Attempting to Check Balance\n");
						sem_wait(mutex);
						int rand_num = rand();
						if (rand_num % 2 == 0) {
								withdraw(localBalance);
						}
						else {
								printf("Poor Student: Last Checking Balance = $%d\n", *localBalance);
						}
						sem_post(mutex);
				}
				printf("Exiting...\n");
				exit(0);
		}

		wait(&status);
		printf("Server has detected the completion of its child...\n");
		shmdt((void *) ShmPTR);
		printf("Server has detached its shared memory...\n");
		shmctl(ShmID, IPC_RMID, NULL);
		printf("Server has removed its shared memory...\n");
		printf("Server exits...\n");
		exit(0);
}

void deposit(int* BankAccount) {
		int localBalance = *BankAccount;
		int amount = rand() % 101;
		if (amount % 2 == 0) {
				localBalance += amount;
				printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
				*BankAccount = localBalance;
		}
		else {
			printf("Dear old Dad: Doesn't have any money to give\n");
		}
}

void withdraw(int* BankAccount) {
		int localBalance = *BankAccount;
		int need = rand() % 51;
		printf("Poor Student needs $%d\n", need);
		if (need <= localBalance) {
				localBalance -= need;
				printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
				*BankAccount = localBalance;
		}
		else {
				printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
		}
}