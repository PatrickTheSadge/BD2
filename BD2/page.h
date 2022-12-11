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
	void print(int indent);
	void simple_insert(long long key, int addr, int addr_off);
};