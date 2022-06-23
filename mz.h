/*

Заголовочный файл модуля mz.c.

Панасенко Дмитрий Игоревич, Маткин Илья Александрович     30.04.2020

*/


#ifndef _MZ_H_
#define _MZ_H_


#include <windows.h>


#define PARAGRAPH_SIZE  16
#define BYTES_PER_LINE  16
#define IMAGE_BASE_SEG  0x1000

#define SIZE_OF_SEG_P   0x1000
#define SIZE_OF_SEG_B   0x10000

#define MZ_SIGNATURE    0x5A4D

//----------------------------------------

// Структура заголовка DOS
typedef struct _DOS_HEADER {
    WORD   e_magic;                 // Магическое число (сигнатура MZ)
    WORD   e_cblp;                  // Число байт последней 512-байтовой страницы файла
    WORD   e_cp;                    // Общее число 512-байтовых страниц в файле (считая последнюю страницу)
    WORD   e_crlc;                  // Число элементов в таблице релоков
    WORD   e_cparhdr;               // Размер заголовка в параграфах (параграф - 16 байт)
    WORD   e_minalloc;              // Минимальное число дополнительных параграфов
    WORD   e_maxalloc;              // Максимальное число дополнительных параграфов (обычно равно 0FFFFh)
    WORD   e_ss;                    // Начальное значение сегментного регистра SS (относительно начала загруженного модуля)
    WORD   e_sp;                    // Начальное значение регистра SP
    WORD   e_csum;                  // Контрольная сумма (даёт 0 при суммировании с остальными словами в файле)
    WORD   e_ip;                    // Начальное значение регистра IP
    WORD   e_cs;                    // Начальное значение сегментного регистра CS (относительно начала загруженного модуля)
    WORD   e_lfarlc;                // Смещение до начала таблицы релоков
    WORD   e_ovno;                  // Порядок наложения (обычно равен 0h)
} DOS_HEADER, *PDOS_HEADER;

// структура элемента таблицы релоков
typedef struct _RELOC_TABLE_ENTRY {
    WORD offset;
    WORD segment;
} RELOC_TABLE_ENTRY, *PRELOC_TABLE_ENTRY;

// вспомогательная структура загруженного MZ-файла
typedef struct _MZHeaders {
    char                *filename;      // имя файла

    HANDLE              fd;             // хендл открытого файла
    HANDLE              mapd;           // хендл файловой проекции
    PBYTE               mem;            // указатель на память спроецированного файла
    DWORD               filesize;       // размер спроецированной части файла

    PDOS_HEADER         doshead;        // указатель на DOS заголовок

    PBYTE               data;           // указатель на сегмент данных программы (начало образа программы)
    DWORD               data_size;      // размер сегмента данных
    PBYTE               stack;          // указатель на сегмент стека программы
    DWORD               stack_size;     // размер сегмента стека
    PBYTE               code;           // указатель на сегмент кода программы
    DWORD               code_size;      // размер сегмента кода

    PRELOC_TABLE_ENTRY  relocs;         // указатель на начало таблицы релоков
    WORD                relocs_size;    // размер таблицы релоков
} MZHeaders;

BOOL LoadMZFile(char *filename, MZHeaders *mz);

void UnloadMZFile(MZHeaders *mz);

void ApplyRelocs(MZHeaders *mz, WORD imageBase);

void RemoveRelocs(MZHeaders *mz, WORD imageBase);

void PrintDOSHeader(MZHeaders *mz);

void PrintRelocsTable(MZHeaders *mz);

void PrintSegments(MZHeaders *mz);

//----------------------------------------

#endif  // _MZ_H_
