#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H

#include "BufferManager.h"
#include "common.h"

class IndexManager {
private:
	BufferManager *buffer_manager_;
	char* read_node(File *file, NodePointer  node_ptr);
	void val_to_cstring(char* insert_value, int attr_type, const string& attr_value);
	Block * find_block_not_full(File *file);
	int block_not_in_buffer(File *file);
public:
	IndexManager() {}
	~IndexManager() { buffer_manager_->close_db(); }
	void set_buffer_manager(BufferManager *buffer_manger) { buffer_manager_ = buffer_manger; }
	void create_index_file(const string& indexname, int attr_type);
	void insert(const string& indexname, const Table& table, int attr_offset);
	void find(const string& indexname, Table &table, int attr_offset);
	void remove(const string& indexname, const Table & table, int attr_offset);
};

#endif
