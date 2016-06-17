#include <tuple>
#include <iostream>
#include "MiniSQL.h"
#include "BufferManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "Interpreter.h"
#include "Node.h"

using namespace std;

void test_node()
{
	Table table;
	table.attr_type = { -1, -2, 10 };
	int record_size = 2 + NODE_CAPACITY * (sizeof(NodePointer) + calc_size(table.attr_type[0]));
	char *node_address = new char[record_size];
	Node *node = new Node(-1, node_address, { 0,0 });
	char *new_node_address = new char[record_size];
	Node *new_node = new Node(-1, new_node_address, { 0,0 });
	node->reset();
	node_address[1] = 1;
	node->insert({ 1,1 }, string("1"));
	node->insert({ 2,2 }, string("5"));
	node->insert({ 2,2 }, string("3"));
	node->insert({ 2,2 }, string("6"));
	node->insert({ 3,3 }, string("2"));
	node->insert({ 2,2 }, string("4"));
	node->insert({ 2,2 }, string("7"));
	node->insert({ 2,2 }, string("8"));
	node->insert({ 2,2 }, string("9"));
	node->insert({ 10,10 }, string("10"));
	NodePointer node_ptr = node->find("10");
	//node->move_half(new_node);
}

void test_record_manager()
{
	BufferManager bm;
	RecordManager rm;
	rm.set_buffer_manger(&bm);

	Table table;
	table.attr_type = vector<int>{ -1,-2,10 };

	rm.create_table("hello.table", table.attr_type);
	for (int i = 0; i < 1000; i++)
	{
		Record record;
		get<2>(record) = vector<string>{ "4","3.3","hello" };
		table.records.push_back(record);
	}
	rm.insert("hello.table", table);

	vector<Condition> conditions;
	Condition con{ 0, LESS, "5" };
	conditions.push_back(con);
	//rm.delete_without_index("hello.table", table.attr_type, conditions);
	rm.find_without_index("hello.table", table, conditions);
	for (auto record_iter = table.records.begin(); record_iter != table.records.end(); record_iter++)
	{
		cout << get<0>(*record_iter) << "\t" << get<1>(*record_iter) << "\t";
		for (auto attr_iter = get<2>(*record_iter).begin(); attr_iter != get<2>(*record_iter).end(); attr_iter++)
		{
			cout << *attr_iter << "\t";
		}
		cout << endl;
	}
}

void test_index_manager()
{
	IndexManager im;
	BufferManager *bm = new BufferManager();
	im.set_buffer_manager(bm);

	Table table;
	table.attr_type = { -1,-2,10 };
	for (int i = 1; i <= 10; i++)
	{
		Record record;
		get<0>(record) = 12;
		get<1>(record) = 12;
		get<2>(record) = { to_string(i), "2.2", "hello" };
		table.records.push_back(record);
	}
	
	//im.create_index_file("hello.index", -1);
	im.insert("hello.index", table, 0);
	//im.find("hello.index", table, 0);
	cout << "hh";
}

/*-----------------------------------------------------------------
Variable name explanation:
max: max number of file or block or record, starting from 1
count: existing number of file or block or record, starting from 1
num: the ordinal number of file or block or record, starting from 0
------------------------------------------------------------------*/

int main()
{
	//test_index_manager();
	//test_index_manager();

	cout << "\t\t***********************************************" << endl;
	cout << "\t\t             Welcome to use MiniSQL !" << endl;
	cout << "\t\t               Version (1.0)  " << endl;
	cout << "\t\t   Author: Chen Changan, q604815016@gmail.com" << endl;
	cout << "\t\t      Copyright(C) 2016-all right reserved !" << endl;
	cout << "\t\t***********************************************" << endl;
	cout << endl;

	BufferManager bm;
	RecordManager rm;
	rm.set_buffer_manger(&bm);
	//IndexManager im;
	CatalogManager cm;
	APIModule api(&rm, NULL, &cm);
	Interpreter parser;
	parser.set_api_module(&api);
	parser.set_catalog_manager(&cm);
	string create_table = "create table student ( id int, height float, name char(20) unique, primary key(id));";
	string create_index = "create index stu_name on student(name);";
	string select = "select * from student where id = 2;";
	string insert = "insert into student values(2, 172, 'world');";
	string remove = "delete from student where id = 1;";
	string sentence;
	//sentence = remove;
	bool run = true;
	while (run)
	{
		cout << "Enter your SQL sentence, use --help for details." << endl;
		cout << ">>>";
		getline(cin, sentence, '\n');
		try
		{
			parser.parse(sentence);
		}
		catch (ParseError& error)
		{
			cout << "===" << error.error_message << "===" << endl;
			continue;
		}
		catch (int i)
		{
			if (i == 1)
				run = false;
		}
	}
}