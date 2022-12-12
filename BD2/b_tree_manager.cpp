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
	//printf("%lld", file_size);
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
	this->searched_should_be_at_addr = 0;
	this->searched_should_be_at_in_page = 0;
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
	if (buffed_pages_addrs[p_ind] == addr) return;
	if (buffed_pages_addrs[p_ind] > -1)
	{
		write_page(buff_pages[p_ind], buffed_pages_addrs[p_ind]);
	}

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

void b_tree_manager::write_page(page* p, int addr, int* saved_at)
{
	if (addr < 0) return;
	char* bytes = p->to_bytes();
	if (addr < file_num_of_pages)
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::in);
		file.seekp(addr * page_byte_size);
		file.write(bytes, page_byte_size);
		file.close();
		*saved_at = addr;
	}
	else
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::app);
		file.write(bytes, page_byte_size);
		file.close();
		*saved_at = file_num_of_pages;
		file_num_of_pages++;
		file_size += page_byte_size;
	}
	delete bytes;

	(*disk_accesses)++;
}

void b_tree_manager::write_page(page* p, int addr)
{
	int tmp;
	write_page(p, addr, &tmp);
}

bool b_tree_manager::page_cached(int p_ind, int addr)
{
	if (buffed_pages_addrs[p_ind] == addr) return true;
	return false;
}

bool b_tree_manager::search_from(long long key, record* r, int depth, int page_addr)
{
	this->searched_should_be_at_addr = page_addr;
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
		this->searched_should_be_at_in_page = j;
		
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
			this->searched_should_be_at_in_page++;
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
	int addr = searched_should_be_at_addr;
	//load page to memory if absent
	if (!page_cached(p_ind, addr)) read_page(p_ind, addr);

	int w_addr = 0;
	int w_offset = 0;
	mfm->write(r, &w_addr, &w_offset);

	page* pg = buff_pages[p_ind];

	if (pg->m < 2 * d)
		pg->simple_insert(r->key, w_addr, w_offset, -1, -1);
	else
	{
		if (!compensate(p_ind))
			split(p_ind, r->key, w_addr, w_offset, -1, -1);
		insert(r);
	}

	return true;
}

void b_tree_manager::insert_into(int cached_ind, long long r_key, int r_addr, int r_addr_off, int left_addr, int right_addr)
{
	page* pg = buff_pages[cached_ind];

	if (pg->m < 2 * d)
		pg->simple_insert(r_key, r_addr, r_addr_off, left_addr, right_addr);
	else
	{
		if (cached_ind > 0)
		{
			if (!compensate(cached_ind))
				split(cached_ind, r_key, r_addr, r_addr_off, left_addr, right_addr);
		}
		else
		{
			int old_root_new_addr;


			// this is root and has no more space
			
			// save old root -> have new address of old root
			page* old_root = buff_pages[0];
			write_page(old_root, file_num_of_pages, &old_root_new_addr);

			// update all childs of old root -> change their parent address to new address of old root
			for(int i = 0; i < old_root->m; i++)
			{
				read_page(9, old_root->ptr_s[i]);
				buff_pages[9]->parent = old_root_new_addr;
			}
			// update old root -> change it's parent to 0
			// save new root -> at 0


			// make new root
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

			
			

			// refresh buffered pages?



			if (!compensate(old_root_new_addr))
				split(old_root_new_addr, r_key, r_addr, r_addr_off, left_addr, right_addr);
		}
	}
}

void b_tree_manager::split(int cache_ind, long long key, int r_addr, int r_addr_off, int left_addr, int right_addr)
{
	//if (cache_ind > 0)
	//{
		int m_key, m_addr, m_addr_off, m_index;
		page* p_split = buff_pages[cache_ind];
		m_index = p_split->give_median(&m_key, &m_addr, &m_addr_off);

		int parent = p_split->parent;
		unsigned int d = this->d;
		unsigned int m = 0;
		int ptr0 = p_split->ptr_s[m_index];

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

		for (int i = m_index + 1, int j=0; i < p_split->m; i++, j++)
		{
			m += 1;
			ptr_s[j] = p_split->ptr_s[i];
			addr_s[j] = p_split->addr_s[i];
			addr_s_off[j] = p_split->addr_s_off[i];
			key_s[j] = p_split->key_s[i];
		}

		page* p = new page(this->d, parent, m, ptr0, ptr_s, addr_s, addr_s_off, key_s);

		int saved_page_addr;
		write_page(p, 0, &saved_page_addr);

		for (int i = m_index; i < p_split->m; i++)
		{
			p_split->ptr_s[i] = -1;
			p_split->addr_s[i] = -1;
			p_split->addr_s_off[i] = -1;
			p_split->key_s[i] = -1;
		}
		p_split->m = p_split->m / 2;
		insert_into(cache_ind - 1, m_key, m_addr, m_addr_off, buffed_pages_addrs[cache_ind], saved_page_addr);
	//}
}

bool b_tree_manager::compensate(int cache_ind)
{
	/*int this_addr = buffed_pages_addrs[cache_ind];
	int parent_addr = buffed_pages_addrs[cache_ind - 1];
	
	int left_bro_addr = buff_pages[cache_ind-1]->left_brother(this_addr);
	int right_bro_addr = buff_pages[cache_ind-1]->right_brother(this_addr);

	
	if (left_bro_addr >= 0)
	{
		read_page(9, left_bro_addr);
		if ((buff_pages[9]->m) < 2 * d)
		{
			
			return true;
		}
	}

	if (right_bro_addr >= 0)
	{
		read_page(9, right_bro_addr);
		if ((buff_pages[9]->m) < 2 * d)
		{
			
			return true;
		}
	}*/
	
	return false;
}

void b_tree_manager::print_tree(page* p, int p_addr, int depth)
{
	if (!page_cached(depth, p_addr)) read_page(depth, p_addr);
	p->print(depth);

	if (p->ptr0 >= 0)
	{
		read_page(depth + 1, p->ptr0);
		print_tree(buff_pages[depth+1], buffed_pages_addrs[depth+1], depth+1);
	}
	for (int i = 0; i < p->m; i++)
	{
		if (p->ptr_s[i] >= 0)
		{
			read_page(depth + 1, p->ptr_s[i]);
			print_tree(buff_pages[depth + 1], buffed_pages_addrs[depth + 1], depth + 1);
		}
	}
}

void b_tree_manager::print_tree_full(page* p, int p_addr, int depth)
{
	if (!page_cached(depth, p_addr)) read_page(depth, p_addr);
	p->print_full(depth);

	if (p->ptr0 >= 0)
	{
		read_page(depth + 1, p->ptr0);
		print_tree(buff_pages[depth + 1], buffed_pages_addrs[depth + 1], depth + 1);
	}
	for (int i = 0; i < p->m; i++)
	{
		if (p->ptr_s[i] >= 0)
		{
			read_page(depth + 1, p->ptr_s[i]);
			print_tree(buff_pages[depth + 1], buffed_pages_addrs[depth + 1], depth + 1);
		}
	}
}

void b_tree_manager::print_tree()
{
	print_tree(buff_pages[0], buffed_pages_addrs[0], 0);
}

void b_tree_manager::print_tree(bool full)
{
	if(full)
		print_tree_full(buff_pages[0], buffed_pages_addrs[0], 0);
	else
		print_tree(buff_pages[0], buffed_pages_addrs[0], 0);
}