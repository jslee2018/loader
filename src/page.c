#include <sys/mman.h>
#include "page.h"

static void * map_addr;

void page_init(){
    map_addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void * get_map_addr(){
    if(!map_addr)
        page_init();

    return map_addr;
}

void * register_page(struct page * page){
    return NULL;
}

struct page * get_page(void * va){
    return NULL;
}

void * load_page(void * va){
    if(!map_addr)
        return NULL;
    
    if(va < map_addr)
        return NULL;

    int index = ((va - map_addr) & 0xfffff000);

    struct page * page;

    if(!(page = get_page)){
        return NULL;
    }

    mmap(page->va, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    return page->va;
}