#ifndef __ELF_LOADER_H__
#define __ELF_LOADER_H__

#include <esp_elf.h>

extern const uint8_t test_elf_start[] asm("_binary_test_xtensa_elf_start");
extern const uint8_t test_elf_end[] asm("_binary_test_xtensa_elf_end");

class ElfLoader
{
public:
    static int elf_args_decode(const char *str, int *argc, char ***argv);
    static void elf_args_free(int argc, char **argv);
    static void elf_loader(const uint8_t *elf_start, const uint8_t *elf_end);
};

#endif // __ELF_LOADER_H__