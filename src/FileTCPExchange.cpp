/*
 * FileTCPExchange.cpp
 *
 *  Created on: May 11, 2017
 *      Author: Admin
 */

#include <FileTCPExchange.h>
#include <termios.h>
#include <sys/stat.h>
#include <libgen.h>
FileTCPExchange::FileTCPExchange() {
	// TODO Auto-generated constructor stub

}

FileTCPExchange::~FileTCPExchange() {
	// TODO Auto-generated destructor stub
}

#ifdef SERVER
void * FileTCPExchange::server_conn_handler(void *client_socket)
{
	Log(server_conn_handler);
	TCPHandler tcp;
	FileHandler file;
	//Get the socket descriptor
	thread_conn_param_t * client_param = (thread_conn_param_t *) client_socket;
	int sock = client_param->socket;
	int read_size;
	char message[MSG_SIZE];
	char client_message[MSG_SIZE];
	char pattern[PATTERN_SIZE+1];
	char filename[FNAME_SIZE];
	char user[255];
	unsigned char pass[255];
	int index;
	int i;
	struct utimbuf FileTime;
	struct stat FileStat;
	//Send some messages to the client
	memset(message,0,MSG_SIZE);
	sprintf(message,"This is thread %d with socket %d\n",client_param->threadId,client_param->socket);
	write(sock, message, strlen(message));
	client_param->status = CONNECTED;
	//Receive a message from client
	while((read_size = recv(sock, client_message, MSG_SIZE, 0)) > 0)
	{
	      if(read_size == -1)
		{
			perror("recv failed");
			break;
		}
		LOG_MSG(Logger(server_conn_handler),"read_size " <<read_size);
		memset(pattern,0,PATTERN_SIZE+1);
		strncpy(pattern,client_message,PATTERN_SIZE);
		LOG_MSG(Logger(server_conn_handler),"Client_message " << client_message << "from socket " << sock);
		LOG_MSG(Logger(server_conn_handler),"Pattern ="<<pattern);
		if (strcmp(pattern,REQ_FILELIST)==0)
		{

			while (*(client_param->FileListOnServerLock)) {}
			*(client_param->FileListOnServerLock) = true;
			int N = 0;
			while (client_param->FileListOnServer[N].filename[0]!='\0')
			{
				N++;
			}
			SendFileList(client_param->FileListOnServer,N,sock);
			*(client_param->FileListOnServerLock) = false;
			for (i = 0; i<N; i++)
			{
				LOG_MSG (Logger(server_conn_handler),"File "<< i << " : " << client_param->FileListOnServer[i].filename);
			}
		}
		else if (strcmp(pattern,PUS_FILELIST)==0)
		{
			while (*(client_param->FileListOnServerLock)) {}
			*(client_param->FileListOnServerLock) = true;

			//SendRes(sock);
			tcp.SendMsg(sock,RES_MSG);
			RecvFileList (client_param->FileListOnServer,sock);

			*(client_param->FileListOnServerLock) = false;
			i = 0;
			while (client_param->FileListOnServer[i].filename[0]!='\0')
			{
				if (stat(client_param->FileListOnServer[i].filename, &FileStat) < 0)
				{
					printf("ERROR 204");
					perror(client_param->FileListOnServer[i].filename);
					return (0);
				}
				if ((client_param->FileListOnServer[i].CreatedTime > FileStat.st_atime) || (client_param->FileListOnServer[i].CreatedTime == 0))
					client_param->FileListOnServer[i].CreatedTime = FileStat.st_atime;
				if ((client_param->FileListOnServer[i].ModifiedTime < FileStat.st_ctime) || (client_param->FileListOnServer[i].ModifiedTime == 0))
					client_param->FileListOnServer[i].ModifiedTime = FileStat.st_ctime;
				FileTime.actime = client_param->FileListOnServer[i].CreatedTime;
				FileTime.modtime = client_param->FileListOnServer[i].ModifiedTime;
				utime(client_param->FileListOnServer[i].filename, &FileTime);
				i++;
			}
		}
		else if (strcmp(pattern,PUS_FILE)==0)
		{
			memset(filename,0,sizeof(filename));
			strncpy(filename,client_message+PATTERN_SIZE,strlen(client_message)-PATTERN_SIZE);
			index = file.CheckFileInList(filename,client_param->FileListOnServer);
			if (index != -1) //file on server
			{
				while (client_param->FileListOnServer[index].status == UPDATING) {}
				client_param->FileListOnServer[index].status = UPDATING;
			}
			//SendRes(sock);
			tcp.SendMsg(sock,RES_MSG);
			RecvFile(filename,sock);
			if (index != -1) client_param->FileListOnServer[index].status = UPDATED;
		}
		else if (strcmp(pattern,REQ_FILE)==0)
		{
			memset(filename,0,sizeof(filename));
			strncpy(filename,client_message+PATTERN_SIZE,strlen(client_message)-PATTERN_SIZE);
			index = file.CheckFileInList(filename,client_param->FileListOnServer);
			if (index != -1) //file on server
			{
				while (client_param->FileListOnServer[index].status == UPDATING) {}
				SendFile(filename,sock);
			}
			else
				write(sock, FILE_NOT_FOUND,PATTERN_SIZE);
		}
		else if (strcmp(pattern,LOGIN)==0)
		{
			LOG_MSG(Logger(server_conn_handler),"In case LOGIN");
			strcpy(user,client_message+strlen(pattern));
			LOG_MSG(Logger(server_conn_handler),"User = " << user<<":");
			//SendRes(sock);
			tcp.SendMsg(sock,RES_MSG);
			//RecvPasswd(pass,sock);
			tcp.RecvMsg(sock, pass,sizeof(pass));
			char hexpass[65];
			memset(hexpass,0,65);
			strcpy (hexpass,file.HashToString(pass));
			LOG_MSG(Logger(server_conn_handler),"Hexpass :"<<hexpass);
			login_result_t login_result = CheckUserPass(user,hexpass,client_param->userpass_list);
			if (login_result == Success)
			{
				printf("Login success!\n");
				send(sock,SUCCESS_MSG,sizeof(SUCCESS_MSG),MSG_WAITALL);
			}
			else
			{
				printf("Login failed!\n");
				send(sock,FAILED_MSG,sizeof(FAILED_MSG),MSG_WAITALL);
			}

		}
		else if (strcmp(client_message,"Hello")==0)
		{
			strcpy(message,"WTF?");
			//Send the message back to client
			write(sock, message, strlen(message));
		}
		else
		{
			//write(sock, client_message, strlen(client_message));
		}
		memset(client_message,0,sizeof(client_message));

	}
	puts("Client disconnected");
	client_param->status = DISCONNECTED;
	close (sock);
	pthread_exit(NULL);
	return (client_socket);
}
#endif

