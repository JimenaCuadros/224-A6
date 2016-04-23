#include <stdio.h>       /* standard I/O routines                     */
#define __USE_GNU
#include <pthread.h>     /* pthread functions and data structures     */
#include <stdlib.h>      /* rand() and srand() functions              */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
/* number of threads used to service requests */
#define NUM_HANDLER_THREADS 3
#define BufferSize 1024
#define BIG_ENUF 4096 // For request header

int sockfd, newsockfd, portno, clilen,Connect_Count=0;
char buffer[BufferSize]; // for communicating with client
pid_t pid;int BufferNdx; //
FILE * F;
struct stat S;// to find file length
struct sockaddr_in serv_addr, cli_addr;
int n;
// some buffering for the child process
char * TmpBuffer, *SavePtr, *FileName, *GetToken;


/* global mutex for our program. assignment initializes it. */
/* note that we use a RECURSIVE mutex, since a handler      */
/* thread might try to lock it twice consecutively.         */
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/* global condition variable for our program. assignment initializes it. */
pthread_cond_t  got_request   = PTHREAD_COND_INITIALIZER;

int num_requests = 0;	/* number of pending requests, initially none */

/* format of a single request. */
struct request {
    int number;		    /* number of the request                  */
    struct request* next;   /* pointer to next request, NULL if none. */
};

struct request* requests = NULL;     /* head of linked list of requests. */
struct request* last_request = NULL; /* pointer to last request.         */

/*
 * function add_request(): add a request to the requests list
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex.
 * output:    none.
 */

 void error(char *msg)
 {
     perror(msg);
     exit(1);
 }

void
add_request(int request_num,
	    pthread_mutex_t* p_mutex,
	    pthread_cond_t*  p_cond_var)
{
    int rc;	                    /* return code of pthreads functions.  */
    struct request* a_request;      /* pointer to newly added request.     */

    /* create structure with new request */
    a_request = (struct request*)malloc(sizeof(struct request));
    if (!a_request) { /* malloc failed?? */
	fprintf(stderr, "add_request: out of memory\n");
	exit(1);
    }
    a_request->number = request_num;
    a_request->next = NULL;

    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(p_mutex);

    /* add new request to the end of the list, updating list */
    /* pointers as required */
    if (num_requests == 0) { /* special case - list is empty */
	requests = a_request;
	last_request = a_request;
    }
    else {
	last_request->next = a_request;
	last_request = a_request;
    }

    /* increase total number of pending requests by one. */
    num_requests++;

#ifdef DEBUG
    printf("add_request: added request with id '%d'\n", a_request->number);
    fflush(stdout);
#endif /* DEBUG */

    /* unlock mutex */
    rc = pthread_mutex_unlock(p_mutex);

    /* signal the condition variable - there's a new request to handle */
    rc = pthread_cond_signal(p_cond_var);
}

/*
 * function get_request(): gets the first pending request from the requests list
 *                         removing it from the list.
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex.
 * output:    pointer to the removed request, or NULL if none.
 * memory:    the returned request need to be freed by the caller.
 */
struct request*
get_request(pthread_mutex_t* p_mutex)
{
    int rc;	                    /* return code of pthreads functions.  */
    struct request* a_request;      /* pointer to request.                 */

    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(p_mutex);

    if (num_requests > 0) {
	a_request = requests;
	requests = a_request->next;
	if (requests == NULL) { /* this was the last request on the list */
	    last_request = NULL;
	}
	/* decrease the total number of pending requests */
	num_requests--;
    }
    else { /* requests list is empty */
	a_request = NULL;
    }

    /* unlock mutex */
    rc = pthread_mutex_unlock(p_mutex);

    /* return the request to the caller. */
    return a_request;
}

/*
 * function handle_request(): handle a single given request.
 * algorithm: prints a message stating that the given thread handled
 *            the given request.
 * input:     request pointer, id of calling thread.
 * output:    none.
 */
