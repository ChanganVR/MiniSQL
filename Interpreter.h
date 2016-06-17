#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include "APIModule.h"
#include "common.h"

using namespace std;

class Interpreter {
private:
	APIModule *api_module_;
	CatalogManager *catalog_manager_;
public:
	Interpreter(){}
	~Interpreter(){}
	void set_api_module(APIModule *api_module) { api_module_ = api_module; }
	void set_catalog_manager(CatalogManager *catalog_manager) { catalog_manager_ = catalog_manager; }
	void parse(string& sentence);
	string get_token(string& sentence, const string& delim = " ");
	void create_table_clause(string& sentence);
	void create_index_clause(string& sentence);
	void select_clause(string& sentence);
	void insert_clause(string& sentence);
	void delete_clause(string& sentence);
	void check_string(string& token);
	void where_clause(const TableInfo* table_info, vector<Condition>& conditions, string& sentence);
};

#endif