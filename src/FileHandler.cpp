#include "FileHandler.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
FileHandler::FileHandler() {
	//ctor
 	m_counter = 0;
	NumberOfFile = 0;
	/*
	FileListing((char *) FolderPath);
	for (m_counter = 0; m_counter < NumberOfFile; m_counter++) {
		UpdateTimeInfo(data + m_counter);
	} */
}

FileHandler::~FileHandler() {
	//dtor
}

int FileHandler::isDirectory(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) != 0)
		return (0);
	return S_ISDIR(statbuf.st_mode);
}

void FileHandler::FileListing(const char * folder, file_hashing_t * list_file) 
{
	DIR *dir;
	m_counter =0;
	struct dirent *entry;
	if (!(dir = opendir(folder)))
		return;
	while ((entry = readdir(dir))!= NULL)
	    {
	      char path[1024];
	      memset(path,0,sizeof(path));
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue;
		else
		  {
		      sprintf(path,"%s/%s",folder,entry->d_name);
		      if (!isDirectory(path))
		      {
			  strcpy(list_file[m_counter].filename, path);
			  m_counter++;
		      }
		      else
			{
			   FileListing(path,list_file+m_counter);
			}
		  }

	    }
	closedir(dir);
	NumberOfFile = m_counter;
}

bool FileHandler::FileExist(char * filename) {
	FILE *pFile;
	pFile = fopen(filename, "r");

	if (pFile != NULL) {
		fclose(pFile);
		return (true);
	}
	return (false);
}

int FileHandler::ReadFileToBuffer(char * inputFile, char * buffer, int buffersize) {
	Log(ReadFileToBuffer);
	FILE * pFile;
	size_t lSize;
	size_t result;
	pFile = fopen(inputFile, "rb");
	if (pFile == NULL) {
		LOG_ERROR(Logger(ReadFileToBuffer),"File "<<inputFile <<" open error");
		return (-1);
	}

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);
	memset(buffer, 0, buffersize); //Clear buffer
	if (buffer == NULL) {
		LOG_ERROR(Logger(ReadFileToBuffer),"Buffer Error !");
		return (-2);
	}

	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		LOG_ERROR(Logger(ReadFileToBuffer),"Reading error");
		return (-3);
	}

	/* the whole file is now loaded in the memory buffer. */

	// terminate
	fclose(pFile);
	return (result);
}

unsigned long FileHandler::GetFileSize(char * inputFile) {
	Log(GetFileSize);
	FILE * pFile;
	unsigned long lSize;
	pFile = fopen(inputFile, "rb");
	if (pFile == NULL) {
		LOG_ERROR(Logger(GetFileSize),"File "<<inputFile <<" open error");
		return (-1);
	}
	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	// terminate
	fclose(pFile);
	return (lSize);
}

void FileHandler::sha256(char * filename, unsigned char *outputBuffer) {
	Log(sha256);
	const int bufSize = 100;
	char buffer[bufSize];
	int bytesRead = 0;
	FILE * fp = fopen(filename, "rb");
	if (fp == NULL) {
		LOG_ERROR(Logger(sha256),"File " <<filename <<" open error !");
		return;
	}
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	while ((bytesRead = fread(buffer, 1, bufSize, fp))) {
		SHA256_Update(&sha256, buffer, bytesRead);
	}
	SHA256_Final((unsigned char *) outputBuffer, &sha256);
	fclose(fp);
}
void FileHandler::printHash(unsigned char *hash) {
	int i = 0;
	for (i = 0; i < 32; i++) {
		unsigned int a = hash[i];
		printf("%02x", a);
	}
	printf("\n");
}

char * FileHandler::HashToString(unsigned char *hash)
{
	static char str[65];
	char *p = str;
	int i = 0;
		for (i = 0; i < 32; i++) {
			unsigned int a = hash[i];
			p+=sprintf(p,"%02x", a);
		}
	str[64]='\0';
	return (str);
}

void FileHandler::HashingFile(char * filename, unsigned char * outputBuffer) {
	memset(outputBuffer, 0, 32); //Clear outputBuffer
	sha256(filename, outputBuffer);
}

bool FileHandler::HashStrMatch(unsigned char * hash1, unsigned char * hash2) {
	int i = 0;
	for (i = 0; i < 32; i++)
		if (hash1[i] != hash2[i])
			return (false);
	return (true);
}

bool FileHandler::IsHashStrEmpty(unsigned char * hash) {
	int i = 0;
	for (i = 0; i < 32; i++)
		if (hash[i] != '\0')
			return (false);
	return (true);
}

void FileHandler::UpdateTimeInfo(file_hashing_t * file) {
	struct stat FileStat;
	if (stat(file->filename, &FileStat) < 0) {
		perror(file->filename);
	} else {
		file->CreatedTime = FileStat.st_atime;
		file->ModifiedTime = FileStat.st_ctime;
#ifdef DEBUG
		LOG_MSG ("File : %s\n",file);
		LOG_MSG ("CreatedTime : %ld\n",FileStat.st_atime);
		LOG_MSG ("ModifiedTime : %ld\n",FileStat.st_ctime);
#endif
	}
}

int FileHandler::CheckFileInList(const char * filename,file_hashing_t * filelist)
{
	int i = 0;
	while (filelist[i].filename[0] != '\0') {
		if (strcmp(filelist[i].filename, filename) == 0)
			return (i);
		i++;
	}
	return (-1);
}
