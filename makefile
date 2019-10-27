.SUFFIXES: .c

all:
	gcc -c lib/convo_networking.c -o o_files/convo_networking.o
	gcc -c lib/file_functions.c -o o_files/file_functions.o
	gcc -c lib/util.c -o o_files/util.o 
	gcc -Ilib -c client/ftp_client.c -o o_files/ftp_client.o
	gcc -Ilib -c server/ftp_server.c -o o_files/ftp_server.o
	gcc o_files/convo_networking.o o_files/file_functions.o o_files/util.o o_files/ftp_client.o -o client/ftp_client
	gcc o_files/convo_networking.o o_files/file_functions.o o_files/util.o o_files/ftp_server.o -o server/ftp_server -lpthread

clean:
	/bin/rm -f *.o *~ *.dat core server/ftp_server client/ftp_client o_files/*