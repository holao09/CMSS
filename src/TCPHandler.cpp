#include "TCPHandler.h"


TCPHandler::TCPHandler()
{
	//ctor
}

TCPHandler::~TCPHandler()
{
	//dtor
}

#ifdef SERVER
int TCPHandler::CreateServer(const char ip_addr[16], const int port)
{
	Log(CreateServer);
	int bsize = SOCKET_SIZE;
	struct sockaddr_in server;
	//Create socket
	int main_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (main_socket == -1)
	{
		LOG_ERROR(Logger(CreateServer),"Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_port = htons(port);

	//Bind
	setsockopt(main_socket, SOL_SOCKET, SO_SNDBUF|SO_KEEPALIVE, &bsize, sizeof(int));
	if( bind(main_socket,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return (1);
	}
	puts("bind done");

	//Listen
	listen(main_socket, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	return (main_socket);
}

#endif

#ifdef CLIENT
int TCPHandler::CreateClient(const char ip_addr[16], const int port)
{

	Log(CreateClient);
	int sock;
	struct sockaddr_in conn2server;
	int bsize = SOCKET_SIZE;

	conn2server.sin_family = AF_INET;
	conn2server.sin_port = htons(port);
	conn2server.sin_addr.s_addr = inet_addr(ip_addr);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		LOG_ERROR(Logger(CreateClient),"Could not create socket");
	}
	puts("Socket created");
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bsize,
	           sizeof(int));
	while (connect(sock,(struct sockaddr *) &conn2server, sizeof(conn2server)) != 0)
	{
		LOG_ERROR(Logger(CreateClient),"Can't connect to server");
		LOG_ERROR(Logger(CreateClient),"Retry in 5s");
		sleep(5);
	}
	return (sock);
}

#endif


int TCPHandler::SendMsg(int sock, const char * msg, ...)
{
	va_list vl;
	va_start(vl, msg);
	char req[MSG_SIZE];
	if (strcmp(msg, REQ_FILE) == 0)
	{
		char * filename = va_arg(vl, char *);
		stpcpy(req, REQ_FILE);
		strcat(req, filename);
		send(sock, req, MSG_SIZE, 0);
	}
	else if (strcmp(msg, PUS_FILE) == 0)
	{
		char * filename = va_arg(vl, char *);
		stpcpy(req, PUS_FILE);
		strcat(req, filename);
		send(sock, req, MSG_SIZE, 0);
	}
	else if (strcmp(msg, REQ_FILELIST) == 0)
	{
		stpcpy(req, REQ_FILELIST);
		send(sock, req, strlen(req), MSG_WAITALL);
	}
	else if (strcmp(msg, PUS_FILELIST) == 0)
	{
		stpcpy(req, PUS_FILELIST);
		send(sock, req, strlen(req), MSG_WAITALL);
	}
	else
	if (strcmp(msg, LOGIN) == 0)
	{
		char * username = va_arg(vl, char *);
		stpcpy(req, LOGIN);
		strcat(req, username);
		send(sock, req, MSG_SIZE, 0);
	}
	else
	if (strcmp(msg, RES_MSG) == 0)
	{
		send(sock, RES_MSG, sizeof(RES_MSG), MSG_WAITALL);
	}
	else
	{
		int msg_size = va_arg(vl, int);
		send(sock,msg, msg_size,MSG_WAITALL);
	}
	return (0);
}

int TCPHandler::RecvMsg(int sock, unsigned char * msg, int msgsize)
{
	memset (msg,0,msgsize);
	return (recv(sock, msg, msgsize, 0));
}

int TCPHandler::SendData(int sock, block_t * data)
{
	send(sock, data, sizeof(data), 0);
    return (data->datasize);
}


int TCPHandler::RecvData(int sock, block_t * data)
{
	recv(sock, data, sizeof(data), MSG_WAITALL);
    return (data->datasize);
}
