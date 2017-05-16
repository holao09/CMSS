/*
 * main.h
 *
 *  Created on: May 11, 2017
 *      Author: Admin
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <utime.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>

#ifndef MAIN_H_
#define MAIN_H_

#define DATA_FOLDER		"data"
#define MAXFILES			1000u
#define MAXIMUM_CLIENTS		1000u
#define MAXIMUM_USERS		1000u
#define FNAME_SIZE			255u
#define USERNAME_LENGTH		255u
#define PASSWORD_LENGTH		255u
#define FILESIZE_PATTERN 	"FSIZE  :"
#define BUFFSIZE_PATTERN 	"BSIZE  :"
#define FILEPART_PATTERN 	"FPART  :"
#define FILEEOF_PATTERN	 	"FEOF   :"
#define FILE_EXISTED		"Existed!"
#define FILE_NOT_FOUND		"MSG_F404"
#define SEPERATE			":"

#define Log(x) log4cxx::LoggerPtr logger##x(log4cxx::Logger::getLogger(#x))
#define Logger(x) logger##x
#define LOG_TRACE(...) 	LOG4CXX_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...) 	LOG4CXX_DEBUG(__VA_ARGS__)
#define LOG_MSG(...) 	LOG4CXX_INFO(__VA_ARGS__)
#define LOG_WARN(...) 	LOG4CXX_WARN(__VA_ARGS__)
#define LOG_ERROR(...) 	LOG4CXX_ERROR(__VA_ARGS__)
#define LOG_FATAL(...) 	LOG4CXX_FATAL(__VA_ARGS__)

typedef enum _file_update_status {
	UPDATED, UPDATING, OUTDATED, MODIFIED, DELETED
} file_update_status_t;

typedef struct _filehash {
	char filename[255];
	file_update_status_t status;
	unsigned char hash_num[32];
	time_t CreatedTime;
	time_t ModifiedTime;
} file_hashing_t;


typedef enum _connection_status {
	DISCONNECTED, CONNECTED
} connection_status_t;

typedef struct _userpasswd
{
	int NoUsers;
	char user [MAXIMUM_USERS][USERNAME_LENGTH];
	char pass [MAXIMUM_USERS][PASSWORD_LENGTH];
}userpass_t;

typedef struct _thread_conn_param {
	int threadId;
	pthread_t thrd;
	int socket;
	connection_status_t status;
	file_hashing_t * FileListOnServer;
	bool * FileListOnServerLock;
#ifdef CLIENT
	file_hashing_t * FileListOnClient;
	bool * FileListOnClientLock;
	bool * LoginStatus;
#endif
#ifdef SERVER
	userpass_t * userpass_list;
#endif	
} thread_conn_param_t;

typedef struct _server_config
{
	const char * ip_addr;
	unsigned int port;
	int main_socket;
	int max_clients;
	userpass_t userpass_list;
	thread_conn_param_t client_socket[MAXIMUM_CLIENTS];
}server_config_t;

typedef struct _client_config
{
	const char * ip_addr;
	unsigned int port;
	thread_conn_param_t conn_socket;
}client_config_t;

typedef enum _login_result
{
	Success,
	Failed
}login_result_t;



#endif /* MAIN_H_ */
