#include "BufferManager.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>

using namespace std;

//make sure num-th exist in file, otherwise call new_block method
Block* BufferManager::get_block(File *file, int num)
{
	auto iter = file->blocks.begin();
	for (; iter != file->blocks.end(); iter++)
	{
		if ((*iter)->num == num)
			break;
	}
	if (iter != file->blocks.end())// block already in buffer
	{
		return *iter;
	}
	else // read block into buffer
	{
		Block *block = new Block;
		block->dirty = false;
		block->file = file;
		block->num = num;
		block->pin = false;
		block->address = new char[BLOCK_SIZE];
		FILE *pFile = fopen(file->filename.c_str(), "rb");
		fseek(pFile, num * BLOCK_SIZE + sizeof(FileHeader), SEEK_SET);
		fread(block->address, BLOCK_SIZE, 1, pFile);
		fclose(pFile);
		memcpy(&block->header, block->address, sizeof(BlockHeader));
		file->blocks.push_front(block);
		block_handler_.push_front(block);
		return block;
	}
}

int BufferManager::get_block_amount(const string& filename)
{
	File *file = open_file(filename);
	return file->header.block_count;
}

/*open existing files*/
File* BufferManager::open_file(const string& filename)
{
	auto iter = file_handler_.begin();
	for (; iter != file_handler_.end(); iter++)
	{
		if ((*iter)->filename == filename)
			break;
	}
	File *file;
	/* file is not found in the file_handler_ */
	if (iter == file_handler_.end())
	{
		FILE * pFile = fopen(filename.c_str(), "rb");
		FileHeader file_header;
		if (!pFile)
		{
			cout << "open file failed";
			return nullptr;
		}
		else
		{
			fseek(pFile, 0, SEEK_SET);
			fread(&file_header, sizeof(FileHeader), 1, pFile);
			fclose(pFile);
		}
		file = new File;
		file->header = file_header;
		file->filename = filename;
		if (file_handler_.size() == MAX_FILE)
			replace_file(file);
		else
			file_handler_.push_front(file);
	}
	else
		file = *iter;
	return file;
}

/* replacing new file with the most least used file */
void BufferManager::replace_file(File * file)
{
	auto iter = file_handler_.end();
	iter--;
	close_file(*iter);
	file_handler_.erase(iter);
	file_handler_.push_front(file);
}

void BufferManager::close_file(File * file)
{
	FILE *pFile = fopen(file->filename.c_str(), "rb+");
	if (!pFile)
	{
		cout << "open file fail";
		return;
	}
	fseek(pFile, 0, SEEK_SET);// write file header
	fwrite(&file->header, sizeof(FileHeader), 1, pFile);
	for (auto iter = file->blocks.begin(); iter != file->blocks.end();)
	{
		if ((*iter)->dirty)
			write_block(*iter, pFile);
		iter = file->blocks.erase(iter);
	}
	fclose(pFile);
}

void BufferManager::write_block(Block *block, FILE *pFile)
{
	memcpy((*block).address, &(*block).header, sizeof(BlockHeader));
	if (!pFile) // write single block without pFile
	{
		pFile = fopen(block->file->filename.c_str(), "rb+");
		fseek(pFile, sizeof(FileHeader) + block->num*BLOCK_SIZE, SEEK_SET);
		fwrite(block->address, BLOCK_SIZE, 1, pFile);// one block contains the block->header and its muliple records
		fclose(pFile);
	}
	else // write continuous blocks with pFile
	{
		fseek(pFile, sizeof(FileHeader) + block->num*BLOCK_SIZE, SEEK_SET);
		fwrite(block->address, BLOCK_SIZE, 1, pFile);// one block contains the block->header and its muliple records
	}
	delete[] block;
}

Block * BufferManager::new_block(File * file)
{
	Block *new_block = new Block;
	new_block->dirty = false;
	new_block->file = file;
	new_block->num = file->header.block_count++;
	new_block->header.record_count = 0;
	new_block->header.remain_record = (BLOCK_SIZE - sizeof(BlockHeader)) / file->header.record_size;
	new_block->pin = false;
	new_block->address = new char[BLOCK_SIZE];
	if (block_handler_.size() == MAX_BLOCK)
	{
		replace_block(new_block);
	}
	else
	{
		block_handler_.push_front(new_block);
	}
	file->blocks.push_front(new_block);
	return new_block;
}


/* replacing block by a least recently used block in the block_handler_ */
void BufferManager::replace_block(Block * block)
{
	//the most recently used block is placed at the beginning of the list 
	auto iter = block_handler_.end();
	iter--;
	for (; iter != block_handler_.end(); iter--)
	{
		if ((*iter)->pin == false)
			break;
	}
	if ((*iter)->dirty)
		write_block(*iter, nullptr);
	delete[] * iter;
	block_handler_.erase(iter);
	block_handler_.push_front(block);
}

void BufferManager::close_db(void)
{
	for (auto iter = file_handler_.begin(); iter != file_handler_.end();)
	{
		close_file(*iter);
		iter = file_handler_.erase(iter);
	}
}
