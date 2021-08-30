
#define PAGE_SIZE 4096

struct page{
    void * va;
    int file_offset;
    int size;
};