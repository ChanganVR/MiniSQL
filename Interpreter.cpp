#include "Interpreter.h"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include "common.h"

using namespace std;
using namespace boost::algorithm;

void Interpreter::parse(string& sentence)
{
	to_lower(sentence);
	if (sentence == "\n")
		return;
	for (auto iter = sentence.begin(); iter != sentence.end(); iter++)
	{
		if ((*iter) == '(' || (*iter) == ')' || (*iter) == ',' || (*iter) == '=' || (*iter) == '*')
		{
			iter = sentence.insert(iter, ' ');
			iter = sentence.insert(iter + 2, ' ');
		}
		else if ((*iter) == '>' || (*iter) == '<')
		{
			if (*(iter + 1) == '=')
			{
				iter = sentence.insert(iter, ' ');
				iter = sentence.insert(iter + 3, ' ');
			}
			else
			{
				iter = sentence.insert(iter, ' ');
				iter = sentence.insert(iter + 2, ' ');
			}
		}
		else if ((*iter) == '\n')
			*iter = ' ';
		else if ((*iter) == ';')
		{
			iter = sentence.insert(iter, ' ');
			break;
		}
	}
	string token = get_token(sentence);
	if (token == "create")
	{
		string token2 = get_token(sentence);
		if (token2 == "index")
			create_index_clause(sentence);
		else if (token2 == "table")
			create_table_clause(sentence);
		else
			throw(ParseError("You have an error in your SQL syntax near 'create'"));
	}
	else if (token == "drop")
	{
		string token2 = get_token(sentence);
		if (token2 == "index")
		{
			string index_name = get_token(sentence);
			catalog_manager_->drop_index(sentence);
		}
		else if (token2 == "table")
		{
			string table_name = get_token(sentence);
			catalog_manager_->drop_table(table_name);
		}
		else
			throw(ParseError("You have an error in your SQL syntax near 'drop'"));
	}
	else if (token == "select")
	{
		select_clause(sentence);
	}
	else if (token == "insert")
	{
		insert_clause(sentence);
	}
	else if (token == "delete")
	{
		delete_clause(sentence);
	}
	else if (token == "quit")
	{
		throw(1);
	}
	else if (token == "list")
	{
		string next_token = get_token(sentence);
		if (next_token != "all")
			throw(ParseError("Your SQL misses 'all' after 'list'"));
		next_token = get_token(sentence);
		if (next_token == "tables")
			catalog_manager_->list_all_tables();
		else if (next_token == "indexes")
			catalog_manager_->list_all_indexes();
	}
	else if (token == "execfile")
	{
		ifstream ifs;
		ifs.open(get_token(sentence));
		string subSQL;
		if (ifs.is_open())
		{
			while (ifs.good())
			{
				getline(ifs, subSQL, ';');
				if (subSQL == "\n")
					break;
				subSQL += ";";
				try
				{
					parse(subSQL);
				}
				catch (ParseError error)
				{
					cout << error.error_message << endl;
				}
			}
		}
		else
			throw(ParseError("Your file can't not be opened, please check whether this file exists"));
	}
	else if (token == "--help")
	{
		cout << "Your SQL sentence should obey SQL syntax, which should end with a semicolon.\n"
			<< "where clause only supports 'and' operation\n"
			<< "Using 'quit' to quit miniSQL\n"
			<< "Using 'execfile [filename]' to execute multi SQL sentence in one file, each sentence should end up with a semicolon\n"
			<< "Using 'list all tables' to show all table informations\n"
			<< "Using 'list all indexes' to show all indexes informations\n"
			<< "Thanks for using";
	}
	else
		throw(ParseError("Your input is illegal"));
}


string Interpreter::get_token(string & sentence, const string& delim)
{
	trim_left_if(sentence, is_any_of(delim));
	auto  iter = find_if(sentence.begin(), sentence.end(), is_any_of(delim));
	string token = sentence.substr(0, iter - sentence.begin());
	sentence.erase(0, iter-sentence.begin());
	return token;
}

void Interpreter::create_table_clause(string & sentence)
{
	TableInfo *table = new TableInfo;
	table->table_name = get_token(sentence);
	if (get_token(sentence) != "(")
		throw(ParseError("Your SQL misses a left parenthesis"));
	while (1)
	{
		string attr_name = get_token(sentence);
		if (attr_name == ";"|| attr_name == ")")
			break;
		bool is_primary = false;
		if (attr_name == "primary" && get_token(sentence) == "key")
		{
			if (get_token(sentence) != "(")
				throw(ParseError("Your SQL misses a left parenthesis before primary key name"));
			attr_name = get_token(sentence);
			for (auto iter = table->attrs.begin(); iter != table->attrs.end(); iter++)
				if (get<0>(*iter) == attr_name)
				{
					get<2>(*iter) = true;
					get<3>(*iter) = true;//primary key must be unique
					break;
				}
			if (get_token(sentence) != ")")
				throw(ParseError("Your SQL misses a right parenthesis after primary key name"));
			continue;
		}
		string type = get_token(sentence);
		int attr_type;
		if (type == "int")
			attr_type = -1;
		else if (type == "float")
			attr_type = -2;
		else if (type.substr(0, 4) == "char")
		{
			if (get_token(sentence) != "(")
				throw(ParseError("Your SQL misses a left parenthesis before char"));
			attr_type = stoi(get_token(sentence));
			if (get_token(sentence) != ")")
				throw(ParseError("Your SQL misses a right parenthesis after char"));
		}
		else
			throw(ParseError("Attribute type can't be recognized"));
		string str = get_token(sentence);
		bool is_unique = false;
		if (str == "unique")
			is_unique = true;
		else
			sentence = str + " " + sentence;
		Attr attr(attr_name, attr_type, is_primary, is_unique);
		table->attrs.push_back(attr);
		string tuple_end = get_token(sentence);
		if (tuple_end == "," || tuple_end == ")")
			continue;
		else 
			throw(ParseError("You have an error in your SQL syntax near )"));
	}
	table->file_name = table->table_name + ".table";
	api_module_->create_table(table->file_name, table);
	catalog_manager_->create_table(table);
}

