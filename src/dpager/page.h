
#define PAGE_SIZE 4096

struct page{
    void * va;
    void * load_addr;
    int file_offset;
    int read_bytes;
    struct page * next;
};

void page_init(int fd);
void * load_page(void * va);
void * get_map_addr(void);
void * register_page(struct page * page);