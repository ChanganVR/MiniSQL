#include "CatalogManager.h"
#include "common.h"
#include <iostream>
#include <fstream>

void CatalogManager::open_catalog_file(const string& filename)
{
	ifstream ifs;
	ifs.open(filename, ifstream::in);
	if (ifs.is_open())
	{
		int table_size;
		ifs >> table_size;
		for (int i = 0; i < table_size; i++)
		{
			TableInfo *table = new TableInfo;
			ifs >> table->table_name;
			ifs >> table->file_name;
			int attr_num;
			ifs >> attr_num;
			for (int j = 0; j < attr_num; j++)
			{
				Attr attr;
				ifs >> get<0>(attr);
				ifs >> get<1>(attr);
				ifs >> get<2>(attr);
				ifs >> get<3>(attr);
				table->attrs.push_back(attr);
			}
			table_list_.push_back(table);
		}
		
		int index_size;
		ifs >> index_size;
		for (int i = 0; i < index_size; i++)
		{
			IndexInfo *index = new IndexInfo;
			ifs >> index->index_name;
			ifs >> index->file_name;
			ifs >> index->attr_type;
			ifs >> index->attr_offset;
			index_list_.push_back(index);
		}
	}
}

void CatalogManager::write_catalog_file(const string& filename)
{//write back when list is dirty
	ofstream ofs;
	ofs.open(filename, ofstream::out);
	if (ofs.is_open())
	{
		ofs << table_list_.size() << "\n";
		for (auto table = table_list_.begin(); table != table_list_.end(); table++)
		{
			ofs << (*table)->table_name << "\n";
			ofs << (*table)->file_name << "\n";
			ofs << (*table)->attrs.size() << "\n" ;
			for (auto attr = (*table)->attrs.begin(); attr != (*table)->attrs.end(); attr++)
			{
				ofs << get<0>(*attr) << "\n";
				ofs << get<1>(*attr) << "\n";
				ofs << get<2>(*attr) << "\n";
				ofs << get<3>(*attr) << "\n";
			}
		}
		ofs << index_list_.size() << "\n";
		for (auto index = index_list_.begin(); index != index_list_.end(); index++)
		{
			ofs << (*index)->index_name << "\n";
			ofs << (*index)->file_name << "\n";
			ofs << (*index)->attr_type << "\n";
			ofs << (*index)->attr_offset<< "\n";
		}
	}
}

std::string CatalogManager::find_index_file(const string& index_name)
{
	for (auto index = index_list_.begin(); index != index_list_.end(); index++)
	{
		if ((*index)->index_name == index_name)
			return (*index)->file_name;
	}
	return string();
}

std::string CatalogManager::find_table_file(const string& table_name)
{
	for (auto table = table_list_.begin(); table != table_list_.end(); table++)
	{
		if ((*table)->table_name == table_name)
			return (*table)->file_name;
	}
	return string();
}

TableInfo * CatalogManager::find_table(const string & table_name)
{
	for (auto iter = table_list_.begin(); iter != table_list_.end(); iter++)
	{
		if ((*iter)->table_name == table_name)
			return *iter;
	}
}

void CatalogManager::list_all_tables()
{
	cout << "===Query OK===";
	for (auto table = table_list_.begin(); table != table_list_.end(); table++)
	{
		cout << endl << "===" << (*table)->table_name << " ";
		for (auto attr = (*table)->attrs.begin(); attr != (*table)->attrs.end(); attr++)
		{
			cout << endl << "===";
			cout << get<0>(*attr) << " ";
			switch(get<1>(*attr))
			{
			case -1:
				cout << "int ";
				break;
			case -2:
				cout << "float ";
				break;
			default:
				cout << "char(" << get<1>(*attr) << ") ";
			}
			if (get<2>(*attr))
				cout << "primary key ";
			if (get<3>(*attr))
				cout << "unique ";
		}
	}
	cout << endl;
}

void CatalogManager::list_all_indexes()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void CatalogManager::drop_table(const string& table_name)
{
	for (auto table = table_list_.begin(); table != table_list_.end(); )
	{
		if ((*table)->table_name == table_name)
		{
			remove((*table)->file_name.c_str());
			table_list_.erase(table);
			cout << "===Query OK===";
			return;
		}
		else
			table++;
	}
	throw(ParseError("Table " + table_name + " is not found, check your table name spelling"));
}
