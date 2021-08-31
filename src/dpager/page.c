#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include "page.h"

static void * map_addr;
static int file;

static struct page * front;
static struct page * back;

void page_init(int fd){
    front = NULL;
    back = NULL;
    file = fd;
    map_addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void * get_map_addr(){
    return map_addr;
}

void * register_page(struct page * page){
    if(!front){
        front = back = page;
    }else{
        back -> next = page;
        back = page;
    }

    return page -> va;
}

struct page * get_page(void * va){
    for(struct page * p = front; p != NULL; p = p -> next){
        if(p -> va == va){
            return p;
        }
    }

    return NULL;
}

struct page * remove_page(struct page * page){
    for(struct page * p = front; p != NULL; p = p -> next){
        if(p -> next == page){
            p -> next = page -> next;
            return page;
        }
    }

    return NULL;
}

void * load_page(void * va){
    if(!map_addr)
        return NULL;
    
    if(va < map_addr)
        return NULL;

    int index = ((va - map_addr) & 0xfffff000);

    struct page * page;

    if(!(page = get_page((int)map_addr + index))){
        return NULL;
    }

    mmap(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    lseek(file, page -> file_offset, SEEK_SET);
    
    read(file, page -> load_addr, page -> read_bytes);

    return page->va;
}