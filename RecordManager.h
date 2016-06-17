#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <string>
#include <vector>
#include "BufferManager.h"
#include "common.h"

using namespace std;

enum ConditionType {
	LESS,
	LESS_EQUAL,
	EQUAL,
	LARGER,
	LARGER_EQUAL,
	BETWEEN
};

struct Condition {
	int attr_offset;
	ConditionType type;
	string value;
};

class RecordManager {
private:
	BufferManager *buffer_manager_;
	void fill_record_address(char * record_temp, vector<int>& attr_type, Record & record);
	int calc_size(vector<int> attr_type);
	int block_not_in_buffer(File * file);
	Block* find_block_not_full(File *file);
	void read_tuple(char* record_address, const vector<int>& attr_type, vector<string>& attrs);;
	bool equal(const string& s1, const string& s2, int type);
	bool smaller(const string& s1, const string& s2, int type);
public:
	RecordManager(){}
	~RecordManager(){}
	void create_table(const string& filename, const vector<int>& attr_type);
	void set_buffer_manger(BufferManager *buffer_manager) { buffer_manager_ = buffer_manager; }
	void find_without_index(const string& filename, Table& table, const vector<Condition>& conditions);
	void find_with_index(const string& filename, Table& table);
	void select_without_index(string filename, vector<string> attr_field);
	void select_with_index(const string& filename, const Table& table, const vector<Condition>& conditions);
	void delete_without_index(const string& filename, const vector<int>& attr_type, const vector<Condition>& conditions);
	void delete_with_index(const string& filename, const Table& table);
	/* insert records into not full blocks and update records with block and record offset*/
	void insert(const string& filename, Table& table);
};


#endif 
