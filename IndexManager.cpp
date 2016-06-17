#include "IndexManager.h"
#include "Node.h"
#include <iostream>

char* IndexManager::read_node(File *file, NodePointer node_ptr)
{
	Block* block = buffer_manager_->get_block(file, node_ptr.block_offset);
	return block->address + sizeof(BlockHeader) + file->header.record_size * node_ptr.node_offset;
}

void IndexManager::val_to_cstring(char* addr, int attr_type, const string& attr_value)
{
	if (attr_type == -1)
	{
		int a = stoi(attr_value);
		memcpy(addr, &a, sizeof(int));
	}
	else if (attr_type == -2)
	{
		float a = stof(attr_value);
		memcpy(addr, &a, sizeof(float));
	}
	else
	{
		memcpy(addr, attr_value.c_str(), attr_type);
	}
}

void IndexManager::create_index_file(const string& indexname, int attr_type)
{
	FILE *pFile = fopen(indexname.c_str(), "wb");
	if (!pFile)
	{
		cout << "create index fail";
	}
	FileHeader file_header;
	file_header.block_count = 0;
	file_header.record_size = 2 + NODE_CAPACITY * (sizeof(NodePointer) + calc_size(attr_type));
	fwrite(&file_header, sizeof(FileHeader), 1, pFile);
	fclose(pFile);

	//create an empty node in the first block
	File *file = buffer_manager_->open_file(indexname);
	Block *block = find_block_not_full(file);
	block->dirty = true;
	char *node_address = block->address + sizeof(BlockHeader) + file->header.record_size * block->header.record_count;
	memset(node_address, -1, file->header.record_size);
	node_address[0] = 0;//deleted flag bit is set 0
	node_address[1] = 1;//leaf flag bit is set 1
	block->header.record_count++;
	block->header.remain_record--;
}

/* records in table have block_offset and record_offset */
void IndexManager::insert(const string & indexname, const Table & table, int attr_offset)
{
	File *file = buffer_manager_->open_file(indexname);
	int attr_type = table.attr_type[attr_offset];
	for (auto record = table.records.begin(); record != table.records.end(); record++)
	{
		string attr_value = get<2>(*record)[attr_offset];
		list<Node*> node_list;
		Node *node = new Node(attr_type, read_node(file, { 0,0 }), { 0,0 });// NodePointer{0,0} is head node of B+ tree
		node_list.push_back(node);
		while (!node->is_leaf())// if only one node exist in B+ tree, it is marked as leaf
		{
			NodePointer node_pointer = node->find(attr_value);
			Node * next_node = new Node(attr_type, read_node(file, node_pointer), node_pointer);
			node_list.push_back(next_node);
			node = next_node;
		}
		if (!node->is_full())
		{
			NodePointer node_pointer{ get<0>(*record), get<1>(*record) };
			node->insert(node_pointer, attr_value);
			Block *block = buffer_manager_->get_block(file, node->get_pointer().block_offset);
			block->dirty = true;
		}
		else
		{
			//initial insert attr as a leaf node, insert_value is the first value of this node and get<0>,get<1> is the node pointer
			char* insert_value = new char[2 + sizeof(NodePointer) +calc_size(attr_type)];
			//memcpy(insert_value + 2, &get<0>(*record), sizeof(int));
			//memcpy(insert_value + 2 + sizeof(int), &get<1>(*record), sizeof(int));
			val_to_cstring(insert_value + 2 + sizeof(NodePointer), attr_type, attr_value);//TODO:add to common method
			Node *insert_node = new Node(attr_type, insert_value, {get<0>(*record), get<1>(*record)});
			auto node = node_list.end();
			node--;//last node in node_list
			while ((*node)->is_full())
			{
				Block *block = find_block_not_full(file);
				block->dirty = true;
				char *node_address = block->address + sizeof(BlockHeader) + block->file->header.record_size * block->header.record_count;
				block->header.record_count++;
				block->header.remain_record--;
				NodePointer node_pointer{ block->num ,block->header.record_count-1};
				node_address[0] = 0;
				node_address[1] = (char)(*node)->is_leaf();
				Node *new_node = new Node(attr_type, node_address, node_pointer);
				new_node->reset();
				(*node)->move_half(new_node);
				(*new_node).insert(insert_node->get_pointer(), insert_node->get_first_value());
				if (node == node_list.begin())//head node, copy head node to copy_node, insert copy_node and new_node to original head node
				{
					Block *block = find_block_not_full(file);
					block->dirty = true;
					char *node_address = block->address + sizeof(BlockHeader) + block->file->header.record_size * block->header.record_count;
					block->header.record_count++;//TODO: add method allocating a record in buffer manager for both record and index node
					block->header.remain_record--;
					(*node)->copy_value(node_address);
					NodePointer node_pointer{ block->num, block->header.record_count - 1 };
					node_address[0] = 0;
					node_address[1] = 1;
					Node *copy_node = new Node(attr_type, node_address, node_pointer);
					(*node)->reset();
					(*node)->insert(node_pointer, copy_node->get_first_value());
					(*node)->insert(new_node->get_pointer(), new_node->get_first_value());
					(*node)->get_address()[1] = 0;//change to nonleaf node
					break;
				}
				node--;
				insert_node = new_node;
			}
		}
	}
}

void IndexManager::find(const string & indexname, Table & table, int attr_offset)
{
	File *file = buffer_manager_->open_file(indexname);
	for (auto record = table.records.begin(); record != table.records.end(); record++)
	{
		string &attr_value = get<2>(*record)[attr_offset];
		Node *node = new Node(table.attr_type[attr_offset], read_node(file, { 0, 0 }), { 0,0 });
		while (!node->is_leaf())// if only one node exist in B+ tree, it is marked as leaf
		{
			NodePointer node_pointer = node->find(attr_value);
			Node * next_node = new Node(table.attr_type[attr_offset], read_node(file, node_pointer), node_pointer);
			node = next_node;
		}
		NodePointer record_pointer = node->find(attr_value);
		get<0>(*record) = record_pointer.block_offset;
		get<1>(*record) = record_pointer.node_offset;
	}
}

Block * IndexManager::find_block_not_full(File * file)
{
	//find not full block in buffer
	for (auto block_iter = file->blocks.begin(); block_iter != file->blocks.end(); block_iter++)
	{
		if ((*block_iter)->header.remain_record > 0)
			return *block_iter;
	}
	Block *block;
	//find block not in buffer	
	while (file->header.block_count != file->blocks.size())
	{
		int num = block_not_in_buffer(file);
		if (num == -1)
			block = buffer_manager_->new_block(file);
		else
			block = buffer_manager_->get_block(file, num);
		if (block->header.remain_record > 0)
			return block;
		else
			continue;
	}
	//all blocks are full, create a new block
	block = buffer_manager_->new_block(file);
	return block;
}

int IndexManager::block_not_in_buffer(File * file)
{
	bool *all_block = new bool[file->header.block_count]{ false };
	if (file->header.block_count == 0)//file has no block
		return -1;
	for (auto iter = file->blocks.begin(); iter != file->blocks.end(); iter++)
	{
		all_block[(*iter)->num] = true;
	}
	for (int i = 0; i < file->header.block_count; i++)
	{
		if (!all_block[i])
			return i;
	}
}

