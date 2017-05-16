/*
 * FileTCPExchange.h
 *
 *  Created on: May 11, 2017
 *      Author: Admin
 */

#ifndef FILETCPEXCHANGE_H_
#define FILETCPEXCHANGE_H_
#include "FileHandler.h"
#include "TCPHandler.h"
class FileTCPExchange {
public:
	FileTCPExchange();
	virtual ~FileTCPExchange();
#ifdef SERVER
	static void * server_conn_handler(void *client_socket);
#endif
#ifdef CLIENT
	static void * client_conn_handler(void *client_socket);
	static ssize_t getpasswd (unsigned char *pw, int mask);
#endif
	static int SendFile(char * filename, int sock);
	static login_result_t CheckUserPass(char * user, char * pass, userpass_t * userpass_list);
	static int SendFileList(file_hashing_t * filelist, int filelistsize, int sock);
	static int RecvFileList(file_hashing_t * filelist, int sock);
	static int RecvFile(char * filename, int sock);
	static bool FileListHasUpdate(file_hashing_t * file_list1, file_hashing_t * file_list2, int listsize);

};

#endif /* FILETCPEXCHANGE_H_ */
