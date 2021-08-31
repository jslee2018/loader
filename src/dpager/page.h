
#define PAGE_SIZE 4096

struct page{
    void * va;
    void * load_addr;
    int file_offset;
    int size;
    struct page * next;
};

void page_init(int fd);
void * get_map_addr(void);