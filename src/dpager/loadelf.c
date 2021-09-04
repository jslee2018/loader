#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "loadelf.h"
#include "page.h"
#include "signal.h"

static void * handle;

void * dl_init(){
    handle = dlopen("libc.so.6", RTLD_NOW);
}

void * load_elf(FILE * file, void ** dest){
    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
    fread(ehdr, sizeof(Elf32_Ehdr), 1, file);

    if(!elf_check_valid(ehdr)){
        printf("not valid elf\n");
        return NULL;
    }

    Elf32_Phdr * phdr = (Elf32_Phdr *) malloc(sizeof(Elf32_Phdr) * ehdr -> e_phnum);
    fseek(file, ehdr -> e_phoff, SEEK_SET);
    fread(phdr, sizeof(Elf32_Phdr), ehdr -> e_phnum, file);
    // lseek(file, ehdr -> e_phoff, SEEK_SET);
    // read(file, phdr, sizeof(Elf32_Phdr) * ehdr -> e_phnum);

    if(!load_segment(ehdr, phdr, dest)){
        printf("load_segment failed\n");
        return NULL;
    }

    Elf32_Shdr * shdr = (Elf32_Shdr *) malloc(sizeof(Elf32_Shdr) * ehdr -> e_shnum);
    fseek(file, ehdr -> e_shoff, SEEK_SET);
    fread(shdr, sizeof(Elf32_Shdr), ehdr -> e_shnum, file);
    // read(file, shdr, sizeof(Elf32_Shdr) * ehdr -> e_shnum);
    
    if(!relocate(file, ehdr, shdr, dest)){
        printf("relocate failed\n");
        return NULL;
    }

    return find_entry(ehdr, dest);
}

bool elf_check_valid(Elf32_Ehdr * ehdr){
    if(ehdr -> e_ident[EI_MAG0] != 0x7f || ehdr -> e_ident[EI_MAG1] != 'E' || ehdr -> e_ident[EI_MAG2] != 'L' || ehdr -> e_ident[EI_MAG3] != 'F')
        return false;

    if(ehdr -> e_ident[EI_CLASS] != ELFCLASS32)
        return false;

    if(ehdr -> e_machine != EM_386)
        return false;

    return true;
}

bool load_segment(Elf32_Ehdr * ehdr, Elf32_Phdr * phdr, void ** load_addr){

    unsigned length = 0;

    if(ehdr -> e_phentsize != sizeof(Elf32_Phdr)){
        printf("invalid phentsize\n");
        return NULL;
    }

    int i;
    for(i = 0; i < ehdr -> e_phnum; i++){
        switch (phdr[i].p_type)
        {
        case PT_LOAD:
            if(length < phdr[i].p_memsz + phdr[i].p_vaddr){
                length = phdr[i].p_memsz + phdr[i].p_vaddr;
            }
            break; 
        }
    }

    length = (length / PAGE_SIZE + (length % PAGE_SIZE != 0)) * PAGE_SIZE;

    *load_addr = get_map_addr(length);

    for(i = 0; i < ehdr -> e_phnum; i++){
        switch (phdr[i].p_type)
        {
        case PT_LOAD:
            if(phdr[i].p_filesz > phdr[i].p_memsz || !phdr[i].p_filesz){
                printf("filesz error\n");
                return false;
            }

            int total_read_bytes = phdr[i].p_filesz;
            int l_addr = phdr[i].p_vaddr;

            while(total_read_bytes > 0){
                struct page * page = malloc(sizeof(struct page));
                page -> load_addr = (int) get_map_addr(length) + l_addr;
                page -> va = (int) get_map_addr(length) + l_addr & 0xfffff000;
                page -> file_offset = phdr[i].p_offset;
                page -> read_bytes = page -> va - page -> load_addr + PAGE_SIZE;
                page -> flags = phdr[i].p_flags;

                register_page(page);
                if(!l_addr)
                    load_page(page -> va);
                total_read_bytes -= page -> read_bytes;
                l_addr += page -> read_bytes;
            }
            break; 
        }
    }

    return true;
}

Elf32_Shdr * find_section(Elf32_Ehdr * ehdr, Elf32_Shdr * shdr, unsigned type){
    for (int i = 0; i < ehdr -> e_shnum; i++)
    {
        if (shdr[i].sh_type == type)
        {
            return shdr + i;
            break;
        }
    }
    return NULL;
}

void * find_entry(Elf32_Ehdr * ehdr, void ** load_addr){
    return *load_addr + ehdr -> e_entry;
}

void * load_dl(const char* sym)
{
    if(!handle)
    {
        handle = dlopen("libc.so.6", RTLD_NOW);
    }

    if(!handle)
        return NULL;

    return dlsym(handle, sym);
}

bool relocate_section(Elf32_Rel* rel, void * dst, Elf32_Shdr * shdr, Elf32_Sym * syms, void * strings){
    for(int j = 0; j < shdr->sh_size / sizeof(Elf32_Rel); j++){
        char * sym = strings + syms[ELF32_R_SYM(rel[j].r_info)].st_name;
        switch (ELF32_R_TYPE(rel[j].r_info)){
            case R_386_RELATIVE:
                *(Elf32_Word*)(dst + rel[j].r_offset) += (Elf32_Word) dst;
                break;
            case R_386_JMP_SLOT:
            case R_386_GLOB_DAT:
                *(Elf32_Word*)(dst + rel[j].r_offset) = (Elf32_Word)load_dl(sym);
                break;
        }
    }
    return true;
}

bool relocate(FILE * file, Elf32_Ehdr * ehdr, Elf32_Shdr * shdr, void ** load_addr){
    if(ehdr -> e_shentsize != sizeof(Elf32_Shdr)){
        printf("invalid shentsize\n");
        return NULL;
    }

    Elf32_Shdr * dynsym = find_section(ehdr, shdr, SHT_DYNSYM);

    Elf32_Sym * syms = (Elf32_Sym *) malloc(dynsym -> sh_size);
    fseek(file, dynsym -> sh_offset, SEEK_SET);
    fread(syms, 1, dynsym -> sh_size, file);
    // read(file, syms, dynsym -> sh_size);

    char * strings = malloc(shdr[dynsym -> sh_link].sh_size);
    fseek(file, shdr[dynsym -> sh_link].sh_offset, SEEK_SET);
    fread(strings, 1, shdr[dynsym -> sh_link].sh_size, file);
    // read(file, strings, shdr[dynsym -> sh_link].sh_size);

    int i;
    for(i = 0; i < ehdr -> e_shentsize; i++){
        if(shdr[i].sh_type == SHT_REL){
            Elf32_Rel* rel = (Elf32_Rel*) malloc(shdr[i].sh_size);
            fseek(file, shdr[i].sh_offset, SEEK_SET);
            fread(rel, 1, shdr[i].sh_size, file);

            if(!relocate_section(rel, *load_addr, shdr + i, syms, strings))
                return false;
        }
    }

    return true;
}