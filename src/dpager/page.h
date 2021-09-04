
#define PAGE_SIZE 4096

#define true 1
#define false 0

typedef char bool;

struct rel{
    void * va;
    bool add;
    void * val;
    struct rel * next;
};

struct page{
    void * va;
    void * load_addr;
    int file_offset;
    int read_bytes;
    unsigned flags;
    struct rel * rel_front;
    struct rel * rel_back;
    struct page * next;
};



void page_init(FILE * fd);
void * load_page(void * va);
void * get_map_addr(unsigned length);
void * register_page(struct page * page);
struct page * get_page(void * va);

void register_rel(void * addr, void * val, bool add);
void load_rel(struct page * page);