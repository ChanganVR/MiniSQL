#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <vector>
#include <list>
#include <string>
#include "common.h"

using namespace std;

class BufferManager {
private:
	list<File*> file_handler_;
	list<Block*> block_handler_;
public:
	BufferManager() {};
	~BufferManager() { close_db(); };
	Block* get_block(File *file, int num);
	int get_block_amount(const string& filename);
	File* open_file(const string& filename);
	void replace_file(File*file);
	void close_file(File *file);
	void write_block(Block *block, FILE *pFile);
	Block* new_block(File * file);
	void replace_block(Block *block);
	void close_db(void);
};

#endif