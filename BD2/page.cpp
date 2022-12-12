#include "page.h"

/*page::page()
{
	parent = -1;
	m = 0;
	ptr0 = -1;
	ptr_s = nullptr;
	addr_s = nullptr;
	addr_s_off = nullptr;
	key_s = nullptr;
	byte_size = sizeof(page);
	this->d = -1;
}*/

page::page(unsigned int d, int parent, unsigned int m, int ptr0, int* ptr_s, int* addr_s, int* addr_s_off, long long* key_s)
{
	this->parent = parent;
	this->d = d;
	this->m = m;
	this->ptr0 = ptr0;
	this->ptr_s = ptr_s;
	this->addr_s = addr_s;
	this->addr_s_off = addr_s_off;
	this->key_s = key_s;
	this->byte_size = sizeof(int)*4 + sizeof(int)*2*d*3 + sizeof(long long)*2*d;
	//this->byte_size = sizeof(page);
}

page::~page()
{
	delete ptr_s;
	delete addr_s;
	delete addr_s_off;
	delete key_s;
}

char* page::to_bytes()
{
	char* bytes = new char[byte_size];
	int offset = 0;
	std::memcpy((void*)(bytes + offset), this, sizeof(int)*4);
	offset += sizeof(int)*4;
	std::memcpy((void*)(bytes + offset), ptr_s, sizeof(int)*2*d);
	offset += sizeof(int)*2*d;
	std::memcpy((void*)(bytes + offset), addr_s, sizeof(int)*2*d);
	offset += sizeof(int) * 2 * d;
	std::memcpy((void*)(bytes + offset), addr_s_off, sizeof(int) * 2 * d);
	offset += sizeof(int) * 2 * d;
	std::memcpy((void*)(bytes + offset), key_s, sizeof(long long) * 2 * d);
	return bytes;
}

void page::print_full(int indent)
{
	printf("\n");
	for (int i = 0; i < indent; i++) printf(" ");
	//printf("|\033[0;37m^%d m:%d ", parent, m);
	printf("|\033[0;37mm:%d ", m);
	printf("\033[0;33m&%d \033[0m", ptr0);
	for (int i = 0; i < m; i++) printf("[\033[0;36m%I64d \033[0;32m(%d;%d)\033[0m] \033[0;33m&%d \033[0m", key_s[i], addr_s[i], addr_s_off[i], ptr_s[i]);
}

void page::print(int indent)
{
	printf("\n");
	for (int i = 0; i < indent; i++) printf(" ");
	//printf("^%d ", parent);
	printf("|.");
	for (int i = 0; i < m; i++) printf("[%I64d].", key_s[i]);
}

void page::simple_insert(long long key, int addr, int addr_off, int left_addr, int right_addr)
{
	if (m >= 2 * d) return;

	int j = 0;

	for (int i = 0; i < m; i++)
	{
		j = i;
		if (key <= key_s[i]) break;
	}
	if (key > key_s[j])		//last
	{
		j++;
	}
	if (m < 1) j = 0;

	for (int i = m; i > j; i--)		//shift biggers
	{
		key_s[i] = key_s[i - 1];
		ptr_s[i] = ptr_s[i - 1];
		addr_s[i] = addr_s[i - 1];
		addr_s_off[i] = addr_s_off[i - 1];
	}

	if (j == 0)
	{
		ptr_s[0] = ptr0;
		ptr0 = left_addr;
	}
	else
		ptr_s[j-1] = left_addr;
	ptr_s[j] = right_addr;

	printf("\nsi: %d", j);
	key_s[j] = key;					//insert new at index
	addr_s[j] = addr;
	addr_s_off[j] = addr_off;
	
	m++;
}

int page::left_brother(int addr)
{
	int left_ind = -1;
	int left = -1;
	for (int i = 0; i < m; i++)
	{
		if (addr_s[i] == addr)
		{
			left_ind = i-1;
			break;
		}
	}
	if (left_ind >= 0)
		left = addr_s[left_ind];
	return left;
}

int page::right_brother(int addr)
{
	int right_ind = -1;
	int right = -1;
	for (int i = 0; i < m; i++)
	{
		if (addr_s[i] == addr)
		{
			right_ind = i + 1;
			break;
		}
	}
	if (right_ind < m)
		right = addr_s[right_ind];
	return right;
}

int page::give_median(long long* m_key, int* m_addr, int* m_addr_off)
{
	int index = m / 2;
	*m_key = key_s[index];
	*m_addr = addr_s[index];
	*m_addr_off = addr_s_off[index];
	return index;
}