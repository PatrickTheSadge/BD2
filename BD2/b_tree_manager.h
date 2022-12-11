#pragma once
#include "page.h"
#include "record.h"
#include "main_file_manager.h"

class b_tree_manager
{
private:
	const char* file_name;
	int* disk_accesses;
	long long file_size;
	int file_num_of_pages;
	unsigned int d;
	int page_byte_size;
	int last_used_addr;
	int last_depth;

	//page** buff_pages;
	int* buffed_pages_addrs;

	main_file_manager* mfm;

	//void compensation(int addr);
	//void split(int addr);
	//void merge(int addr);
	//void get_next_record(????);
	bool page_cached(int p_ind, int addr);
	bool search_from(long long key, record* r, int depth, int page_addr);
public:

	page** buff_pages;

	void read_page(int p_ind, int addr);
	void write_page(page* p, int addr);

	b_tree_manager(const char* file_name, unsigned int d, int* disk_accesses, main_file_manager* mfm);
	~b_tree_manager();
	bool search(long long key, record* r);
	bool insert(record* r);
	//print_tree();
	//delete(long long key);
	//void read_entire_tree(???);
};