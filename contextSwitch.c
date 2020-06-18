/* Author: Ashraf Taifour
 * Date: June 18 2020
 * This program uses only ONE CPU core, and it uses pipe to communicate between the child and the parent.
 * as a result the print out expected to the terminal will be different than how it would be on 
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h>

int main()
{
	cpu_set_t my_set;        /* Defines the cpu_set bitmask. */
	CPU_ZERO(&my_set);       /* Initialize it to 0 meaning nothing is selected */
	CPU_SET(1, &my_set);     /* the bit is now set to select core 1. */
	sched_setaffinity(0, sizeof(cpu_set_t), &my_set); /* Set affinity of tihs process to the defined mask, i.e. only 1.  */

	struct timeval start, end; //time variables

	int fd1[2]; //first end of pipe
	int fd2[2]; //second end of pipe


	//strings that will be printed to the file
	char * child_str = "Hello from child\n";
	char * parent_str = "Hello from parent\n";

	//strings that will be received by child and parent
	char newc_str[100];
	char newp_str[100];

	int fd = open("file.txt",  O_RDWR | O_CREAT | O_APPEND, 0777); //creates file.txt with RDWR so we can check the printout
	
	/* ERROR CHECKING BEGINS */
	if(fd == -1)
		perror("open error\n");
	
	int pipe_err1 = pipe(fd1);
	if(pipe_err1 == -1)
		perror("pipe1 error\n");
	
	int pipe_err2 = pipe(fd2);
	if(pipe_err2 == -1)
		perror("pipe2 error\n");

	int fork_ret = fork();
	if(fork_ret == -1)
		perror("fork error\n");

	/* ERROR CHECKING ENDS */

	else if(fork_ret == 0)
	{
		close(fd1[0]); //close read for first pipe
		close(fd2[1]); //close write for second pipe

		write(fd1[1],child_str, strlen(child_str)+1); //write to pipe, first context switch starts here, jumps to parent's code

		printf("child wrote to parent, now waiting for parent...\n");

		read(fd2[0], newc_str, sizeof(newc_str));
		write(fd, newc_str, strlen(newc_str)+1);

		printf("child wrote to file.txt\n");
	}

	else
	{
		close(fd1[1]); //close write for first pipe
		close(fd2[0]); //close read for second pipe

		read(fd1[0], newp_str, sizeof(newp_str));
		
		write(fd, newp_str, strlen(newp_str)+1);

		printf("parent wrote to file.txt\n");

		//sleep(1); //see what happens if you uncomment this!
		
		write(fd2[1], parent_str, strlen(parent_str)+1); //write to pipe

		printf("parent wrote to child\n");

		//first context switch will end here after all parent's code is executed, now it will jump to the child's first printf instruction
	}	
	close(fd);

	return 0;
}
