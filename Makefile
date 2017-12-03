a4: a4p1Server.c a4p1Client1.c a4p2Server.c a4p2Client1.c a4p2Client2.c a4p3Server.c a4p3Client1.c a4p3Client2.c a4p3Client3.c
	g++ a4p1Server.c -o a4p1Server -lpthread -l sqlite3
	g++ a4p2Server.c -o a4p2Server -lpthread -l sqlite3
	g++ a4p3Server.c -o a4p3Server -lpthread -l sqlite3
	g++ a4p1Client1.c -o a4p1Client1
	g++ a4p2Client1.c -o a4p2Client1
	g++ a4p3Client1.c -o a4p3Client1
	g++ a4p2Client2.c -o a4p2Client2
	g++ a4p3Client2.c -o a4p3Client2
	g++ a4p3Client3.c -o a4p3Client3

clean:
	rm a4p1Server a4p2Server a4p3Server a4p1Client1 a4p2Client1 a4p3Client3 a4p2Client2 a4p3Client2 a4p3Client3
cleanlog:
	rm a4p1ServerLog.txt a4p2ServerLog.txt a4p3ServerLog.txt a4p1Client1Log.txt a4p2Client1Log.txt a4p3Client1Log.txt
	rm a4p2Client2Log.txt a4p3Client2Log.txt a4p3Client3Log.txt



