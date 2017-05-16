/*
 C socket server example, handles multiple clients using threads
 */

#include "TCPHandler.h"
#include "ConfigHandler.h"
#include "FileHandler.h"
#include <FileTCPExchange.h>

file_hashing_t file_list[100];
bool file_list_lock = false;


server_config_t server_config;




int main(int argc, char *argv[]) {
	Log(main_server);
	int client_sock;
	struct sockaddr_in clientx;
	socklen_t clilen = sizeof(clientx);
	int threadID;
	log4cxx::PropertyConfigurator::configure("../Log4cxxConfig.cfg");
	ConfigHandler ConfigFromFile;
	ConfigFromFile.ReadConfig("Server.conf");
	server_config.ip_addr 	= 	ConfigFromFile.ip_addr();
	server_config.port 		=	ConfigFromFile.port();
	server_config.max_clients	= ConfigFromFile.max_clients();
	ConfigFromFile.ReadUserPasswd(&(server_config.userpass_list),"userpass");
	LOG_MSG(Logger(main_server),"This is logging of Server\n");
	LOG_MSG(Logger(main_server),"IP 		: " << server_config.ip_addr);
	LOG_MSG(Logger(main_server),"PORT 		: " << server_config.port);
	LOG_MSG(Logger(main_server),"MAXCLIENTS	: " << server_config.max_clients);
	
	FileHandler FileControl;
	FileControl.FileListing(DATA_FOLDER,file_list);
	TCPHandler TCPControl;
	server_config.main_socket = TCPControl.CreateServer(ConfigFromFile.ip_addr(),ConfigFromFile.port());
	int current_connections = 0;
	while (1)
	{
		while(current_connections <= server_config.max_clients)
		{
			client_sock = accept(server_config.main_socket, (struct sockaddr *)&clientx, &clilen);
			if (client_sock < 0)
			{
				perror("accept failed");
				return (1);
			}
			else
			{
				threadID = 0; // re-count threadID
				while ((server_config.client_socket[threadID].status == CONNECTED) )
				{
					threadID++;
				} //find free socket

				server_config.client_socket[threadID].socket = client_sock;
				server_config.client_socket[threadID].FileListOnServer= file_list;
				server_config.client_socket[threadID].FileListOnServerLock = &file_list_lock;
				server_config.client_socket[threadID].userpass_list = &(server_config.userpass_list);
				puts("Connection accepted");
				server_config.client_socket[threadID].threadId = threadID;
				if( pthread_create( &server_config.client_socket[threadID].thrd, NULL, FileTCPExchange::server_conn_handler, (void*) &server_config.client_socket[threadID]) < 0)
				{
					LOG_ERROR(Logger(main_server),"could not create thread");
					return (1);
				}
				else
				{
					pthread_detach(server_config.client_socket[threadID].thrd);
				}
				//N = current_connections(&server_config);
				LOG_MSG(Logger(main_server),"Thread " << server_config.client_socket[threadID].threadId << " assgined to handle socket " << server_config.client_socket[threadID].socket);
				puts("Handler assigned");
			}
		}
		//char * message = (char *)"Connection is full, please reconnect later\n\r";
		//write(client_sock, message, strlen(message));
		//close(client_sock);
	};
	return (0);
}



