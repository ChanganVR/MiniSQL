#ifndef NODE_H
#define NODE_H

#include <string>
#include "common.h"
using namespace std;

class Node {
private:
	char * address_;//attr_value is stored as binary number instead of ascii codes in memory
	int attr_type_;
	int current_element_;
	int node_size_;
	int attr_size_;
	int unit_size_;
	NodePointer node_pointer_;
	Node() {}
public:
	Node(int attr_type, char* address, NodePointer node_pointer)
		:address_(address), attr_type_(attr_type), node_pointer_(node_pointer)
	{
		attr_size_ = calc_size(attr_type);
		unit_size_ = attr_size_ + sizeof(NodePointer);
		node_size_ = 2 + NODE_CAPACITY * unit_size_;
		calc_current_element();
	}
	~Node() {}
	char* get_first_value(void);
	//NodePointer find_next_node(const string& attr_value);
	NodePointer get_pointer(void) { return node_pointer_; }
	void insert(NodePointer node_pointer, const char *attr_value);//attr_value should be stored in binary number
	void insert(NodePointer node_pointer, const string & attr_value);
	NodePointer find(const string& attr_value);
	void remove(const string& attr_value);
	bool is_leaf(void) { return (bool)address_[1]; }
	void calc_current_element();
	bool is_full(void);
	void move_half(Node * new_node);
	void copy_value(char* node_addresss);
	void reset();
	int compare(const char* s1, const string& s2);
	int compare(const char* s1, const char* s2);
	char * get_address();
};

#endif
