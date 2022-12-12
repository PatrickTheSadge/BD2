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
	int left_brother(int addr);
	int right_brother(int addr);
	int give_median(long long* m_key, int* m_addr, int* m_addr_off);
};