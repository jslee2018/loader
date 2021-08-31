#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "loadelf.h"

static void * handle;

void * load_elf(void * src, void ** dest){
    if(!elf_check_valid(src)){
        printf("not valid elf\n");
        return NULL;
    }

    if(!load_segment(src, dest)){
        printf("load_segment failed\n");
        return NULL;
    }
    

    if(!relocate(src, dest)){
        printf("relocate failed\n");
        return NULL;
    }

    return find_entry(src, dest);
}

bool elf_check_valid(void * src){
    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    if(ehdr -> e_ident[EI_MAG0] != 0x7f || ehdr -> e_ident[EI_MAG1] != 'E' || ehdr -> e_ident[EI_MAG2] != 'L' || ehdr -> e_ident[EI_MAG3] != 'F')
        return false;

    if(ehdr -> e_ident[EI_CLASS] != ELFCLASS32)
        return false;

    if(ehdr -> e_machine != EM_386)
        return false;

    return true;
}

bool load_segment(void * src, void ** load_addr){

    *load_addr = mmap(NULL, 1048576 * sizeof(char), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    memset(*load_addr, 0, 1048576 * sizeof(char));

    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    Elf32_Phdr * phdr = (Elf32_Phdr *) (src + ehdr -> e_phoff);

    if(ehdr -> e_phentsize != sizeof(Elf32_Phdr)){
        printf("invalid phentsize\n");
        return NULL;
    }

    int i;
    for(i = 0; i < ehdr -> e_phnum; i++){
        switch (phdr[i].p_type)
        {
        case PT_LOAD:
            if(phdr[i].p_filesz > phdr[i].p_memsz || !phdr[i].p_filesz){
                printf("filesz error\n");
                return false;
            }

            memcpy(*load_addr + phdr[i].p_vaddr, src + phdr[i].p_offset, phdr[i].p_filesz);

            if (!(phdr[i].p_flags & PF_W))
            {
                mprotect((unsigned char *) *load_addr + phdr[i].p_vaddr, phdr[i].p_memsz, PROT_READ | PROT_WRITE);
            }

            if (phdr[i].p_flags & PF_X)
            {
                mprotect((unsigned char *) *load_addr + phdr[i].p_vaddr, phdr[i].p_memsz, PROT_EXEC | PROT_WRITE);
            }
            break; 
        }
    }

    return true;
}

Elf32_Shdr * find_section(void * src, unsigned type){
    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    Elf32_Shdr * shdr = (Elf32_Shdr *)(src + ehdr -> e_shoff);

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

void * find_entry(void * src, void ** load_addr){

    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    return *load_addr + ehdr -> e_entry;
}

void * find_sym(void * src, void * dest){
    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    Elf32_Shdr * shdr = (Elf32_Shdr *)(src + ehdr -> e_shoff);

    int sym_size = find_section(src, SHT_SYMTAB) -> sh_size / sizeof(Elf32_Sym);

    Elf32_Shdr * symtab = find_section(src, SHT_SYMTAB);

    Elf32_Sym * syms = (Elf32_Sym*)(src + symtab -> sh_offset);
    
    for(int i = 0; i < sym_size; i++){
        if(ELF32_ST_TYPE(syms[i].st_info) == 3){
            printf("sym %x %x\n", syms[i].st_value, *(int *)(dest+syms[i].st_value));
            // *(int *)(dest+syms[i].st_value) =  *(int *)(dest+syms[i].st_value);
        }
    }

    return NULL;
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

bool relocate_section(void * src, void * dst, Elf32_Shdr * shdr, Elf32_Sym * syms, void * strings){
    Elf32_Rel* rel = (Elf32_Rel*)(src + shdr->sh_offset);
    for(int j = 0; j < shdr->sh_size / sizeof(Elf32_Rel); j++){
        char * sym = strings + syms[ELF32_R_SYM(rel[j].r_info)].st_name;
        switch (ELF32_R_TYPE(rel[j].r_info)){
            case R_386_RELATIVE:
                *(Elf32_Word*)(dst + rel[j].r_offset) = *(Elf32_Word*)(dst + rel[j].r_offset) + (Elf32_Word) dst;
                break;
            case R_386_JMP_SLOT:
            case R_386_GLOB_DAT:
                *(Elf32_Word*)(dst + rel[j].r_offset) = (Elf32_Word)load_dl(sym);
                break;
        }
    }
    return true;
}

bool relocate(void * src, void ** load_addr){

    Elf32_Ehdr * ehdr = (Elf32_Ehdr *) src;

    Elf32_Shdr * shdr = (Elf32_Shdr *)(src + ehdr -> e_shoff);

    if(ehdr -> e_shentsize != sizeof(Elf32_Shdr)){
        printf("invalid shentsize\n");
        return NULL;
    }

    Elf32_Shdr * dynsym = find_section(src, SHT_DYNSYM);

    Elf32_Sym * syms = (Elf32_Sym*)(src + dynsym -> sh_offset);
    char * strings = src + shdr[dynsym -> sh_link].sh_offset;

    int i;
    for(i = 0; i < ehdr -> e_shentsize; i++){
        switch(shdr[i].sh_type){
            case SHT_REL:
                if(!relocate_section(src, *load_addr, shdr + i, syms, strings))
                    return false;
        }
    }

    return true;
}