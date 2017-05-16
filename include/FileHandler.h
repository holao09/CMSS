#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "main.h"
class FileHandler {
public:
	FileHandler();
	virtual ~FileHandler();

	int ReadFileToBuffer(char * inputFile, char * buffer, int buffersize);
	unsigned long GetFileSize(char * inputFile);
	void WriteBufferToFile(char * buffer, char * ouputFile);
	void HashingFile(char * filename, unsigned char * outputBuffer);
	void FileListing(const char * folder,file_hashing_t * list_file);
	bool FileExist(char * filename);
	static void printHash(unsigned char *hash);
	static char * HashToString(unsigned char *hash);
	bool HashStrMatch(unsigned char * hash1, unsigned char * hash2);
	bool IsHashStrEmpty(unsigned char * hash);
	void UpdateTimeInfo(file_hashing_t * file);
	int CheckFileInList(const char * filename,file_hashing_t * filelist);
protected:

private:
	int isDirectory(const char *path);
	int m_counter;
	int NumberOfFile;
	static void sha256(char * filename, unsigned char *outputBuffer);

};

#endif // FILEHANDLER_H
