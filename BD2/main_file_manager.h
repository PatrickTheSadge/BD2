#pragma once
#include "record.h"

class main_file_manager
{
private:
	const char* file_name;
	int block_size;
	char* block;
	int* disk_accesses;
	long long file_size;
	int file_num_of_blocks;
	
	int block_cached_addr;		//address of the block that is cached in memory
	int record_size;			//size of record in bytes

	void print_block();
	void read_block_from_file(int block_addr);
	void write_block_to_file(int block_addr);
public:
	main_file_manager(const char* file_name, int block_size, int* disk_accesses);
	~main_file_manager();
	bool read(record* r, int block_addr, int record_pos_in_block);
	void write(record* r, int* addr_writen, int* off_writen);
	void write_at(record* r, int block_addr, int record_pos_in_block);
};

