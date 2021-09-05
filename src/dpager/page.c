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

void register_rel(void * addr, void * val, bool add){
    struct rel * rel = malloc(sizeof(struct rel));

    rel -> va = addr;
    rel -> val = val;
    rel -> add = add;
    rel -> next = NULL;

    struct page * page = get_page((unsigned) addr & 0xfffff000);

    if(!page){
        printf("page null\n");
        exit(-1);
    }

    if(!page -> rel_front){
        page -> rel_front = page -> rel_back = rel;
    }else{
        page -> rel_back -> next = rel;
        page -> rel_back = rel;
    }
}

void load_rel(struct page * page){
    struct rel * probe = page -> rel_front;

    while(probe != NULL){
        if(probe -> add){
            *(Elf32_Word *) (probe -> va) += (Elf32_Word) (probe -> val);
        }else{
            *(Elf32_Word *) (probe -> va) = (Elf32_Word) (probe -> val);
        }
        struct rel * temp = probe;
        probe = probe -> next;

        free(temp);
    }

}

void page_init(FILE * fd){
    front = NULL;
    back = NULL;
    file = fd;
    map_addr = NULL;
}

void * get_map_addr(unsigned length){
    if(!map_addr){
        map_addr = mmap(NULL, length + PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        mprotect(map_addr, length, PROT_NONE);
        mprotect(map_addr + length, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    }

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
    if(va < get_map_addr(NULL)){
        return NULL;
    }

    int index = ((va - map_addr) & 0xfffff000);

    struct page * page;

    void * mmap_addr = 1;

    if(!(page = get_page((int)map_addr + index))){
        return NULL;
    }

    mprotect(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE);

    fseek(file, page -> file_offset, SEEK_SET);
    fread(page -> load_addr, 1, page -> read_bytes, file);

    if (!(page -> flags & PF_W))
        mprotect(page -> va, PAGE_SIZE, PROT_READ | PROT_WRITE);

    if (page -> flags & PF_X)
        mprotect(page -> va, PAGE_SIZE, PROT_EXEC | PROT_WRITE);

    load_rel(page);

    return page->va;
}