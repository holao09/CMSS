#include "ConfigHandler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>     /* atoi */
#define SPACE	" "
#define COLON 	":"
#define IP_ADDR 	"IP_ADDR"
#define PORT		"PORT"
#define MAXCLIENTS	"MAXCLIENTS"
#define LOGFILE		"LOG"
ConfigHandler::ConfigHandler() {
	//ctor
	strcpy(config.ip_addr, "127.0.0.1");
	config.port = 3456;
#ifdef SERVER
	config.max_clients = MAXIMUM_CLIENTS;
#endif
}

ConfigHandler::~ConfigHandler() {
	//dtor
}
void ConfigHandler::ReadConfig(const char * config_file) {
	Log(ReadConfig);
	FILE *fp = fopen(config_file, "r");
	char buf[0x1000];
	char pattern[100];
	if (fp == NULL) {
		LOG_ERROR(Logger(ReadConfig),"Open file "<<config_file << " error !");
	} else {
		while (fgets(buf, sizeof(buf), fp) != NULL) {

			if (buf[0] == '#')
				continue; //# is symbol of comment
			else {
				char *pos;
				while (((pos = strchr(buf, '\n')) != NULL)
						|| ((pos = strchr(buf, '\r')) != NULL))
					*pos = '\0';
				char * first_space = strstr(buf, SPACE);
				memset(pattern, 0, sizeof(pattern));
				strncpy(pattern, buf, first_space - buf);
				if (strcmp(IP_ADDR, pattern) == 0) {
					strcpy(config.ip_addr, first_space + 1);
				} else if (strcmp(PORT, pattern) == 0)
					config.port = atoi(first_space + 1);
#ifdef SERVER
				else
				if (strcmp(MAXCLIENTS,pattern)==0)
				config.max_clients = atoi(first_space+1);
#endif
			}
		}
	}

}

void ConfigHandler::ReadUserPasswd(userpass_t * userpasslist, const char * userpassfile)
{
	Log(ReadUserPasswd);
	char buf[1000];
	int i = 0;
	FILE *fp = fopen (userpassfile,"r");
	if (fp == NULL) {
			LOG_ERROR(Logger(ReadUserPasswd),"Open file "<<userpassfile<<" error !");
		} else {
			while (fgets(buf, sizeof(buf), fp) != NULL) {

				if (buf[0] == '#')
					continue; //# is symbol of comment
				else
				{
					char *pos;
					while (((pos = strchr(buf, '\n')) != NULL) || ((pos = strchr(buf, '\r')) != NULL))
						*pos = '\0';
					char * seperate = strstr(buf, COLON);
					memset (userpasslist->user[i],0,USERNAME_LENGTH);
					memset (userpasslist->pass[i],0,PASSWORD_LENGTH);
					strncpy(userpasslist->user[i], buf, seperate - buf);
					strcpy (userpasslist->pass[i],seperate+1);
					i++;
				}
				userpasslist->NoUsers = i;
			}
		}
}
