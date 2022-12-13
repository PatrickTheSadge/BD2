#pragma once
#include <fstream>

class page
{
public:
	int parent;
	unsigned int d;
	unsigned int m;
	int ptr0;
	int* ptr_s;
	int* addr_s;
	int* addr_s_off;
	long long* key_s;
	int byte_size;

	//page();
	page(unsigned int d, int parent, unsigned int m, int ptr0, int* ptr_s, int* addr_s, int* addr_s_off, long long* key_s);
	~page();

	char* to_bytes();
	void print_full(int indent);
	void print(int indent);
	void simple_insert(long long key, int addr, int addr_off, int left_addr, int right_addr);
	void simple_remove(long long key, int* rem_addr, int* rem_off);
	int left_brother(int addr);			//returns addr
	int left_brother_by_key(long long key);	//returns addr
	int right_brother_by_key(long long key);	//returns addr
	int right_brother(int addr);		//returns addr
	int give_median(long long* m_key, int* m_addr, int* m_addr_off);
	bool is_leaf();
	long long biggest_key(int* addr, int* off);
	long long smallest_key(int* addr, int* off);
	void address_of_key(long long key, int* addr, int* offset);
};