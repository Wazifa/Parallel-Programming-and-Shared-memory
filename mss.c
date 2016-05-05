/*
 * Name: Wazifa Jafar
 * Description: A program that will read a text file and return the number of
 * 		times that the user's input appears in the text file and 
 * 		the time it takes to find them.
 * 		To do this, it use shared memory, multiple processes and pipes. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>

int main( void ) 
{
  char input[255], **arr, *filename = "shakespeare.txt";
  int status;

  int fd;
  int pfd[100][2];
  char *data;
  struct stat buf; 
  struct timeval begin;
  struct timeval end;

  signal (SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

/*
 * The program will copy the text file into shared memory and the processes will use the shared
 * memory to find the number of words.
 */
 
  pid_t pid[100];
  fd = open("shakespeare.txt", O_RDONLY);
  stat("shakespeare.txt", &buf);
    
  data = mmap((caddr_t)0, buf.st_size, PROT_READ, MAP_SHARED, fd, 0) ;

  printf("Welcome to the Shakespeare word count service.\n");
  printf("Enter: search [word] [workers] to start your search.\n");
/*
 * The program will first check for help or quit and if none it will go into the search branch
 * of if-else.
*/

  while(1)
  {
    printf("> ");

    arr = malloc(4*sizeof(char*));
    char *temp;
    int i = -1;

    fgets(input, 255, stdin);

    temp = strtok(input, " \r\n\0");

    if (temp == NULL)
    {

       continue;

    }

    arr[i] = malloc(strlen(temp));
    arr[i] = temp;
    i++;

    while(temp != NULL)
    {
      arr[i] = malloc(strlen(temp));
      arr[i] = temp;
      temp = strtok(NULL, " \r\n\0");
      i++;
    }

    if(strcmp(arr[0], "quit") == 0)
    {
      exit(0);
    }

    else if (strcmp(arr[0], "help") == 0)
    {
      printf("Shakespeare Word Search Service Command Help\n");
      printf("_____________________________________________\n");
      printf("help    - displays this message.\n");
      printf("quit    - exits\n");
      printf("search [word] [workers] - searches the works of Shakespeare for [work] using [workers].");
      printf("  [workers] can be from 1 to 100.\n");
    }
    
/*
 * This branch will divide the work (using fork) into the provided number of workers and count 
 * the number of files from the text file.
 * It will use pipes to get the count from the children and pass to the parent then print 
 * the output to the user along with the time it took to find the number of instances.
 */

    else if (strcmp(arr[0], "search") == 0)
    {
      int x = 0, count =0, workers = atoi(arr[2]);
      int str_worker, work_load = buf.st_size / workers;
      int end_address;
      for (str_worker = 1; str_worker <= workers; str_worker++)
      {
	gettimeofday(&begin, NULL);
        pipe(pfd[str_worker-1]);
        end_address = work_load * str_worker;

        if ((pid[str_worker-1] = fork()) == 0)
        {
          for (x; x<end_address; x++)
          {
            if(! memcmp(data+x, arr[1], strlen(arr[1])))
            {
              count++;
            }
          }
          
          x = end_address + 1;
          close(pfd[str_worker-1][0]);
          write(pfd[str_worker-1][1], &count, sizeof(int));
          exit(0);   
        }

        else
        {
          wait(&status);     
        }
	
	gettimeofday(&end, NULL);
	
      }
      int q;

      for (q=0; q<workers; q++)
      {
        close (pfd[q][1]);
        read(pfd[q][0], &count, sizeof(int));
      }
      
      float elapsed_time = (end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec- begin.tv_usec);
      printf("Found %d instances of %s in %f microseconds\n", count, arr[1], elapsed_time);
    }
    
    else
    {
      printf("Command not found\n");
    }
 
  }

  return 0;
}
