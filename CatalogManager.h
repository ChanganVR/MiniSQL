#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#include <tuple>
#include <vector>
#include <string>

using namespace std;

typedef tuple<string, int, bool, bool> Attr;//attr_name, attr_type, is_primary, is_unique

struct TableInfo{
	string table_name;
	string file_name;
	vector<Attr> attrs;
};

struct IndexInfo {
	string index_name;
	string file_name;
	string table_name;
	string attr_name;
	int attr_type;
	int attr_offset;
};

class CatalogManager {
private:
	vector<TableInfo*> table_list_;
	vector<IndexInfo*> index_list_;
public:
	CatalogManager()
	{
		open_catalog_file("minisql.catalog");
	}
	~CatalogManager()
	{
		write_catalog_file("minisql.catalog");
	}
	void open_catalog_file(const string& filename);
	void write_catalog_file(const string& filename);
	void create_table(TableInfo *table)
	{
		table_list_.push_back(table);
	}
	void create_index(IndexInfo *index)
	{
		index_list_.push_back(index);
	}
	string find_index_file(const string& index_name);
	string find_table_file(const string& table_name);
	TableInfo *find_table(const string& table_name);
	void list_all_tables();
	void list_all_indexes();
	void drop_table(const string& table_name);
	void drop_index(const string& index_name){}
};

#endif
