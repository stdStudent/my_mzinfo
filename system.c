/*

Модуль вспомогательных системных функций.

Панасенко Дмитрий Игоревич, Маткин Илья Александрович     30.04.2020

*/


#include <windows.h>
#include <stdio.h>

#include "system.h"


//
// Отладочная печать сообщений об ошибках.
//
void PrintErrorMessage (unsigned int err) {
    LPSTR msg;

    DWORD res = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL,
            err,    // код ошибки
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),  // идентификатор английского языка
            (LPSTR) &msg,
            0,
            NULL);

    if (res != 0) {
        printf("%s", msg);
        LocalFree(msg);    // освобождение буфера с текстом сообщения
    }
}


//
// Отладочная печать сообщения о последней ошибке.
//
void PrintLastErrorMessage() {
    PrintErrorMessage (GetLastError());
}