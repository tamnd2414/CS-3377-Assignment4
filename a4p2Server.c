/*
Assignment 4: Part 2 Server
Author: Tam Nguyen
Date: 12/02/2017
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sqlite3.h>
#include <fcntl.h>

int clientCount = 0;//number of clients
int byteCount = 0;//number of bytes to be written to result file
pthread_mutex_t lock;//mutex lock

struct serverParm{
	int connectionfd;
	int clientNO;
};

static int callback(void *data, int argc, char **argv, char **azColName) {
	int *fileDesc;
	fileDesc = (int *) data;
	int i;
	for (i = 0; i < argc; i++) {
		//write result from sql statement to result file
		byteCount += write(*fileDesc, azColName[i], strlen(azColName[i]));
		byteCount += write(*fileDesc, " = ", 3);
		byteCount += write(*fileDesc, argv[i], strlen(argv[i]));
		byteCount += write(*fileDesc, "\n", 1);
	}
	byteCount += write(*fileDesc, "\n", 1);
	return 0;
}

void sqlHandler(char cmd[]) {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[1025];
	//set up result file
	int *fileDesc;
	fileDesc = (int *) malloc(sizeof(fileDesc));
	*fileDesc = open("result.txt", O_APPEND | O_WRONLY);

	rc = sqlite3_open("book.db", &db);
	if (rc) {
		fprintf(stderr, "Can't open database\n", sqlite3_errmsg(db));
		exit(0);
	}
	else {
		//fprintf(stdout, "Open database successfully\n");
	}

	strcpy(sql, cmd);
	printf("Statement: %s\n", sql);

	rc = sqlite3_exec(db, sql, callback, (void*)fileDesc, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		byteCount += write(*fileDesc, "SQL error: ", 11);
		byteCount += write(*fileDesc, zErrMsg, strlen(zErrMsg));
		byteCount += write(*fileDesc, "\n", 1);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	close(*fileDesc);
	free(fileDesc);
}//end of sqlHandler

void *threadFunction(void *parmPtr){
	#define PARMPTR ((struct serverParm *) parmPtr)
	char cmd[1025];
	int cmdLen;
	char result[4096];//buffer to read from result file
	int readfd;//file descriptor to read from result file
	int writefd;//file descriptor to write to result file
	//thread id
	pthread_t tid;
	tid = pthread_self();
	//current time
	time_t curtime;
	struct tm *timestamp;
	char timeHeading[100];

	printf("Connection established: %d\n", PARMPTR->connectionfd);
	clientCount++;
	PARMPTR->clientNO = clientCount;
	while((cmdLen = recv(PARMPTR->connectionfd, cmd, 1025, 0)) > 0){
		pthread_mutex_lock(&lock);//lock the mutex before handling tasks
		if(strcmp(cmd, "exit") == 0){
			printf("Client #%d disconnected\n", PARMPTR->clientNO);
			close(PARMPTR->connectionfd);
			free(PARMPTR);
			clientCount--;
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
		}
		//get current time then write it to file
		curtime = time(NULL);
		timestamp = localtime(&curtime);
		strftime(timeHeading, sizeof(timeHeading), "%c", timestamp);
		writefd = open("result.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644);
		byteCount += write(writefd, timeHeading, strlen(timeHeading));
		byteCount += write(writefd, " Thread ID: ", 12);
		close(writefd);
		//write thread id to file
		FILE *file;
		file = fopen("result.txt", "a");
		byteCount += fprintf(file, "%lu (0x%lx)\n", (unsigned long)tid, (unsigned long)tid);
		fclose(file);
		//Server message and write to log
		printf("%s ProcessID: %d ThreadID: %lu (0x%lx)\nProcessing Client#%d's request: %s\n", timeHeading, getpid(), (unsigned long)tid, (unsigned long)tid, PARMPTR->clientNO, cmd);
		FILE *logfile;
		logfile = fopen("a4p2ServerLog.txt", "a");
		fprintf(logfile, "%s ProcessID: %d ThreadID: %lu (0x%lx)\nProcessing Client#%d's request: %s\n\n", timeHeading, getpid(), (unsigned long)tid, (unsigned long)tid, PARMPTR->clientNO, cmd);
		fclose(logfile);
		//handle the sql statement
		sqlHandler(cmd);
		//flush the socket buffer
		recv(PARMPTR->connectionfd, cmd, 1025, 0);
		//send result back to client
		readfd = open("result.txt", O_RDONLY);
		read(readfd, result, byteCount);
		//printf("%s\n", result);
		send(PARMPTR->connectionfd, result, strlen(result), 0);
		memset(&result, 0, sizeof(result));//clear buffer
		send(PARMPTR->connectionfd, result, 4096, 0);//flush socket buffer
		byteCount = 0;
		pthread_mutex_unlock(&lock);//unlock the mutex after finishing the tasks
	}//end of while: receiving cmd from client
	close(PARMPTR->connectionfd);
	free(PARMPTR);
	clientCount--;
	pthread_exit(NULL);
}

int main(){
	int listenSocket, acceptSocket, n;
	struct sockaddr_in cliAddr, servAddr;
	pthread_t threadID;
	struct serverParm *parmPtr;
	
	//initialize mutex
	if(pthread_mutex_init(&lock, NULL) != 0){
		perror("Failed to initialize mutex");
		exit(1);
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(6000);

	bind(listenSocket, (struct sockaddr *)&servAddr, sizeof(servAddr));

	listen(listenSocket, 5);
	printf("Starts listening...\n");

	while(1){
		acceptSocket = accept(listenSocket, NULL, NULL);
		printf("Client accepted!!! Assigning thread to serve client\n");
		parmPtr = (struct serverParm *) malloc(sizeof(struct serverParm));
		parmPtr->connectionfd = acceptSocket;
		if(pthread_create(&threadID, NULL, threadFunction, (void *)parmPtr) != 0){
			perror("Failed to create thread");
			close(acceptSocket);
			close(listenSocket);
			exit(1);
		}//end of if: thread created
		printf("Server ready for another client\n");
	}//end of while: infinite loop 
	close(listenSocket);
}//end of main
