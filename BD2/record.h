#pragma once
class record
{
public:
	record(long long key, int* fields, int r_len);
	~record();
	int byte_size;
	int size;
	int* fields;
	unsigned char* to_bytes();
	void print();
	void print(bool full);
	long long key;
	unsigned char flags = 1;
	bool exists();
	bool last();
};

/* FLAGS
	bit(0) - exists:	1 - true;			0 - deleted
	bit(1) - not last:  1 - last;			0 - not last
	...
*/