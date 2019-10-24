.SUFFIXES: .c

all:
	gcc client/ftp_client.c -o client/ftp_client 
	gcc server/ftp_server.c -o server/ftp_server -lpthread
	gcc tests/dynamic_input.c -o tests/dynamic_input 
	gcc tests/getline_input.c -o tests/getline_input 
	gcc tests/pthread_multithreading.c -o tests/pthread_test -lpthread

clean:
	/bin/rm -f *.o *~ *.dat core server/ftp_server client/ftp_client