#include "b_tree_manager.h"
#include <fstream>

b_tree_manager::b_tree_manager(const char* file_name, unsigned int d, int* disk_accesses, main_file_manager* mfm)
{
	this->file_name = file_name;
	this->d = d;
	this->disk_accesses = disk_accesses;

	this->mfm = mfm;
	
	this->page_byte_size = sizeof(int) * 4 + sizeof(int) * 2 * d * 3 + sizeof(long long) * 2 * d;


	std::fstream file;
	file.open(file_name, std::ios_base::binary | std::ios_base::out | std::ios_base::app);
	file.seekg(0, file.end);
	this->file_size = file.tellg();
	printf("%lld", file_size);
	file.close();

	this->file_num_of_pages = ((int)ceil((float)file_size / (float)page_byte_size));

	this->buff_pages = new page*[10];
	for(int i=0; i<10; i++) this->buff_pages[i] = nullptr;

	this->buffed_pages_addrs = new int[10];
	for (int i = 0; i < 10; i++)	buffed_pages_addrs[i] = -1;

	if (file_num_of_pages > 0)		//load root page
		read_page(0, 0);
	else							//fresh     file / db / no root
	{
		int parent = -2;
		unsigned int d = this->d;
		unsigned int m = 0;
		int ptr0 = -1;
		int* ptr_s = new int[2 * this->d];
		int* addr_s = new int[2 * this->d];
		int* addr_s_off = new int[2 * this->d];
		long long* key_s = new long long[2 * this->d];

		for (int i = 0; i < 2 * this->d; i++)
		{
			ptr_s[i] = -1;
			addr_s[i] = -1;
			addr_s_off[i] = -1;
			key_s[i] = -1;
		}

		page* root = new page(this->d, parent, m, ptr0, ptr_s, addr_s, addr_s_off, key_s);

		write_page(root, 0);
	}
	this->last_used_addr = 0;
	this->last_depth = 0;
}

b_tree_manager::~b_tree_manager()
{
	for (int i = 0; i < 10; i++)
		if (this->buff_pages[i] != nullptr)
		{
			write_page(this->buff_pages[i], buffed_pages_addrs[i]);
			delete this->buff_pages[i];
		}

	delete buff_pages;
	delete[] buffed_pages_addrs;
}

void b_tree_manager::read_page(int p_ind, int addr)
{
	std::fstream file(this->file_name, std::ios::binary | std::ios::in);
	char* buff = new char[page_byte_size];

	file.seekg(addr * page_byte_size);
	file.read(buff, page_byte_size);
	file.close();

	int offset = 0;
	int parent;
	unsigned int d;
	unsigned int m;
	int ptr0;
	int* ptr_s = new int[2 * this->d];
	int* addr_s = new int[2 * this->d];
	int* addr_s_off = new int[2 * this->d];
	long long* key_s = new long long[2 * this->d];

	std::memcpy(&parent, buff+offset, sizeof(int));
	offset += sizeof(int);
	std::memcpy(&d, buff+offset, sizeof(unsigned int));
	offset += sizeof(unsigned int);
	std::memcpy(&m, buff + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int);
	std::memcpy(&ptr0, buff + offset, sizeof(int));
	offset += sizeof(int);
	std::memcpy(ptr_s, buff + offset, sizeof(int)*2*this->d);
	offset += sizeof(int) * 2 * this->d;
	std::memcpy(addr_s, buff + offset, sizeof(int) * 2 * this->d);
	offset += sizeof(int) * 2 * this->d;
	std::memcpy(addr_s_off, buff + offset, sizeof(int) * 2 * this->d);
	offset += sizeof(int) * 2 * this->d;
	std::memcpy(key_s, buff + offset, sizeof(long long) * 2 * this->d);

	page* p = new page(this->d, parent, m, ptr0, ptr_s, addr_s, addr_s_off, key_s);

	(*disk_accesses)++;

	delete buff_pages[p_ind];
	buff_pages[p_ind] = p;
	buffed_pages_addrs[p_ind] = addr;
}

void b_tree_manager::write_page(page* p, int addr)
{
	if (addr < 0) return;
	char* bytes = p->to_bytes();
	if (addr < file_num_of_pages)
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::in);
		file.seekp(addr * page_byte_size);
		file.write(bytes, page_byte_size);
		file.close();
	}
	else
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::app);
		file.write(bytes, page_byte_size);
		file.close();
		file_num_of_pages++;
		file_size += page_byte_size;
	}
	delete bytes;

	(*disk_accesses)++;
}

bool b_tree_manager::page_cached(int p_ind, int addr)
{
	if (buffed_pages_addrs[p_ind] == addr) return true;
	return false;
}

bool b_tree_manager::search_from(long long key, record* r, int depth, int page_addr)
{
	this->last_used_addr = page_addr;
	this->last_depth = depth;
	//load page to memory if absent
	if (!page_cached(depth, page_addr)) read_page(depth, page_addr);

	page* pg = buff_pages[depth];
	if (pg->m < 1)
	{
		int* flds = new int[r->size];
		for (int i = 0; i < r->size; i++) flds[i] = 0;
		delete[] r->fields;
		r->fields = flds;
		return false;
	}
	else
	{
		int less_than_index = 0;

		for (int i = 0; i < pg->m; i++)
		{
			less_than_index = i;
			if (key <= pg->key_s[i]) break;
		}
		int j = less_than_index;
		if (key == pg->key_s[j])
		{
			mfm->read(r, pg->addr_s[j], pg->addr_s_off[j]);
			return true;
		}
		if (key < pg->key_s[j])
		{
			if (j == 0)
			{
				if (pg->ptr0 == -1)
					return false;
				else
					return search_from(key, r, depth + 1, pg->ptr0);
			}
			else
			{
				if (pg->ptr_s[j - 1] == -1)
					return false;
				else
					return search_from(key, r, depth + 1, pg->ptr_s[j - 1]);
			}
				
		}
		if (key > pg->key_s[j])
		{
			if (pg->ptr_s[j] == -1)
				return false;
			else
				return search_from(key, r, depth + 1, pg->ptr_s[j]);
		}
	}
}

bool b_tree_manager::search(long long key, record* r)
{
	return search_from(key, r, 0, 0);
}

bool b_tree_manager::insert(record* r)
{
	if (search(r->key, r)) return false;
	
	int p_ind = last_depth;
	int addr = last_used_addr;
	//load page to memory if absent
	if (!page_cached(p_ind, addr)) read_page(p_ind, addr);

	int w_addr = 0;
	int w_offset = 0;
	mfm->write(r, &w_addr, &w_offset);

	page* pg = buff_pages[p_ind];

	if (pg->m < 2 * d)
		pg->simple_insert(r->key, w_addr, w_offset);
	else
	{
		//comensate / split
	}

	return true;
}