void
handle_request(struct request* a_request, int thread_id)
{

  //  memset(buffer, 0, BufferSize);

    memset(buffer, 0, BufferSize);n = read(newsockfd,buffer,BufferSize-1);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
    printf("%s\n",(TmpBuffer=strtok_r(buffer,"\n",&SavePtr)));
    GetToken = strtok_r(TmpBuffer," ",&SavePtr);
    printf("%s\n",GetToken);BufferNdx = 0;
    GetToken = strtok_r(NULL," ",&SavePtr);
    printf("%s After Get\n",GetToken);GetToken++;
// now open the file and send it to client ?

     if ((F =  fopen(GetToken,"r")) == NULL) printf("Bad Dog\n");
            else printf("Good Dog\n");
      int FileSize;
    if ((fstat(fileno(F),&S)==-1)) error("failed fstat\n"); // Need file size
    FileSize = S.st_size;
// Looks ok -- now let's write the request header
 // Let's just fill a buffer with header info using sprintf()
    char Response[BIG_ENUF];int HeaderCount=0;
    HeaderCount=0;//Use to know where to fill buffer with sprintf
        HeaderCount+=sprintf( Response+HeaderCount,"HTTP/1.0 200 OK\r\n");
        HeaderCount+=sprintf( Response+HeaderCount,"Server: Flaky Server/1.0.0\r\n");
        char * fileType;
        fileType = strtok_r(NULL,".", &SavePtr);
        if(strcmp(fileType,"jpeg")==0){
            printf("%s\n","JPEG" );
            HeaderCount+=sprintf( Response+HeaderCount,"Content-Type: image/jpeg\r\n");
        }
        //For mp3
        if(strcmp(fileType,"mp3")==0){
          printf("%s\n","MP3" );
            HeaderCount+=sprintf( Response+HeaderCount,"Content-Type: audio/mp3\r\n");
        }
        HeaderCount+=sprintf( Response+HeaderCount,"Content-Length:%d\r\n",FileSize);
 // And finally to delimit header
        HeaderCount+=sprintf( Response+HeaderCount,"\r\n");
        // Let's echo that to stderr to be sure !
        fprintf(stderr,"HeaderCount %d and Header\n",HeaderCount);
        write(STDERR_FILENO, Response, HeaderCount);
        write(newsockfd,Response,HeaderCount); // and send to client
// Now Serve That File in one write to socket
        char  *BigBuffer = malloc(FileSize+2);// Just being OCD -- Slack is 2
        fread(BigBuffer,1,FileSize,F);
        write(newsockfd,BigBuffer,FileSize);
        free(BigBuffer);
// Now close up this client's shop
     close(newsockfd);

  //   if (a_request) {
	// printf("Thread '%d' handled request '%d'\n",
	//        thread_id, a_request->number);
	// fflush(stdout);
  //   }
}

/*
 * function handle_requests_loop(): infinite loop of requests handling
 * algorithm: forever, if there are requests to handle, take the first
 *            and handle it. Then wait on the given condition variable,
 *            and when it is signaled, re-do the loop.
 *            increases number of pending requests by one.
 * input:     id of thread, for printing purposes.
 * output:    none.
 */
void*
handle_requests_loop(void* data)
{
    int rc;	                    /* return code of pthreads functions.  */
    struct request* a_request;      /* pointer to a request.               */
    int thread_id = *((int*)data);  /* thread identifying number           */

#ifdef DEBUG
    printf("Starting thread '%d'\n", thread_id);
    fflush(stdout);
#endif /* DEBUG */

    /* lock the mutex, to access the requests list exclusively. */
    rc = pthread_mutex_lock(&request_mutex);

#ifdef DEBUG
    printf("thread '%d' after pthread_mutex_lock\n", thread_id);
    fflush(stdout);
#endif /* DEBUG */

    /* do forever.... */
    while (1) {
#ifdef DEBUG
    	printf("thread '%d', num_requests =  %d\n", thread_id, num_requests);
    	fflush(stdout);
#endif /* DEBUG */
	if (num_requests > 0) { /* a request is pending */
	    a_request = get_request(&request_mutex);
	    if (a_request) { /* got a request - handle it and free it */
		handle_request(a_request, thread_id);
		free(a_request);
	    }
	}
	else {
	    /* wait for a request to arrive. note the mutex will be */
	    /* unlocked here, thus allowing other threads access to */
	    /* requests list.                                       */
#ifdef DEBUG
    	    printf("thread '%d' before pthread_cond_wait\n", thread_id);
    	    fflush(stdout);
#endif /* DEBUG */
	    rc = pthread_cond_wait(&got_request, &request_mutex);
	    /* and after we return from pthread_cond_wait, the mutex  */
	    /* is locked again, so we don't need to lock it ourselves */
#ifdef DEBUG
    	    printf("thread '%d' after pthread_cond_wait\n", thread_id);
    	    fflush(stdout);
#endif /* DEBUG */
	}
    }
}

/* like any C program, program's execution begins in main */
int
main(int argc, char* argv[])
{
    int        i;                                /* loop counter          */
    int        thr_id[NUM_HANDLER_THREADS];      /* thread IDs            */
    pthread_t  p_threads[NUM_HANDLER_THREADS];   /* thread's structures   */
    struct timespec delay;			 /* used for wasting time */

    /* create the request-handling threads */
    for (i=0; i<NUM_HANDLER_THREADS; i++) {
	thr_id[i] = i;
	pthread_create(&p_threads[i], NULL, handle_requests_loop, (void*)&thr_id[i]);
    }
    sleep(3);
    /* run a loop that generates requests */


	/* pause execution for a little bit, to allow      */
	/* other threads to run and handle some requests.  */




       if (argc < 2) {
           fprintf(stderr,"ERROR, no port provided\n");
           exit(1);
       }
       sockfd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockfd < 0)
          error("ERROR opening socket");
       memset( (char *) &serv_addr, 0, sizeof(serv_addr));
       portno = atoi(argv[1]);
       serv_addr.sin_family = AF_INET;
       serv_addr.sin_addr.s_addr = INADDR_ANY;
       serv_addr.sin_port = htons(portno);
       if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
                error("ERROR on binding");
  while (Connect_Count < 10)
  {
        add_request(i, &request_mutex, &got_request);
         listen(sockfd,5);
        clilen = sizeof(cli_addr);
        newsockfd =
            accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
       if (newsockfd < 0) // exit server
            error( "ERROR on accept");
       // otherwise let's fork
    }
    /* now wait till there are no more requests to process */
    sleep(5);

    printf("Glory,  we are done.\n");

    return 0;
}
