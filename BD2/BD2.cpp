#include <stdio.h>
#include "record.h"
#include "page.h"
#include "main_file_manager.h"
#include "b_tree_manager.h"
#include <chrono>
#include <time.h>

int main()
{
    /*int* fields = new int[2];
    fields[0] = 9;
    fields[1] = 9;
    record r = record(70, fields, 2);
    int disk_accesses = 0;
    main_file_manager* mfm = new main_file_manager("main_file_manager_test.bin", 1000, &disk_accesses);
    mfm->write(&r);
    for (int i = 9; i > 0; i--)
    {
        mfm->read(&r, 0, i);
        printf("\n");
        r.print(true);
    }
    delete mfm;
    printf("\nDISK ACCESSES: %d", disk_accesses);*/
    const int thisd = 5;
    int disk_accesses = 0;
    main_file_manager* mfm = new main_file_manager("main_file_manager_test.bin", (6*4+8+1)*10, &disk_accesses);
    b_tree_manager* btm = new b_tree_manager("b_tree_manager_test.bin", thisd, &disk_accesses, mfm);
	
    auto now = std::chrono::high_resolution_clock::now();
    auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    //srand(timeMillis);
    srand(time(NULL));

    int record_length = 10;
    int* fields = new int[record_length];
    int rec_rand = rand() % 6 + abs(record_length - 6);
    long long key1 = rand();
    long long key2 = rand();
    //long long key = key1*key2;
    long long key = 14;
    for (int i = 0; i < record_length; i++)
    {
        if (i <= rec_rand)
        {
            fields[i] = rand() % 10000;
        }
        else
        {
            fields[i] = 0;
        }
    }
    record r = record(key, fields, record_length);
    btm->insert(&r);

	/*int parent = -1;
	unsigned int d = thisd;
	unsigned int m = 2;
	int ptr0 = -1;
    int ptr_s[2*thisd] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    int addr_s[2 * thisd] = { 0,0,-1,-1,-1,-1,-1,-1,-1,-1 };
    int addr_s_off[2 * thisd] = { 0,1,-1,-1,-1,-1,-1,-1,-1,-1 };
    long long key_s[2 * thisd] = { 4,5,-1,-1,-1,-1,-1,-1,-1,-1 };

	page* p = new page(thisd, parent, m, ptr0, ptr_s, addr_s, addr_s_off, key_s);

    btm->write_page(p, 0);
    record r(7, nullptr, 15);
    printf("\nsearch 7: %d", btm->search(5, &r));*/
    /*btm->read_page(0, 0);
    btm->read_page(1, 1);
    btm->read_page(2, 2);
    btm->read_page(3, 0);

    btm->buff_pages[0]->print(0);
    btm->buff_pages[1]->print(2);
    btm->buff_pages[2]->print(4);
    btm->buff_pages[3]->print(6);*/
    delete btm;
    delete mfm;
    return 0;
}