#ifdef CLIENT
void * FileTCPExchange::client_conn_handler(void *client_socket)
{
	Log(client_conn_handler);
	thread_conn_param_t * client_param = (thread_conn_param_t *) client_socket;
	int sock = client_param->socket;
	char server_message[MSG_SIZE];
	TCPHandler tcp;
	FileHandler file;
	unsigned char buffer[BUF_SIZE];
	memset(server_message,0,MSG_SIZE);
	int read_size = recv(sock, server_message, MSG_SIZE, 0);
	LOG_MSG(Logger(client_conn_handler), server_message);
	int index;
	int N;
	int i;
	struct utimbuf FileTime;
	int sleep_count = 0;
	bool LoginStatus = false;
	while (read_size != 0)
	{
		unsigned char username[255];
		unsigned char password[255];
		unsigned char hashpass[32];
		while (!LoginStatus)
		{
			printf("Username :");
			scanf("%s", username);
			getchar(); //get enter key
			LOG_MSG(Logger(client_conn_handler),"username :"<<username);
			printf("Password :");
			getpasswd (password, '*');
			LOG_MSG(Logger(client_conn_handler),"password :"<<password);
			SHA256_CTX sha256;
			SHA256_Init(&sha256);
			SHA256_Update(&sha256, password, strlen((char*) password));
			SHA256_Final(hashpass, &sha256);
			LOG_MSG(Logger(client_conn_handler),"hashpass :"<<file.HashToString(hashpass));

			tcp.SendMsg(sock, LOGIN, username);
			tcp.RecvMsg(sock, buffer, BUF_SIZE);
			send(sock,hashpass,sizeof(hashpass),MSG_WAITALL);/*send password to client*/
			tcp.RecvMsg(sock, buffer, BUF_SIZE);
			if(strcmp((char *)buffer,SUCCESS_MSG)==0)
			  {
			    LoginStatus = true;
			    LOG_MSG(Logger(client_conn_handler),"Login Success!");
			  }
			else
			  LOG_MSG(Logger(client_conn_handler),"Login failed!");

		}
		*(client_param->LoginStatus) = LoginStatus;
		while (*(client_param->FileListOnServerLock) == true){} //wait client finish use FileListOnServer
		*(client_param->FileListOnServerLock) = true;
		tcp.SendMsg(sock, REQ_FILELIST);
		RecvFileList(client_param->FileListOnServer, sock);
		*(client_param->FileListOnServerLock) = false;
		LOG_MSG(Logger(client_conn_handler),"Wait in thread time : "<< sleep_count);
		while (*(client_param->FileListOnClientLock)){} //wait for loop on main() finish
		*(client_param->FileListOnClientLock) = true;
		i = 0;

		while (client_param->FileListOnClient[i].filename[0] != '\0')
		{
			switch (client_param->FileListOnClient[i].status)
			{
				case MODIFIED:
					LOG_ERROR(Logger(client_conn_handler),"File " << client_param->FileListOnClient[i].filename << " modified and will put to server");
					////PUS_FILE if file not in server
					client_param->FileListOnClient[i].status = UPDATING;
					tcp.SendMsg(sock, PUS_FILE,
					        client_param->FileListOnClient[i].filename);
					tcp.RecvMsg(sock, buffer, BUF_SIZE);
					SendFile(client_param->FileListOnClient[i].filename, sock);
					client_param->FileListOnClient[i].status = UPDATED;
					break;
				case OUTDATED:
					//LOG_MSG("%s : File %s outdated and will get from server\n",
					//		curr_time(),
					//		client_param->FileListOnClient[i].filename);
					tcp.SendMsg(sock, REQ_FILE,
					        client_param->FileListOnClient[i].filename);
					RecvFile(client_param->FileListOnClient[i].filename, sock);
					client_param->FileListOnClient[i].status = UPDATED;
					index = file.CheckFileInList(
					            client_param->FileListOnClient[i].filename,
					            client_param->FileListOnServer);
					FileTime.actime =
					    client_param->FileListOnServer[index].CreatedTime;
					FileTime.modtime =
					    client_param->FileListOnServer[index].ModifiedTime;
					utime(client_param->FileListOnClient[i].filename, &FileTime);
					break;
				default:
					break;
			}
			i++;
		}
		N = i;
		if (FileListHasUpdate(client_param->FileListOnClient,
		                       client_param->FileListOnServer, N))
		{
			LOG_MSG(Logger(client_conn_handler),"Have some update on file list");
			tcp.SendMsg(sock, PUS_FILELIST);
			tcp.RecvMsg(sock, buffer, BUF_SIZE);
			SendFileList(client_param->FileListOnClient, N, sock);
		}
		sleep(5);
		LOG_MSG(Logger(client_conn_handler),"Sleep 5s on thread");
		*(client_param->FileListOnClientLock) = false;
		sleep_count++;
	}
	close(sock);
	pthread_exit(NULL);
	printf("thread end !!!\n");
	return client_socket;
}
#endif

