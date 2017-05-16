#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H
#include "main.h"


class ConfigHandler {
public:

	ConfigHandler();
	virtual ~ConfigHandler();
	void ReadUserPasswd(userpass_t * userpasslist, const char * userpassfile);
	void ReadConfig(const char * config_file);
	const char* ip_addr() {
		return (config.ip_addr);
	}
	const int port() {
		return (config.port);
	}
#ifdef SERVER
	const int max_clients() {return (config.max_clients);}
#endif
protected:

private:
	typedef struct _config_from_file {
		char ip_addr[16];
		unsigned int port;
#ifdef SERVER
		unsigned int max_clients;
#endif
	} config_file_t;
	config_file_t config;
};

#endif // CONFIGHANDLER_H
