#include <stdio.h>
#include "record.h"
#include "page.h"
#include "main_file_manager.h"
#include "b_tree_manager.h"
#include <chrono>
#include <time.h>

struct options
{
    bool help = false;
    bool debug = false;
    char main_file_name[128] = "main_f.bin";
    char index_file_name[128] = "index_f.bin";
    int record_length = 15;
    int block_size = 250;
    int d = 5;
};

void parse_options(int argc, char** argv, options* opts);

int main(int argc, char** argv)
{
    options opts;
    parse_options(argc, argv, &opts);

    if (opts.help)
    {
        printf("Possible arguments:\n");
        printf("\t-h => help\n");
        printf("\t-d => debug (print disk operations)                                  defualt=%d\n", opts.debug);
        printf("\t-mf <string> => main file name                                       default=%s (max_str_len=127)\n", opts.main_file_name);
        printf("\t-if <string> => index file name                                      default=%s (max_str_len=127)\n", opts.index_file_name);
        printf("\t-l <int> =>  max number of fields in record,                         default=%d (integers)\n", opts.record_length);
        printf("\t-b <int> =>  block size in bytes in file (simulated disk behaviour), default=%d\n", opts.block_size);
        printf("\t-d <int> =>  d -> min (d) / max (2*d) keys per index page,           default=%d\n", opts.d);
        return 0;
    }

    printf("main_file_name: \t%s\n", opts.main_file_name);
    printf("index_file_name:\t%s\n", opts.index_file_name);
    printf("block_size:     \t%d bytes\n", opts.block_size);
    printf("record_length:  \t%d integers\n", opts.record_length);
    printf("d:              \t%d\n", opts.d);

    int mfm_disk_accesses = 0;
    int btm_disk_accesses = 0;
    main_file_manager* mfm = new main_file_manager(opts.main_file_name, opts.block_size, &mfm_disk_accesses);
    b_tree_manager* btm = new b_tree_manager(opts.index_file_name, opts.d, &btm_disk_accesses, mfm);

    auto now = std::chrono::high_resolution_clock::now();
    auto timeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    srand(timeMillis);
    char* command = new char[256];
    while (true)
    {
        int mfm_disk = mfm_disk_accesses;
        int btm_disk = btm_disk_accesses;
        printf("\n\033[0;36mcommand: \033[0m");
        scanf("%s", command);

        if (std::strcmp(command, "help") == 0 || std::strcmp(command, "h") == 0)
        {
            printf("Possible commands (and shortform):\n");
            printf("  help                      (h)\n");
            printf("  print_tree                (pt)\n");
            printf("  print_tree_full           (ptf)\n");
            printf("  print_main_file           (pmf)\n");
            printf("  print_records_ordered     (pro)\n");
            printf("  select                    (s)         search for record with specific key\n");
            printf("  insert                    (i)         insert with definied key and fields\n");
            printf("  insert_g                  (ig)        insert with random fields\n");
            printf("  insert_r                  (ir)        insert with random key and fields\n");
            printf("  delete                    (d)\n");
            printf("  reorganise?               (r)\n");
            printf("  exit                      (e)\n");
        }
        else if (std::strcmp(command, "print_tree") == 0 || std::strcmp(command, "pt") == 0)
            btm->print_tree();
        else if (std::strcmp(command, "print_tree_full") == 0 || std::strcmp(command, "ptf") == 0)
            btm->print_tree(true);
        else if (std::strcmp(command, "print_main_file") == 0 || std::strcmp(command, "pmf") == 0)
        {
            record* r = new record(0, nullptr, opts.record_length);
            mfm->print_main_file(r);
            delete r;
        }
        else if (std::strcmp(command, "print_records_ordered") == 0 || std::strcmp(command, "pro") == 0)
        {
            record* r = new record(0, nullptr, opts.record_length);
            btm->print_records_ordered(r);
            delete r;
        }
        else if (std::strcmp(command, "select") == 0 || std::strcmp(command, "s") == 0)
        {
            long long key;
            printf("   key: ");
            scanf("%lld", &key);
            record* r = new record(0, nullptr, opts.record_length);
            if (!btm->search(key, r))
            {
                printf("\033[0;31mRecord not found\033[0m");
            }
            else
                r->print(true);
            delete r;
        }
        else if (std::strcmp(command, "insert") == 0 || std::strcmp(command, "i") == 0)
        {
            long long key;
            int num_of_fields;
            printf("   key: ");
            scanf("%lld", &key);
            printf("   num of fields: ");
            scanf("%d", &num_of_fields);

            int* fields = new int[opts.record_length];

            printf("Insert %d fields of record in format: f1 f2 f3 f4...\n", num_of_fields);
            for (int i = 0; i < opts.record_length; i++)
            {
                int field;
                if (i < num_of_fields)
                {
                    scanf("%d", &field);
                    fields[i] = field;
                }
                else
                {
                    fields[i] = 0;
                }
            }
            record r = record(key, fields, opts.record_length);
            printf("Inserting: ");
            r.print(true);
            if (btm->insert(&r))
                printf("\n\033[0;32mSuccess!\033[0m");
            else
                printf("\n\033[0;31mFailed! Key already in base\033[0m");
        }
        else if (std::strcmp(command, "insert_g") == 0 || std::strcmp(command, "ig") == 0)
        {
            long long key;
            int num_of_fields;
            printf("   key: ");
            scanf("%lld", &key);
            printf("   num of fields: ");
            scanf("%d", &num_of_fields);

            int* fields = new int[opts.record_length];

            for (int i = 0; i < opts.record_length; i++)
            {
                if (i < num_of_fields)
                {
                    fields[i] = rand() % 10000;
                }
                else
                {
                    fields[i] = 0;
                }
            }
            record r = record(key, fields, opts.record_length);
            printf("Inserting: ");
            r.print(true);
            if (btm->insert(&r))
                printf("\n\033[0;32mSuccess!\033[0m");
            else
                printf("\n\033[0;31mFailed! Key already in base\033[0m");
        }
        else if (std::strcmp(command, "insert_r") == 0 || std::strcmp(command, "ir") == 0)
        {
            int num_of_fields;
            printf("   num of fields: ");
            scanf("%d", &num_of_fields);

            int* fields = new int[opts.record_length];
            long long key1 = rand();
            long long key2 = rand();
            long long key = key1*key2 % 10000;

            for (int i = 0; i < opts.record_length; i++)
            {
                if (i < num_of_fields)
                {
                    fields[i] = rand() % 10000;
                }
                else
                {
                    fields[i] = 0;
                }
            }
            record r = record(key, fields, opts.record_length);
            printf("Inserting: ");
            r.print(true);
            if (btm->insert(&r))
                printf("\n\033[0;32mSuccess!\033[0m");
            else
                printf("\n\033[0;31mFailed! Key already in base\033[0m");
        }
        else if (std::strcmp(command, "delete") == 0 || std::strcmp(command, "d") == 0)
        {
            long long key;
            int num_of_fields;
            printf("   key: ");
            scanf("%lld", &key);
            record* r = new record(0, nullptr, opts.record_length);
            int status = btm->remove(key, r, true);
            if (status == 1)
            {
                printf("\033[0;32mSuccess!\033[0m");
            }
            else
                printf("\033[0;32m%d\033[0m", status);
            delete r;
        }
        else if (std::strcmp(command, "reorganise") == 0 || std::strcmp(command, "r") == 0)
        {
            printf("\n\033[0;35mReorganization is not critical for b_tree structure, by reorganisation only disk space would be gained\033[0m\n");
        }
        else if (std::strcmp(command, "exit") == 0 || std::strcmp(command, "e") == 0)
        {
            delete command;
            delete btm;
            delete mfm;
            return 0;
        }
        else
            printf("Invalid command, try help");
            printf("\nOperation took: %d / %d  index / main  file disk accesses", btm_disk_accesses - btm_disk, mfm_disk_accesses - mfm_disk);
    }
    
    delete command;
    delete btm;
    delete mfm;
}

void parse_options(int argc, char** argv, options* opts)
{
    if (argc < 2)
    {
        printf("Default options (use -h for help)\n");
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            if (std::strcmp(argv[i], "-h") == 0)
                opts->help = true;

            else if (std::strcmp(argv[i], "-d") == 0)
                opts->debug = true;

            else if (std::strcmp(argv[i], "-mf") == 0)
                std::strcpy(opts->main_file_name, argv[i + 1]);

            else if (std::strcmp(argv[i], "-if") == 0)
                std::strcpy(opts->index_file_name, argv[i + 1]);

            else if (std::strcmp(argv[i], "-l") == 0)
                opts->record_length = std::atoi(argv[i + 1]);

            else if (std::strcmp(argv[i], "-b") == 0)
                opts->block_size = std::atoi(argv[i + 1]);

            else if (std::strcmp(argv[i], "-d") == 0)
                opts->d = std::atoi(argv[i + 1]);

        }
    }
}