int FileTCPExchange::SendFile(char * filename, int sock) {
	Log(SendFile);
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		LOG_ERROR(Logger(SendFile),"File "<<filename<<" open error !");
		return (-1);
	}
	char res_msg[sizeof(RES_MSG)];
	TCPHandler::block_t Block;
	Block.datasize = BUF_SIZE;
	printf("Transferring ...%s\n", filename);
	while (Block.datasize == BUF_SIZE) {
		memset(Block.data, 0, BUF_SIZE);
		Block.datasize = fread(Block.data, 1, BUF_SIZE, fp);
		printf("datasize = %d\n", Block.datasize);
		recv(sock, &res_msg, sizeof(RES_MSG), MSG_WAITALL);
		//write(sock,&Block,sizeof(Block));
		send(sock, &Block, sizeof(Block), MSG_WAITALL);
	}
	fclose(fp);
	return (0);
}

login_result_t FileTCPExchange::CheckUserPass(char * user, char * pass, userpass_t * userpass_list)
{
	Log(CheckUserPass);
	login_result_t login_result = Failed;
	int i;
	for( i=0;i<userpass_list->NoUsers;i++)
	{
		LOG_MSG(Logger(CheckUserPass),"User:Pass "<<userpass_list->user[i]<<":"<<userpass_list->pass[i]);
		if ((strcmp(userpass_list->user[i],user) == 0) && (memcmp(userpass_list->pass[i],pass,64) == 0))
			login_result = Success;
	}
	return (login_result);
}

