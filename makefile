.SUFFIXES: .c

all:
	gcc client/ftp_client.c -o client/ftp_client
	gcc server/ftp_server.c -o server/ftp_server

clean:
	/bin/rm -f *.o *~ *.dat core server/ftp_server client/ftp_client