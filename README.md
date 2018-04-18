Simple FTP Server
=================
This repository contains the source code for a simple FTP server. This was an assignment for Operating Systems (CS 4414) at the University of Virginia.

**Note: If you are a current student in, or are planning to take, CS 4414, it is an Honor Violation to view this source code.**



Objective
---------
The objective of the assignment was to become familiar with remote procedure calls (RPC), remote file systems, FTP, and Berkeley sockets. The assignment required implementing an FTP server that could list files on the remote file system, store a file on the server from the client file system, and retrieve a file from the server to the client. The server used binary as opposed to ASCII for data transfer. FTP uses two TCP connections: (1) a control connection for the client to send commands and the server to reply with response codes; and (2) a separate data connection for which data is exchanged through the LIST, RETR, and STOR commands.

The implemented server was to have one command-line argument for the port on which to listen and accept incoming control connections from clients. Typically, a port number greater than 1024 would be used, since the OS reserves the lower ports.



Usage
-----
```
make
./ftp_server <port>
```

Once running, you can test the implementation by opening another shell window and connecting to localhost (via FTP) on the port you used to run the server.