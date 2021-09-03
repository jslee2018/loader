#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include "page.h"
#include "loadelf.h"

static void * map_addr;
static FILE * file;

static struct page * front;
static struct page * back;

void page_init(FILE * fd){
    front = NULL;
    back = NULL;
    file = fd;
    map_addr = NULL;
    printf("page init %x\n", map_addr);
}

void * get_map_addr(){
    if(!map_addr)
        map_addr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    return map_addr;
}

void * register_page(struct page * page){
    if(!front){
        front = back = page;
    }else{
        back -> next = page;
        back = page;
    }

    mprotect(page -> va, PAGE_SIZE, PROT_WRITE);

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
    if(va < get_map_addr()){
        return NULL;
    }

    int index = ((va - map_addr) & 0xfffff000);

    struct page * page;

    void * mmap_addr = 1;

    if(!(page = get_page((int)map_addr + index))){
        return NULL;
    }

    // printf("load page %x\n", page -> va);

    mprotect(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE);

    if(page -> va != map_addr)
        mmap_addr = mmap(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED, 0, 0);

    fseek(file, page -> file_offset, SEEK_SET);
    fread(page -> load_addr, 1, page -> read_bytes, file);
    
    // read(file, page -> load_addr, page -> read_bytes);

    if (!(page -> flags & PF_W))
        mprotect(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE);

    if (page -> flags & PF_X)
        mprotect(page -> va, PAGE_SIZE, PROT_EXEC | PROT_WRITE);

    if(mmap_addr == -1){
        printf("load failed\n");
    }

    // printf("load done\n");

    return page->va;
}