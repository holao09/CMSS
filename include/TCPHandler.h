#ifndef TCPHANDLER_H
#define TCPHANDLER_H
#include "main.h"

#define BUF_SIZE		100u
#define PATTERN_SIZE		8u
#define LOGIN			"Login  :"
#define MSG_SIZE		100u
#define REQ_FILELIST		"REQFLIST"
#define PUS_FILELIST		"PUSFLIST"
#define REQ_FILE	 	"REQFILE:"
#define PUS_FILE		"PUSFILE:"
#define RES_MSG			"OK"
#define SOCKET_SIZE		1000u
#define SUCCESS_MSG		"Success!"
#define FAILED_MSG		"Failed  "


class TCPHandler {
public:
	typedef struct _block_t 
	{
		unsigned int datasize;
		char data[BUF_SIZE];
	} block_t;
	TCPHandler();
	virtual ~TCPHandler();
	int SendReq(char * msg);
	int RecvReq(char * msg);

#ifdef SERVER
	int CreateServer(const char ip_addr[16], const int port);
	int DestroyServer(int fd);
#endif
#ifdef CLIENT
	int CreateClient(const char ip_addr[16], const int port);
	int DestroyClient(int fd);
#endif
	static int SendMsg(int sock, const char * msg, ...);
	static int RecvMsg(int sock, unsigned char * msg, int msgsize);
	static int SendData(int sock, block_t * data);
	static int RecvData(int sock, block_t * data);
protected:

private:


};

#ifdef CLIENT

#endif

#endif // TCPHANDLER_H
