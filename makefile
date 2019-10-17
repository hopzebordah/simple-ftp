.SUFFIXES: .c

all:
	gcc client/ftp_client.c -o client/ftp_client
	gcc server/ftp_server.c -o server/ftp_server
	gcc tests/dynamic_input.c -o tests/dynamic_input

clean:
	/bin/rm -f *.o *~ *.dat core server/ftp_server client/ftp_client