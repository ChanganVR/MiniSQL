#include "Node.h"
#include <assert.h>

char * Node::get_first_value(void)
{
	return address_ + 2 * sizeof(char) + sizeof(NodePointer);
}

//attr_value has already been converted to binary represents
void Node::insert(NodePointer node_pointer, const char * attr_value) 
{
	int i = 2;
	for (; i <= node_size_; i += unit_size_)
	{
		if (*(int*)(address_ + i) == -1 || compare(address_ + i + sizeof(NodePointer), attr_value) == 1)//break when ith value > attr_value
			break;
	}
	int j = 2 + (current_element_ - 1) * unit_size_;
	for (; j >= i; j = j - unit_size_)// empty room for attr_value
	{
		memcpy(address_ + j + unit_size_, address_ + j, unit_size_);
	}
	memcpy(address_ + i, &node_pointer, sizeof(NodePointer));
	memcpy(address_ + i + sizeof(NodePointer), attr_value, attr_size_);
	current_element_++;
}

void Node::insert(NodePointer node_pointer, const string & attr_value)
{
	assert(current_element_ != NODE_CAPACITY);
	char* ptr;
	if (attr_type_ == -1)
	{
		int a = stoi(attr_value);
		ptr = new char[4];
		memcpy(ptr, &a, attr_size_);
	}
	else if (attr_type_ == -2)
	{
		float a = stof(attr_value);
		ptr = new char[4];
		memcpy(ptr, &a, attr_size_);
	}
	else
	{
		ptr = new char[attr_type_];
		memcpy(ptr, attr_value.c_str(), attr_size_);
	}
	insert(node_pointer, ptr);
	delete[] ptr;
}

NodePointer Node::find(const string & attr_value)
{
	int i;
	NodePointer record_pointer;
	for (i = 2; i <= 2 + (current_element_ - 1)*unit_size_; i += unit_size_)
	{
		if (compare(address_ + i + sizeof(NodePointer), attr_value) == -1)//larger
		{
			continue;
		}
		else if (compare(address_ + i + sizeof(NodePointer), attr_value) == 0)//equal
		{
			record_pointer.block_offset = *(int*)(address_ + i);
			record_pointer.node_offset = *(int*)(address_ + i + sizeof(int));
			return record_pointer;
		}
		else//smaller
		{
			if (i == 2)
				break;
			i -= unit_size_;
			record_pointer.block_offset = *(int*)(address_ + i);
			record_pointer.node_offset = *(int*)(address_ + i + sizeof(int));
			return record_pointer;
		}
	}
	//larger than all records
	if (i != 2)
		i -= unit_size_;
	record_pointer.block_offset = *(int*)(address_ + i);
	record_pointer.node_offset = *(int*)(address_ + i + sizeof(int));
	return record_pointer;
}

void Node::calc_current_element()
{
	char * address_temp = address_ + 2;
	for (current_element_ = 0; current_element_ < NODE_CAPACITY; current_element_++)
	{
		if (address_temp[0] != -1)
			address_temp += sizeof(NodePointer) + attr_size_;
		else
			break;
	}
}

bool Node::is_full(void)
{
	return current_element_ == NODE_CAPACITY;
}

void Node::move_half(Node * new_node)
{
	char * addr = new_node->get_address();
	int offset = 2 + NODE_CAPACITY / 2 * (sizeof(NodePointer) + attr_size_);
	memcpy(addr+2, address_ + offset, node_size_ - offset);
	memset(address_ + offset, -1, node_size_ - offset);
}

void Node::copy_value(char* node_addresss)
{
	memcpy(node_addresss, address_, node_size_);
	current_element_ /= 2;
}

void Node::reset()
{
	memset(address_ + 2, -1, node_size_ -2);
	current_element_ = 0;
}

int Node::compare(const char * s1, const char* s2)
{
	if (attr_type_ == -1)
	{
		if (*(int*)s1 < *(int*)s2)
			return -1;
		else if (*(int*)s1 == *(int*)(s2))
			return 0;
		else
			return 1;
	}
	else if (attr_type_ == -2)
	{
		if (*(float*)s1 < *(float*)(s2))
			return -1;
		else if (*(float*)s1 == *(float*)(s2))
			return 0;
		else
			return 1;
	}
	else
	{
		string str1(s1, attr_type_);
		string str2(s2, attr_type_);
		if (str1 < str2)
			return -1;
		else if (str1 == str2)
			return 0;
		else return 1;
	}
}

int Node::compare(const char * s1, const string& s2)
{
	if (attr_type_ == -1)
	{
		if (*(int*)s1 < stoi(s2))
			return -1;
		else if (*(int*)s1 == stoi(s2))
			return 0;
		else
			return 1;
	}
	else if (attr_type_ == -2)
	{
		if (*(float*)s1 < stof(s2))
			return -1;
		else if (*(float*)s1 == stof(s2))
			return 0;
		else
			return 1;
	}
	else
	{
		string str1(s1, attr_type_);
		if (str1 < s2)
			return -1;
		else if (str1 == s2)
			return 0;
		else return 1;
	}
}

char * Node::get_address()
{
	return address_;
}