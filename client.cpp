#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include "TCPHandler.h"
#include "FileHandler.h"
#include "ConfigHandler.h"
#include "FileTCPExchange.h"
#include <sys/stat.h>
#include <utime.h>
file_hashing_t file_list_server[MAXFILES];
file_hashing_t file_list_client[MAXFILES];
file_hashing_t file_list_update[MAXFILES];
bool file_list_server_lock = false;
bool file_list_client_lock = false;
bool ClientLoginStatus = false;

client_config_t client_config;
int main(int argc, char *argv[]) {
	int N;
	FileHandler file;
	TCPHandler tcp;
	ConfigHandler ConfigFromFile;
	Log(main_client);
	log4cxx::PropertyConfigurator::configure("../Log4cxxConfig.cfg");
	ConfigFromFile.ReadConfig("Test.conf");
	client_config.ip_addr	=	ConfigFromFile.ip_addr();
	client_config.port 		=	ConfigFromFile.port();
	LOG_MSG(Logger(main_client),"This is logging of Client\n");
	LOG_MSG(Logger(main_client),"IP 		: " << client_config.ip_addr);
	LOG_MSG(Logger(main_client),"PORT 		: " << client_config.port);

#if defined(_WIN32)
	_mkdir(DATA_FOLDER);
#else
	mkdir(DATA_FOLDER, 0755);
#endif
	file.FileListing(DATA_FOLDER,file_list_client);
	client_config.conn_socket.socket = tcp.CreateClient(client_config.ip_addr,client_config.port);
	client_config.conn_socket.FileListOnServer = file_list_server;
	client_config.conn_socket.FileListOnServerLock =&file_list_server_lock;
	client_config.conn_socket.FileListOnClient = file_list_client;
	client_config.conn_socket.FileListOnClientLock = &file_list_client_lock;
	client_config.conn_socket.LoginStatus = &ClientLoginStatus;

	if (pthread_create(&client_config.conn_socket.thrd, NULL,
			FileTCPExchange::client_conn_handler, (void*) &client_config.conn_socket) < 0)
	{
		perror("could not create thread");
		return (1);
	}
	else
	{
		pthread_detach(client_config.conn_socket.thrd);
		LOG_MSG(Logger(main_client),"thread detached!\n");
	}

	LOG_MSG(Logger(main_client),"Client Thread created!\n");
	while (!ClientLoginStatus){}

	while (1) {
		int i = 0;
		int index = 0;
		while (file_list_client_lock == true)	{}
		file_list_client_lock = true;
		file.FileListing(DATA_FOLDER,file_list_update);
		//Compare 2 file-lists on client, update to client list.
		while (file_list_client[i].filename[0] != '\0') {
			index = file.CheckFileInList(file_list_client[i].filename,
					file_list_update);
			if (((index == -1)
					|| (file.FileExist(file_list_client[i].filename)
							== false))) //file deleted
			{
				if (file_list_client[i].status != DELETED) {
					printf("File %s set to be DELETED\n",
							file_list_client[i].filename);
					file_list_client[i].status = DELETED;
					file_list_client[i].ModifiedTime = time(NULL);
				} else {
				}
			} else //file not deleted, check if it's modified or not
			{
				while (file_list_client[i].status == UPDATING) {
				} //wait for file update.
				file_list_client[i].status = UPDATING;
				LOG_MSG(Logger(main_client),"File name:" << file_list_update[index].filename);
				file.HashingFile(file_list_update[index].filename,file_list_update[index].hash_num);
				LOG_MSG(Logger(main_client),"Hash:" << file.HashToString(file_list_update[index].hash_num));
				if (file.IsHashStrEmpty(file_list_client[i].hash_num)) //file on client list not hashed
					memcpy(file_list_client[i].hash_num,
							file_list_update[index].hash_num,
							sizeof(file_list_client[i].hash_num));
				if (!file.HashStrMatch(file_list_update[index].hash_num,
						file_list_client[i].hash_num)) //hash not match
						{
					file_list_client[i].status = MODIFIED;
					memcpy(file_list_client[i].hash_num,
							file_list_update[index].hash_num,
							sizeof(file_list_client[i].hash_num)); //update new hash
					LOG_MSG(Logger(main_client),"File " << file_list_client[i].filename << "just modified !\n");
				} else
					file_list_client[i].status = UPDATED;
				file.UpdateTimeInfo(file_list_client + i);
			}
			i++;
		}
		N = i;
		i = 0;

		while (file_list_update[i].filename[0] != '\0') {
			index = file.CheckFileInList(file_list_update[i].filename,
					file_list_client);
			if ((index == -1)
					&& (file.FileExist(file_list_update[i].filename))) //file created new
					{
				strcpy(file_list_client[N].filename,
						file_list_update[i].filename);
				file_list_client[N].status = UPDATING;
				file.HashingFile(file_list_client[N].filename,
						file_list_client[N].hash_num);
				file_list_client[N].status = UPDATED;
				file.UpdateTimeInfo(file_list_client + N);
				N++;
			}
			i++;
		}
		// Finish compare 2 file-lists on client
		LOG_MSG(Logger(main_client),"Waiting for thread get server file list");
		while (file_list_server_lock)
		{
		    //printf("Deadlock!\n");
		} //waith for thread get file list from server
		file_list_server_lock = true;
		// Compare client vs Server
		i = 0;
		while (file_list_server[i].filename[0] != '\0') {
			index = file.CheckFileInList(file_list_server[i].filename,
					file_list_client);
			if (index == -1) //File not in client list
			{
				strcpy(file_list_client[N].filename,
						file_list_server[i].filename);
				if (file.FileExist(file_list_server[i].filename)) 
				{
					file_list_client[N].status = UPDATING;
					file.HashingFile(file_list_client[N].filename,
							file_list_client[N].hash_num);
					if (!file.HashStrMatch(
							file_list_server[i].hash_num,
							file_list_client[N].hash_num)) //hash not match
						file_list_client[N].status = OUTDATED;
					else
						file_list_client[N].status = UPDATED;
					file.UpdateTimeInfo(file_list_client + N);
				} else {
					file_list_client[N].status = OUTDATED;
				}
				file_list_client[N].CreatedTime =
						file_list_server[i].CreatedTime;
				file_list_client[N].ModifiedTime =
						file_list_server[i].ModifiedTime;
				N++;
			} else //file in client list, check if server file status
			{
				if ((file_list_server[i].status == DELETED)
						&& (file.FileExist(file_list_server[i].filename))) //file deleted on server
						{
					file.UpdateTimeInfo(file_list_client + index);
					LOG_MSG(Logger(main_client),"File " << file_list_server[i].filename);
					LOG_MSG(Logger(main_client),"ModifiedTime on Server : " << file_list_server[i].ModifiedTime);
					LOG_MSG(Logger(main_client),"ModifiedTime on Client : " << file_list_client[index].ModifiedTime);
					if (file_list_client[index].ModifiedTime
							<= file_list_server[i].ModifiedTime) {
						unlink(file_list_server[i].filename);
						file_list_client[index].status = DELETED;
						file_list_client[index].ModifiedTime = time(NULL);
						LOG_MSG(Logger(main_client),"File " << file_list_server[i].filename << " deleted by sync at " <<file_list_client[index].ModifiedTime); 
					} else {
						file_list_client[index].status = MODIFIED;
						file.UpdateTimeInfo(file_list_client + index);
					}
				} else 
				if ((file_list_server[i].status != DELETED)
						&& (!file.FileExist(file_list_server[i].filename))) //file deleted on client
				{
						if ((file_list_client[index].status == DELETED) &&(file_list_client[index].ModifiedTime < file_list_server[i].ModifiedTime)) 
						{
							LOG_MSG(Logger(main_client),"File "<< file_list_client[index].filename << " set to be OUTDATED");
							LOG_MSG(Logger(main_client),"ModifiedTime on Client: " << file_list_client[index].ModifiedTime);
							LOG_MSG(Logger(main_client),"ModifiedTime on Server: " << file_list_server[i].ModifiedTime);
							file_list_client[index].status = OUTDATED;
						}
				}
				else
				if (!file.HashStrMatch(file_list_server[i].hash_num, file_list_client[index].hash_num) && file_list_client[index].status != MODIFIED) //hash not match
				{
					file_list_client[index].status = OUTDATED;
					LOG_MSG(Logger(main_client),"File "<< file_list_client[index].filename << " is outdated");
				}
			}
			i++;
		}

		i = 0;
		while (file_list_client[i].filename[0] != '\0') {
			index = file.CheckFileInList(file_list_client[i].filename,
					file_list_server);
			if ((index == -1)
					&& (file.FileExist(file_list_client[i].filename))) //file created new on client
					{
				file_list_client[i].status = MODIFIED;
			}
			i++;
		}
		file_list_server_lock = false;
		file_list_client_lock = false;
		while (file_list_client_lock == false) {
		}

	};
	return (0);
}
