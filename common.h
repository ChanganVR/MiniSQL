#ifndef COMMON_H
#define COMMON_H

#include <tuple>
#include <vector>
#include <list>
using namespace std;

#define BLOCK_SIZE 4096 //4kB
#define MAX_FILE 5
#define MAX_BLOCK 40
#define NODE_CAPACITY 10

struct BlockHeader {
	int record_count;
	int remain_record;
};

struct File;
struct Block {
	BlockHeader header;
	int num;
	bool dirty;
	bool pin;
	char *address;
	File *file;
};

struct FileHeader {
	int block_count;
	int record_size;
};

struct File {
	string filename;
	FileHeader header;
	list<Block*> blocks;
};

struct NodePointer {
	int block_offset;
	int node_offset;
};

typedef tuple<int, int, vector<string>> Record;

struct Table {
	vector<int> attr_type;
	vector<Record> records;// block offset, tuple offset, attributes of a record
};

int calc_size(int attr_type);

class ParseError {
public:
	string error_message;
	ParseError(string message) :error_message(message) {}
};

#endif
