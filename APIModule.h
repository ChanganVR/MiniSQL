#ifndef APIMODULE_H
#define APIMODULE_H

#include<string>
#include "RecordManager.h"
#include "IndexManager.h"
#include "CatalogManager.h"

using namespace std;

class APIModule {
private:
	RecordManager *record_manager_;
	IndexManager *index_manager_;
	CatalogManager	*catalog_manager_;
public:
	APIModule(RecordManager *rm, IndexManager *im, CatalogManager *cm)
		:record_manager_(rm), index_manager_(im), catalog_manager_(cm){}
	~APIModule(){}
	void select(const string& table_name, Table& table, const vector<Condition>& conditions);
	void insert(const string& table_name, Table& table);
	void remove(const string& table_name, const Table& table, const vector<Condition> conditions);
	void create_table(const string& file_name, TableInfo * table);
};

#endif
