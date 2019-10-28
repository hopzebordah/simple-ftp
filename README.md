# ALEXANDER PETERS hw03 FOR 565 - COMPUTER NETWORKS WITH KATANGUR

my program uses a send - ack structure to ensure that messages are sent one by one.
sometimes, if the messages are skipped or unable to be delivered, then the program may
be stuck waiting for an ACK while the other end is also waiting for data. 

the way the program is designed, sometimes it will mess up. the solution is just to restart
the client or server program. 

to build the project, navigate to the root directory of the project and run: 
    `make`
        - this will compile all code and place executables in the server/ and client/ directories
other make functions:
    `make clean`
        - this will delete all .o and executable files 

the project structure is as follows:

/ 
    client/
        files/
            // dummy test files for transferring are located here
        ftp_client.c
    lib/
        convo_networking.c
        convo_networking.h
        file_functions.c
        file_functions.h
        util.c
        util.h
    server/
        files/
            // dummy test files for transferring are located here
        ftp_server.c
    makefile
    README.md

the lib/ directory contains lots of methods that i found repetitive and abstracted away 
in order to make the code easier to write. 

the convo_networking files contain all networking code
the file_functions files contain all the code that deals with the filesystem
the util files contain other things i didn't feel like writing over and over