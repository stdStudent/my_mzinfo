/*

Модуль main.c.

Главный модуль программы mzinfo отображения информации о MZ-файле.

Панасенко Дмитрий Игоревич, Маткин Илья Александрович     30.04.2020

*/


#include <windows.h>
#include <stdio.h>

#include "disas.h"
#include "mz.h"


//
// Подсказка по использованию.
//
void usage(void) {

    printf("mzinfo <filename> <opt>\n");
    printf("opt:\n"
           "\theader\n"
           "\trelocs\n"
           "\tsegments\n"
           "\tinfo\n"
           "\tdisas\n"
           "\tall\n");
}

int main(int argc, char *argv[], char *envp[]) {
    MZHeaders mz;
    char *filename;

    if(argc < 3){
        usage();
        return 1;
    }

    filename = argv[1];

    if (LoadMZFile(filename, &mz) == FALSE) {
        return 1;
    }

    printf("Information about %s\n\n", filename);

    if (!strcmp (argv[2], "header")) {
        PrintDOSHeader(&mz);
    }

    if (!strcmp (argv[2], "relocs")) {
        PrintRelocsTable(&mz);
    }

    if (!strcmp (argv[2], "segments")) {
        PrintSegments(&mz);
    }

    if (!strcmp (argv[2], "info")) {
        PrintDOSHeader(&mz);
        PrintRelocsTable(&mz);
        PrintSegments(&mz);
    }

    if (!strcmp (argv[2], "disas")) {
        DisasCodeSeg(&mz);
    }

    if (!strcmp (argv[2], "all")) {
        PrintDOSHeader(&mz);
        PrintRelocsTable(&mz);
        PrintSegments(&mz);
        DisasCodeSeg(&mz);
    }

    UnloadMZFile(&mz);

    return 0;
}