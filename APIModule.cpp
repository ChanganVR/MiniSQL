#include "APIModule.h"
#include <iostream>

void APIModule::select(const string& table_name, Table& table, const vector<Condition>& conditions)
{
	//TODO:check B+ tree first	
	string file_name = catalog_manager_->find_table_file(table_name);
	record_manager_->find_without_index(file_name, table, conditions);
	cout << "===Query OK===" << endl;
	for (auto record = table.records.begin(); record != table.records.end(); record++)
	{
		//printf each tuple
		for (auto attr = get<2>(*record).begin(); attr != get<2>(*record).end(); attr++)
		{
			cout << *attr << "\t";
		}
		cout << endl;
	}
}

void APIModule::insert(const string& table_name, Table& table)
{
	TableInfo *table_info = catalog_manager_->find_table(table_name);
	if (!table_info)
		throw(ParseError("Table " + table_name + " not found, check your spelling"));
		//for each attribute, if unique, check all records in file whether duplicates
	for (auto attr = table_info->attrs.begin();attr!=table_info->attrs.end();attr++)
	{
		int attr_offset = attr - table_info->attrs.begin();
		vector<Condition> conditions;
		Condition condition;
		condition.attr_offset = attr_offset;
		condition.type = EQUAL;
		if (get<3>(table_info->attrs[attr_offset]))//is unique
		{
			//for each tuple
			for (auto record = table.records.begin(); record != table.records.end(); )
			{
				condition.value = get<2>(*record)[attr_offset];
				conditions.clear();
				conditions.push_back(condition);
				Table exist_records;
				exist_records.attr_type = table.attr_type;
				record_manager_->find_without_index(table_info->file_name, exist_records, conditions);
				if (!exist_records.records.empty())
				{
					record = table.records.erase(record);
					throw(ParseError("Attribute "  + get<0>(table_info->attrs[attr_offset]) + " is unique, your insert value duplicates"));
				}
				else
					record++;
			}
		}
	}
	string file_name = table_info->file_name;
	record_manager_->insert(file_name, table);
	cout << "===Query OK===" << endl;
}

void APIModule::remove(const string& table_name, const Table& table, const vector<Condition> conditions)
{
	string file_name = catalog_manager_->find_table_file(table_name);
	record_manager_->delete_without_index(file_name, table.attr_type, conditions);
	cout << "===Query OK===" << endl;
}

void APIModule::create_table(const string& file_name, TableInfo * table)
{
	vector<int> attr_type;
	for (auto iter = table->attrs.begin(); iter != table->attrs.end(); iter++)
	{
		attr_type.push_back(get<1>(*iter));
	}
	record_manager_->create_table(file_name, attr_type);
	cout << "===Query OK===" << endl;
}