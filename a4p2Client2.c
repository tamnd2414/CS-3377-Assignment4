/*
Assignment 4: Part 2 Client 2
Author: Tam Nguyen
Date: 12/02/2017
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
	int sockfd;
	struct sockaddr_in servAddr;
	char cmd1[1025] = "update classics set author = '***UNKNOWN***' where title = 'Pride and Prejudice'";
	char cmd2[1025] = "update classics set author = 'Jane Austen' where title = 'Pride and Prejudice'";
	char result[4096];
	int len;
	int recvlen;
	char flush[1025];
	int logfd;
	char separator[50] = "-------------------------------------------\n";
	logfd = open("a4p2Client2Log.txt", O_CREAT | O_APPEND | O_WRONLY, 0644);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("129.110.92.15");
	servAddr.sin_port = htons(6000);

	if(connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
		perror("Connection failed");
		exit(3);
	}
	printf("Connection established!!!\n");
	while(1){
		//sending first sql statement
		send(sockfd, cmd1, strlen(cmd1), 0);
		memset(&flush, 0, 1025);
		send(sockfd, flush, 1025, 0);
		recv(sockfd, result, 4096, 0);
		printf("%s\n", result);
		write(logfd, "Statement: ", 11);
		write(logfd, cmd1, strlen(cmd1));
		write(logfd, "\n", 1);
		write(logfd, result, strlen(result));
		write(logfd, separator, strlen(separator));
		recv(sockfd, result, 4096, 0);//flush socket buffer and clear result
		sleep(2);
		//sending second sql statement
		send(sockfd, cmd2, strlen(cmd2), 0);
		send(sockfd, flush, 1025, 0);
		recv(sockfd, result, 4096, 0);
		printf("%s\n", result);
		write(logfd, "Statement: ", 11);
		write(logfd, cmd2, strlen(cmd2));
		write(logfd, "\n", 1);
		write(logfd, result, strlen(result));
		write(logfd, separator, strlen(separator));
		recv(sockfd, result, 4096, 0);//flush socket buffer and clear result
		sleep(2);
	}
	
	exit(0);
}
