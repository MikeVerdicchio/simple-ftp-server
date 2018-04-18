# Mike Verdicchio
# Makefile

CXX = g++
CXXFLAGS = -g -w
OFILES = ftp_server.o
.SUFFIXES: .o .cpp

main:	$(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) -o ftp_server

clean:
	/bin/rm -f *.o *~
	/bin/rm -f ftp_server

my_ftpd.o: ftp_server.cpp ftp_server.h