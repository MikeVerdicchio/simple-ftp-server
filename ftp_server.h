/* Mike Verdicchio
 * Simple FTP Server
 * ftp_server.cpp
 *
 * This program is an implementation of an
 * FTP Server to ist, store, and retrieve files
 * from the local file system. It uses the current
 * working directory of the ftp_server program as the
 * file system directory.
 */

#define BUFFER_SIZE     100
#define FILE_SIZE       4096
#define RESPONSE_150    "150 File status okay; about to open data connection.\n"
#define RESPONSE_200    "200 Command successful.\n"
#define RESPONSE_202    "202 Command not implemented, superfluous at this site.\n"
#define RESPONSE_220    "220 Successfully connected to Mike Verdicchio's FTP server.\n"
#define RESPONSE_221    "221 Service closing control connection.\n"
#define RESPONSE_226    "226 Closing data connection. Requested file action successful.\n"
#define RESPONSE_425    "425 Can't open data connection.\n"
#define RESPONSE_450    "450 Requested file action not taken.\n"
#define RESPONSE_451    "451 Requested action aborted: local error in processing.\n"
#define RESPONSE_504    "504 Command not implemented for that parameter.\n"

int CreateSocketOnLocalhost(sockaddr_in &sa, int port, bool control);
bool binary = false;				// Flag for whether binary TYPE is set