int FileTCPExchange::SendFileList(file_hashing_t * filelist, int filelistsize, int sock)
{
	Log(SendFileList);
	char res_msg[sizeof(RES_MSG)];
	LOG_MSG(Logger(SendFileList),"Number of file in list : "<<filelistsize);
	char msg_N[10];
	snprintf(msg_N, 10, "%d", filelistsize);
	send(sock, msg_N, sizeof(msg_N), MSG_WAITALL);
	recv(sock, &res_msg, sizeof(RES_MSG), 0);
	for (int i = 0; i < filelistsize; i++) {
		send(sock, &filelist[i], sizeof(file_hashing_t), MSG_WAITALL);
		LOG_MSG(Logger(SendFileList),"Filename :"<<filelist[i].filename);
		LOG_MSG(Logger(SendFileList),"Status :"<<filelist[i].status);
		LOG_MSG(Logger(SendFileList),"CreateTime :"<<filelist[i].CreatedTime);
		LOG_MSG(Logger(SendFileList),"ModifiedTime :"<<filelist[i].ModifiedTime);
		recv(sock, &res_msg, sizeof(RES_MSG), 0);
	}
	return (0);
}

int FileTCPExchange::RecvFileList(file_hashing_t * filelist, int sock) {
	char msg_N[10];
	recv(sock, msg_N, sizeof(msg_N), MSG_WAITALL);
	send(sock, RES_MSG, sizeof(RES_MSG), MSG_WAITALL);
	int N = atoi(msg_N);
	for (int i = 0; i < N; i++) {
		recv(sock, &filelist[i], sizeof(file_hashing_t), MSG_WAITALL);
		send(sock, RES_MSG, sizeof(RES_MSG), MSG_WAITALL);
	}
	return (0);
}

int FileTCPExchange::RecvFile(char * filename, int sock) {

	char * folder = (char*)malloc(strlen(filename));
	strcpy(folder,filename);
	folder = dirname(folder);
	if (strcmp(folder,".") != 0)
	  {
	    mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	  }
	free (folder);
	FILE * fp = fopen(filename, "wb");

	TCPHandler::block_t Block;
	Block.datasize = BUF_SIZE;
	char msg[sizeof(RES_MSG)];
	strcpy(msg, RES_MSG);
	printf("Receiving ...%s\n", filename);
	printf("file %s\n",filename);
	while (Block.datasize == BUF_SIZE) {
		memset(Block.data, 0, BUF_SIZE);
		write(sock, msg, sizeof(RES_MSG));
		recv(sock, &Block, sizeof(TCPHandler::block_t), MSG_WAITALL);
		fwrite(Block.data, Block.datasize, 1, fp);
	}
	fclose(fp);
	return (0);
}

bool FileTCPExchange::FileListHasUpdate(file_hashing_t * file_list1, file_hashing_t * file_list2, int listsize)
{
	int index;
	int i;
	FileHandler file;
	for (i = 0; i < listsize; i++) {
		index = file.CheckFileInList(file_list1[i].filename, file_list2);
		if ((index == -1) || (file_list1[i].status != file_list2[index].status)
				|| (memcmp(file_list1[i].hash_num, file_list2[index].hash_num, 32) != 0)) {
			return (true);
		}
	}
	return (false); //not difference
}

#ifdef CLIENT
ssize_t FileTCPExchange::getpasswd (unsigned char *pw, int mask)
{
	Log(getpasswd);
    if (!pw)
    {
    	LOG_ERROR(Logger(getpasswd),"Invalid input");
    	return -1;       /* validate input   */
    }
    size_t idx = 0;         /* index, number of chars in read   */
    int c = 0;

    struct termios old_kbd_mode;    /* orig keyboard settings   */
    struct termios new_kbd_mode;

    if (tcgetattr (0, &old_kbd_mode)) { /* save orig settings   */
    	LOG_ERROR(Logger(getpasswd), "" <<__func__<<"() error: tcgetattr failed.");
        return -1;
    }   /* copy old to new */
    memcpy (&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);  /* new kbd flags */
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr (0, TCSANOW, &new_kbd_mode)) {
    	LOG_ERROR(Logger(getpasswd), ""<<__func__<<"() error: tcsetattr failed.");
        return -1;
    }

    /* read chars from fp, mask if valid char specified */
    while ((c = fgetc(stdin)) != '\n')
    {
        if (c != 127) {
            if (31 < mask && mask < 127)    /* valid ascii char */
                fputc (mask, stdout);
            pw[idx++] = c;
        }
        else if (idx > 0) {         /* handle backspace (del)   */
            if (31 < mask && mask < 127) {
                fputc (0x8, stdout);
                fputc (' ', stdout);
                fputc (0x8, stdout);
            }
            pw[--idx] = 0;
        }
    }
    pw[idx] = 0; /* null-terminate   */

    /* reset original keyboard  */
    if (tcsetattr (0, TCSANOW, &old_kbd_mode)) {
        LOG_ERROR(Logger(getpasswd), ""<<__func__<<"() error: tcsetattr failed.");
        return -1;
    }

    return idx; /* number of chars in passwd    */
}
#endif
