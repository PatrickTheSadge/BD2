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
	int searched_should_be_at_in_page;
	int searched_should_be_at_addr;
	int last_depth;
	bool root_changed = false;

	page** buff_pages;
	int* buffed_pages_addrs;

	main_file_manager* mfm;

	bool compensate(int cache_ind);
	void split(int cache_ind, long long key, int r_addr, int r_addr_off, int left_addr, int right_addr);
	void merge(int cache_ind, int left_chld, int right_chld);
	bool page_cached(int p_ind, int addr);
	bool search_from(long long key, record* r, int depth, int page_addr);
	void read_page(int p_ind, int addr);
	void write_page(page* p, int addr, int* saved_at);
	void write_page(page* p, int addr);
	void print_tree(page* p, int p_addr, int depth);
	void print_tree_full(page* p, int p_addr, int depth);
	void insert_into(int cached_ind, long long r_key, int r_addr, int r_addr_off, int left_addr, int right_addr);	//move record to specific page
	void print_records_ordered(page* p, int p_addr, int depth, record* r);
	void reload_page(int addr);
public:

	b_tree_manager(const char* file_name, unsigned int d, int* disk_accesses, main_file_manager* mfm);
	~b_tree_manager();
	bool search(long long key, record* r);
	bool insert(record* r);
	
	void print_records_ordered(record* record_template);
	
	void print_tree(bool full);
	void print_tree();
	int remove(long long key, record* record_template, bool remove_from_main_file);
	//bool update();
};