void Interpreter::create_index_clause(string & sentence)
{
	IndexInfo *index = new IndexInfo();
	index->index_name = get_token(sentence);
	if (get_token(sentence) != "on")
		throw(ParseError("Your SQL misses 'on' certain attribute"));
	index->table_name = get_token(sentence);
	index->attr_name = get_token(sentence);
	index->file_name = index->index_name + ".index";
	TableInfo *table = catalog_manager_->find_table(index->table_name);
	for (auto attr = table->attrs.begin(); attr != table->attrs.end(); attr++)
	{
		if (get<0>(*attr) == index->attr_name)
		{
			index->attr_offset = attr - table->attrs.begin();
			index->attr_type = get<1>(*attr);
			break;
		}
	}
	catalog_manager_->create_index(index);
}

void Interpreter::select_clause(string& sentence)
{
	if (get_token(sentence) != "*")
		throw(ParseError("MiniSQL now only suports select *"));
	if (get_token(sentence) != "from")
		throw(ParseError("Your SQL misses 'from' after 'select'"));
	string table_name = get_token(sentence);
	TableInfo* table_info = catalog_manager_->find_table(table_name);
	Table table;
	for (auto iter = table_info->attrs.begin(); iter != table_info->attrs.end(); iter++)
	{
		table.attr_type.push_back(get<1>(*iter));
	}
	string next_token = get_token(sentence);
	vector<Condition> conditions;
	if (next_token == "where")
		where_clause(table_info, conditions, sentence);
	api_module_->select(table_name, table, conditions);
}

void Interpreter::insert_clause(string& sentence)
{
	if (get_token(sentence) != "into")
		throw(ParseError("Your SQL misses 'into' after 'insert'"));
	string table_name = get_token(sentence);
	if (get_token(sentence) != "values")
		throw(ParseError("Your SQL misses 'values'"));
	TableInfo *table_info = catalog_manager_->find_table(table_name);
	Table table;
	if(!table_info)
		throw(ParseError("The table is not found, please check your SQL syntax"));
	for (auto iter = table_info->attrs.begin(); iter != table_info->attrs.end(); iter++)
	{
		table.attr_type.push_back(get<1>(*iter));
	}
	Record record;
	if (get_token(sentence) != "(")
		throw(ParseError("Your SQL misses a left parenthesis after 'values'"));
	for (auto iter = table_info->attrs.begin(); iter != table_info->attrs.end(); iter++)
	{
		string token = get_token(sentence);
		if (get<1>(*iter) > 0)//char
			check_string(token);
		get<2>(record).push_back(token);
		string str = get_token(sentence);
		if (str == ",")
			continue;
		else if (str == ")")
			break;
		else 
			throw(ParseError("Your SQL has an error near sentence end"));
	}
	table.records.push_back(record);
	api_module_->insert(table_name, table);
}

void Interpreter::delete_clause(string& sentence)
{
	if (get_token(sentence) != "from")
		throw(ParseError("Your SQL misses 'from'"));
	string table_name = get_token(sentence);
	TableInfo* table_info = catalog_manager_->find_table(table_name);
	Table table;
	if (!table_info)
		throw(ParseError("Table " + table_name + " cannot be found, check your SQL spelling"));
	for (auto iter = table_info->attrs.begin(); iter != table_info->attrs.end(); iter++)
	{
		table.attr_type.push_back(get<1>(*iter));
	}
	string next_token = get_token(sentence);
	vector<Condition> conditions;
	if (next_token == "where")
		where_clause(table_info, conditions, sentence);
	api_module_->remove(table_name, table, conditions);
}

void Interpreter::check_string(string& token)
{
	if (!starts_with(token, "'"))
		throw(ParseError("Your SQL character string misses a left quote"));
	if (!ends_with(token, "'"))
		throw(ParseError("Your SQL character string misses a right quote"));
	erase_all(token, "'");
}

void Interpreter::where_clause(const TableInfo* table_info, vector<Condition>& conditions, string& sentence)
{
	while (1)
	{
		Condition condition;
		string attr_name = get_token(sentence);
		for (auto iter = table_info->attrs.begin(); iter != table_info->attrs.end(); iter++)
		{
			if (attr_name == get<0>(*iter))
			{
				condition.attr_offset = iter - table_info->attrs.begin();
				break;
			}
		}
		string compare_sym = get_token(sentence);
		if (compare_sym == "between")
		{
			condition.type == BETWEEN;
			condition.value = get_token(sentence);
			assert(get_token(sentence) == "and");
			condition.value += " " + get_token(sentence);
			continue;
		}
		else if (compare_sym == "<")
			condition.type = LESS;
		else if (compare_sym == "<=")
			condition.type = LESS_EQUAL;
		else if (compare_sym == "=")
			condition.type = EQUAL;
		else if (compare_sym == ">")
			condition.type = LARGER;
		else if (compare_sym == ">=")
			condition.type = LARGER_EQUAL;
		else
			throw(ParseError("Unknown comparison operators"));
		string value = get_token(sentence);
		if (starts_with(value, "'") && ends_with(value, "'"))
			erase_all(value, "'");
		condition.value = value;
		conditions.push_back(condition);
		string clause_end = get_token(sentence);
		if (clause_end == "and")
			continue;
		else if (clause_end == ";")
			break;
		else
			throw(ParseError("Your SQL has an error near sentence end"));
	}
}
