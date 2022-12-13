#include "main_file_manager.h"
#include <fstream>

main_file_manager::main_file_manager(const char* file_name, int block_size, int* disk_accesses)
{
	this->file_name = file_name;
	this->block_size = block_size;
	this->disk_accesses = disk_accesses;
	
	this->block = new char[block_size];
	
	std::fstream file;
	file.open(file_name, std::ios_base::binary | std::ios_base::out | std::ios_base::app);
	file.seekg(0, file.end);
	this->file_size = file.tellg();
	//printf("%lld", file_size);
	file.close();

	this->file_num_of_blocks = ((int)ceil((float)file_size / (float)block_size));
	this->block_cached_addr = -1;
	this->record_size = 69;
	//this->record_size = record_size(60) + byte_of_flags(1) + longlong_key(8);
}

main_file_manager::~main_file_manager()
{
	write_block_to_file(block_cached_addr);
	delete block;
}

void main_file_manager::read_block_from_file(int block_addr)
{
	if (block_addr == block_cached_addr) return;
	std::fstream file(this->file_name, std::ios::binary | std::ios::in);
	file.seekg(block_addr * block_size);
	file.read(block, block_size);
	(*disk_accesses)++;
	//printf("\nPULL");
	//print_block();
	file.close();

	block_cached_addr = block_addr;
}

void main_file_manager::print_block()
{
	printf("\n");
	for (int i = 0; i < block_size; i++)
	{
		printf("%02x ", block[i]);
	}
}

void main_file_manager::write_block_to_file(int block_addr)
{
	if (block_addr < 0) return;
	if (block_addr < file_num_of_blocks)
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::in);
		file.seekp(block_addr*block_size);
		file.write(block, block_size);
		file.close();
	}
	else
	{
		std::fstream file(this->file_name, std::ios::binary | std::ios::out | std::ios::app);
		file.write(block, block_size);
		file.close();
		file_num_of_blocks++;
		file_size += block_size;
	}

	(*disk_accesses)++;
}

bool main_file_manager::read(record* r, int block_addr, int record_pos_in_block)
{
	if (block_addr > file_num_of_blocks-1) return false;
	int record_offset = record_pos_in_block * r->byte_size;
	if (block_addr != block_cached_addr)
	{
		write_block_to_file(block_cached_addr);
		read_block_from_file(block_addr);
	}

	int* flds = new int[r->size];
	std::memcpy(&(r->key), &(block[record_offset]), sizeof(long long));
	std::memcpy(&(r->flags), &(block[record_offset + sizeof(long long)]), sizeof(char));
	std::memcpy((void*)flds, &(block[record_offset + sizeof(long long) + sizeof(char)]), (r->size) * sizeof(int));
	r->fields = flds;
	return true;
}

void main_file_manager::remove_record(record* r, int block_addr, int record_pos_in_block)
{
	//just changes bit of existance of the record to 0
	read_block_from_file(block_addr);
	int record_offset = record_pos_in_block * r->byte_size;
	block[record_offset + sizeof(long long)] = block[record_offset + sizeof(long long)] & 0b11111110;
}

void main_file_manager::write(record* r, int* addr_writen, int* off_writen)
{
	if (file_num_of_blocks == 0)
	{
		for (int i = 0; i < block_size; i++)
		{
			block[i] = 0;
		}
		r->flags = 0b00000011;										//last and exists
		unsigned char* bytes = r->to_bytes();
		for (int i = 0; i < r->byte_size; i++)
		{
			block[i] = bytes[i];
		}
		write_block_to_file(0);
		block_cached_addr = 0;
		return;
	}

	int records_per_block = block_size / r->byte_size;
	if (block_cached_addr != file_num_of_blocks-1) read_block_from_file(file_num_of_blocks-1);	//load last block
	int last_r_index = 0;											//index relative to block (not in bytes)
	record* tmp_r = new record(-1, nullptr, r->size);
	read(tmp_r, file_num_of_blocks-1, last_r_index);
	while (!tmp_r->last())
	{
		last_r_index++;
		read(tmp_r, file_num_of_blocks - 1, last_r_index);
	}
	delete tmp_r;

	char last_r_flags_byte = block[last_r_index * r->byte_size + sizeof(long long)];
	last_r_flags_byte = last_r_flags_byte & 0b11111101;				//last is not last anymore
	block[last_r_index * r->byte_size + sizeof(long long)] = last_r_flags_byte;

	r->flags = 0b00000011;
	unsigned char* bytes = r->to_bytes();

	if ((last_r_index + 1) * r->byte_size <= block_size - r->byte_size)	//still place to write to this block
	{
		for (int i = 0; i < r->byte_size; i++) block[(last_r_index + 1) * r->byte_size + i] = bytes[i];
		*addr_writen = file_num_of_blocks - 1;
		*off_writen = last_r_index + 1;
	}
	else
	{
		write_block_to_file(file_num_of_blocks - 1);
		for (int i = 0; i < block_size; i++) block[i] = 0;
		for (int i = 0; i < r->byte_size; i++) block[i] = bytes[i];
		block_cached_addr = file_num_of_blocks;
		write_block_to_file(file_num_of_blocks);
		*addr_writen = file_num_of_blocks - 1;
		*off_writen = 0;
	}
}

void main_file_manager::write_at(record* r, int block_addr, int record_pos_in_block)
{

	/*int record_offset = record_pos_in_block * r->byte_size;
	if (block_addr != block_cached_addr) read_block_from_file(block_addr);

	int* flds = new int[r->size];
	std::memcpy(&(r->key), &(block[record_offset]), sizeof(long long));
	std::memcpy(&(r->flags), &(block[record_offset + sizeof(long long)]), sizeof(char));
	std::memcpy((void*)flds, &(block[record_offset + sizeof(long long) + sizeof(char)]), (r->size) * sizeof(int));
	r->fields = flds;*/
}

void main_file_manager::print_main_file(const record* record_template)
{
	printf("\n========= MAIN FILE:\n");
	printf("(block, offset) | KEY | FLAGS | FIELDS\n");
	for (int i = 0; i < file_num_of_blocks; i++)
	{
		for (int j = 0; j < int(block_size/record_template->byte_size); j++)
		{
			record* r = new record(0, nullptr, record_template->size);
			read(r, i, j);
			printf(" \033[0;32m(%d, %d)\033[0m: ", i, j);
			r->print(true);
			printf("\n");
			if (r->last())
			{
				delete r;
				break;
			}
			delete r;
		}
	}
}