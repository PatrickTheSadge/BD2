#include "record.h"
#include <stdio.h>
#include <fstream>

record::record(long long key, int* fields, int r_len)
{
	this->fields = fields;
	this->size = r_len;
	this->key = key;
	this->byte_size = r_len * sizeof(int) + sizeof(long long) + sizeof(char);
}

record::~record()
{
	delete fields;
}

unsigned char* record::to_bytes()
{
	unsigned char* bytes = new unsigned char[byte_size];
	std::memcpy((void*)bytes, &key, sizeof(long long));
	std::memcpy((void*)(bytes+sizeof(long long)), &flags, sizeof(char));
	std::memcpy((void*)(bytes + sizeof(long long) + sizeof(char)), fields, size*(sizeof(int)));
	return bytes;
}

void record::print()
{
	printf("[%I64d] ", this->key);
}

void record::print(bool full)
{
	if (full)
	{
		printf("[%I64d] \[%d\] ( ", this->key, (int)this->flags);
		for (int i = 0; i < size; i++)
		{
			printf("%d ", fields[i]);
		}
		printf(")");
	}
	else print();
}

bool record::exists()
{
	return bool(flags & 0b00000001);
}

bool record::last()
{
	return bool(flags & 0b00000010);
}