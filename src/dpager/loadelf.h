#include <linux/elf.h>

#define true 1
#define false 0

#define R_386_NONE	0
#define R_386_32	1
#define R_386_PC32	2
#define R_386_GOT32	3
#define R_386_PLT32	4
#define R_386_COPY	5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF	9
#define R_386_GOTPC	10
#define R_386_NUM	11

typedef char bool;

void * load_elf(int file, void ** dest);
void * find_entry(Elf32_Ehdr * ehdr, void ** load_addr);
bool elf_check_valid(Elf32_Ehdr * ehdr);
bool load_segment(Elf32_Ehdr * ehdr, Elf32_Phdr * phdr, void ** load_addr);
bool relocate(int file, Elf32_Ehdr * ehdr, Elf32_Shdr * shdr, void ** load_addr);