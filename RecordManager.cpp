#include "RecordManager.h"
#include <assert.h>

void RecordManager::find_without_index(const string& filename, Table& table, const vector<Condition>& conditions)
{
	File *file = buffer_manager_->open_file(filename);
	for (int i = 0; i < file->header.block_count; i++)
	{
		Block * block = buffer_manager_->get_block(file, i);
		char* record_address = block->address + sizeof(BlockHeader);
		for (int j = 0; j < block->header.record_count; j++, record_address += file->header.record_size)
		{
			Record record;
			get<0>(record) = i;
			get<1>(record) = j;
			read_tuple(record_address, table.attr_type, get<2>(record));
			vector<string>& attrs = get<2>(record);
			for (auto condition = conditions.begin(); condition != conditions.end() && !attrs.empty(); condition++)
			{
				string& attr_value = attrs[(*condition).attr_offset];
				int attr_type = table.attr_type[(*condition).attr_offset];
				switch ((*condition).type)
				{
				case LESS:
					if (!smaller(attr_value, (*condition).value, attr_type))
						attrs.clear();
					break;
				case LESS_EQUAL:
					if (!smaller(attr_value, (*condition).value, attr_type) && !equal(attr_value, (*condition).value, attr_type))
						attrs.clear();
					break;
				case EQUAL:
					if (!equal(attr_value, (*condition).value, attr_type))
						attrs.clear();
					break;
				case LARGER:
					if (smaller(attr_value, (*condition).value, attr_type) || equal(attr_value, (*condition).value, attr_type))
						attrs.clear();
					break;
				case LARGER_EQUAL:
					if (smaller(attr_value, (*condition).value, attr_type))
						attrs.clear();
					break;
				case BETWEEN:// closed interval
					{
						auto offset = (*condition).value.find(' ');
						string s1 = (*condition).value.substr(0, offset);
						string s2 = (*condition).value.substr(offset + 1);
						if (smaller(attr_value, s1, attr_type) || smaller(s2, attr_value, attr_type))
							attrs.clear();
						break;
					}
				default:
					assert(1);
				}
			}
			if (!get<2>(record).empty())
				table.records.push_back(record);
		}
	}
}

void RecordManager::read_tuple(char* record_address, const vector<int>& attr_type, vector<string>& attrs)
{
	char is_deleted;
	memcpy(&is_deleted, record_address, 1);
	record_address += 1;
	if (is_deleted)//1 is written in the head of a record when deleting this record
		return;
	for (auto iter = attr_type.begin(); iter != attr_type.end(); iter++)
	{
		if (*iter == -1)
		{
			int a;
			memcpy(&a, record_address, sizeof(int));
			attrs.push_back(to_string(a));
			record_address += sizeof(int);
		}
		else  if (*iter == -2)
		{
			float a;
			memcpy(&a, record_address, sizeof(float));
			attrs.push_back(to_string(a));
			record_address += sizeof(float);
		}
		else
		{
			char *a = new char[*iter + 1];
			memcpy(a, record_address, *iter);
			a[*iter] = 0;
			string str(a);
			//str.erase(str.find((char)0x00));
			attrs.push_back(str);
			record_address += *iter;
		}
	}
}

void RecordManager::delete_without_index(const string& filename, const vector<int>& attr_type, const vector<Condition>& conditions)
{
	Table table;
	table.attr_type = attr_type;
	find_without_index(filename, table, conditions);
	delete_with_index(filename, table);
}

void RecordManager::delete_with_index(const string & filename, const Table & table)
{
	File *file = buffer_manager_->open_file(filename);
	for (auto record = table.records.begin(); record != table.records.end(); record++)
	{
		int block_offset = get<0>(*record);
		int record_offset = get<1>(*record);
		Block *block = buffer_manager_->get_block(file, block_offset);
		char *record_address = block->address + sizeof(BlockHeader) + record_offset*block->file->header.record_size;
		memset(record_address, 1, 1);
		block->dirty = true;
	}
}

bool RecordManager::equal(const string& s1, const string& s2, int type)
{
	if (type == -1)
		return stoi(s1) == stoi(s2);
	else if (type == -2)
		return stof(s1) == stof(s2);
	else
		return s1 == s2;
}

bool RecordManager::smaller(const string& s1, const string& s2, int type)
{
	if (type == -1)
		return stoi(s1) < stoi(s2);
	else if (type == -2)
		return stof(s1) < stof(s2);
	else
		return s1 < s2;
}

void RecordManager::insert(const string& filename, Table& table)
{
	File *file = buffer_manager_->open_file(filename);
	Block *block = find_block_not_full(file);
	//insert records into not full block tuple by tuple
	for (auto record_iter = table.records.begin(); record_iter != table.records.end(); record_iter++)
	{
		if (block->header.remain_record == 0)
			block = find_block_not_full(file);
		block->dirty = true;
		char *record_address = block->address + sizeof(BlockHeader) + block->file->header.record_size * block->header.record_count;
		memset(record_address, 0, sizeof(char)); // set the deleted flag as 0
		fill_record_address(record_address + sizeof(char), table.attr_type, *record_iter);
		block->header.record_count++;
		block->header.remain_record--;
		get<0>(*record_iter) = block->num;
		get<1>(*record_iter) = block->header.record_count - 1;
	}
}

void RecordManager::create_table(const string& filename, const vector<int>& attr_type)
{
	int block_amount = 0;
	int record_size = 1 + calc_size(attr_type);
	FILE *pFile = fopen(filename.c_str(), "w");
	fwrite(&block_amount, sizeof(int), 1, pFile);
	fwrite(&record_size, sizeof(int), 1, pFile);
	fclose(pFile);
}

void RecordManager::fill_record_address(char * record_address, vector<int>& attr_type, Record & record)
{
	for (int i = 0; i < attr_type.size(); i++)
	{
		if (attr_type[i] == -1)
		{
			int a = stoi(get<2>(record)[i]);
			memcpy(record_address, &a, sizeof(int));
			record_address += sizeof(int);
		}
		else if (attr_type[i] == -2)
		{
			float a = stof(get<2>(record)[i]);
			memcpy(record_address, &a, sizeof(float));
			record_address += sizeof(float);
		}
		else
		{
			int len = get<2>(record)[i].length();
			memcpy(record_address, get<2>(record)[i].c_str(), len);
			memset(record_address + len, 0, attr_type[i] - len);
			record_address += attr_type[i];	
		}
	}
}

int RecordManager::calc_size(vector<int> attr_type)
{
	int size = 0;
	for (auto iter = attr_type.begin(); iter != attr_type.end(); iter++)
	{
		if (*iter == -1 || *iter == -2)
			size += 4;
		else
			size += *iter;
	}
	return size;
}

int RecordManager::block_not_in_buffer(File * file)
{
	bool *all_block = new bool[file->header.block_count]{ false };
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

Block * RecordManager::find_block_not_full(File * file)
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
