
#define PAGE_SIZE 4096

struct page{
    void * va;
    int file_offset;
    int size;
};

void page_init(void);
void * get_map_addr(void);