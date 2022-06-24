/*

Модуль дизассемблирования.

Панасенко Дмитрий Игоревич, Маткин Илья Александрович     02.05.2020

*/


#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "disas.h"
#include "mz.h"
#include "system.h"

#define GET_VARS \
    BYTE opc  = *(mz->code + pos);                  \
    BYTE addr = *(mz->code + pos + 1);              \
    /********/                                      \
    BYTE num1 = *(mz->code + pos + 2); /* byte3 */  \
    BYTE num2 = *(mz->code + pos + 1); /* byte2 */  \
    WORD result;                                    \
    /********/                                      \
    int D_S = (opc & 0b000000010) / 0b10;           \
    int W   = opc & 0b000000001;                    \
    /********/                                      \
    int MOD = (addr & 0b11000000) / 0b1000000;      \
    int REG = (addr & 0b00111000) / 0b1000;         \
    int R_M = addr & 0b00000111;                    \
    /********/                                      \
    char* dest = chooseSet(MOD, R_M, W, num1);      \
    char* reg  = get_reg(REG, W);

// массив опкодов (от 00 до FF)
d_func opcodes[] = {
        d_add,    d_add,    d_add,    d_add,    d_add,    d_add,    d_push,   d_pop,
        d_or,     d_or,     d_or,     d_or,     d_or,     d_or,     d_push,   d_unk,
        d_adc,    d_adc,    d_adc,    d_adc,    d_adc,    d_adc,    d_push,   d_pop,
        d_sbb,    d_sbb,    d_sbb,    d_sbb,    d_sbb,    d_sbb,    d_push,   d_pop,
        d_and,    d_and,    d_and,    d_and,    d_and,    d_and,    d_es,     d_daa,
        d_sub,    d_sub,    d_sub,    d_sub,    d_sub,    d_sub,    d_cs,     d_das,
        d_xor,    d_xor,    d_xor,    d_xor,    d_xor,    d_xor,    d_ss,     d_aaa,
        d_cmp,    d_cmp,    d_cmp,    d_cmp,    d_cmp,    d_cmp,    d_ds,     d_aas,
        d_inc,    d_inc,    d_inc,    d_inc,    d_inc,    d_inc,    d_inc,    d_inc,
        d_dec,    d_dec,    d_dec,    d_dec,    d_dec,    d_dec,    d_dec,    d_dec,
        d_push,   d_push,   d_push,   d_push,   d_push,   d_push,   d_push,   d_push,
        d_pop,    d_pop,    d_pop,    d_pop,    d_pop,    d_pop,    d_pop,    d_pop,
        d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,
        d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,
        d_jo,     d_jno,    d_jb,     d_jae,    d_jz,     d_jnz,    d_jbe,    d_ja,
        d_js,     d_jns,    d_jpe,    d_jpo,    d_jl,     d_jge,    d_jle,    d_jg,
        d_gr80,   d_gr80,   d_gr80,   d_gr80,   d_test,   d_test,   d_xchg,   d_xchg,
        d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_lea,    d_mov,    d_pop,
        d_nop,    d_xchg,   d_xchg,   d_xchg,   d_xchg,   d_xchg,   d_xchg,   d_xchg,
        d_cbw,    d_cwd,    d_call,   d_wait,   d_pushf,  d_popf,   d_sahf,   d_lahf,
        d_mov,    d_mov,    d_mov,    d_mov,    d_movsb,  d_movsw,  d_cmpsb,  d_cmpsw,
        d_test,   d_test,   d_stosb,  d_stosw,  d_lodsb,  d_lodsw,  d_scasb,  d_scasw,
        d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,
        d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,    d_mov,
        d_unk,    d_unk,    d_retn,   d_retn,   d_les,    d_lds,    d_mov,    d_mov,
        d_unk,    d_unk,    d_retf,   d_retf,   d_int3,   d_int,    d_into,   d_iret,
        d_grd0,   d_grd0,   d_grd0,   d_grd0,   d_aam,    d_aad,    d_unk,    d_xlat,
        d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,    d_unk,
        d_loopnz, d_loopz,  d_loop,   d_jcxz,   d_in,     d_in,     d_out,    d_out,
        d_call,   d_jmp,    d_jmp,    d_jmp,    d_in,     d_in,     d_out,    d_out,
        d_lock,   d_unk,    d_rep,    d_rep,    d_hlt,    d_cmc,    d_grf6,   d_grf6,
        d_clc,    d_stc,    d_cli,    d_sti,    d_cld,    d_std,    d_grfe,   d_grff
};

static int vasprintf(char** strp, const char* fmt, va_list ap)
{
    va_list ap_copy;
    int formattedLength, actualLength;
    size_t requiredSize;

    // be paranoid
    *strp = NULL;

    // copy va_list, as it is used twice
    va_copy(ap_copy, ap);

    // compute length of formatted string, without NULL terminator
    formattedLength = _vscprintf(fmt, ap_copy);
    va_end(ap_copy);

    // bail out on error
    if (formattedLength < 0)
    {
        return -1;
    }

    // allocate buffer, with NULL terminator
    requiredSize = ((size_t)formattedLength) + 1;
    *strp = (char*)malloc(requiredSize);

    // bail out on failed memory allocation
    if (*strp == NULL)
    {
        errno = ENOMEM;
        return -1;
    }

    // write formatted string to buffer, use security hardened _s function
    actualLength = vsnprintf_s(*strp, requiredSize, requiredSize - 1, fmt, ap);

    // again, be paranoid
    if (actualLength != formattedLength)
    {
        free(*strp);
        *strp = NULL;
        errno = 131; //EOTHER
        return -1;
    }

    return formattedLength;
}

static int asprintf(char** strp, const char* fmt, ...)
{
    int result;

    va_list ap;
    va_start(ap, fmt);
    result = vasprintf(strp, fmt, ap);
    va_end(ap);

    return result;
}

static char* concatf(const char* fmt, ...) {
    va_list args;
    char* buf = NULL;
    va_start(args, fmt);
    int n = vasprintf(&buf, fmt, args);
    va_end(args);
    if (n < 0) { free(buf); buf = NULL; }
    return buf;
}

typedef struct { bool isWord; BYTE word; } WW;
WW def = {.isWord = false, .word = 0};

/*
 * ifWord == {false, 0} by default
 */
char* _chooseSet(int MOD, int R_M, int W, BYTE ifByte, WW ifWord) {
    WORD num;
    if (ifWord.isWord)
        num = (ifWord.word * 0x100 + ifByte);
    else num = ifByte;

    bool flagM = false;
    if (ifWord.isWord == false && num > 0x7F) {
        flagM = true;
        num = ~num;
        num = num % 0x80;
        num = num + 1;
    } else if (ifWord.isWord == true && num > 0x7FFF) {
        flagM = true;
        num = ~num + 1;
    }

    char* dest;
    switch (MOD) {
        case 0b00:
            switch (R_M) {
                case 0b000:
                    W ? (dest = "word ptr[BX + SI]") : (dest = "byte ptr[BX + SI]");
                    return dest;

                case 0b001:
                    W ? (dest = "word ptr[BX + DI]") : (dest = "byte ptr[BX + DI]");
                    return dest;

                case 0b010:
                    W ? (dest = "word ptr[BP + SI]") : (dest = "byte ptr[BP + SI]");
                    return dest;

                case 0b011:
                    W ? (dest = "word ptr[BP + DI]") : (dest = "byte ptr[BP + DI]");
                    return dest;

                case 0b100:
                    W ? (dest = "word ptr[SI]") : (dest = "byte ptr[SI]");
                    return dest;

                case 0b101:
                    W ? (dest = "word ptr[DI]") : (dest = "byte ptr[DI]");
                    return dest;

                case 0b110:
                    if (flagM)
                    {
                        dest = concatf("-%dh", num);
                    }
                    dest = concatf("%dh", num); // D16
                    return dest;

                case 0b111:
                    W ? (dest = "word ptr[BX]") : (dest = "byte ptr[BX]");
                    return dest;

                default:
                    exit(-100);
            }
            break; // 0b00

        case 0b01:
            switch (R_M) {
                case 0b000:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX + SI - %dh]", num)) : (dest = concatf("byte ptr[BX + SI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BX + SI + %dh]", num)) : (dest = concatf("byte ptr[BX + SI + %dh]", num)); // D8
                    return dest;

                case 0b001:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX + DI - %dh]", num)) : (dest = concatf("byte ptr[BX + DI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BX + DI + %dh]", num)) : (dest = concatf("byte ptr[BX + DI + %dh]", num)); // D8
                    return dest;

                case 0b010:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BP + SI - %dh]", num)) : (dest = concatf("byte ptr[BP + SI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BP + SI + %dh]", num)) : (dest = concatf("byte ptr[BP + SI + %dh]", num)); // D8
                    return dest;

                case 0b011:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BP + DI - %dh]", num)) : (dest = concatf("byte ptr[BP + DI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BP + DI + %dh]", num)) : (dest = concatf("byte ptr[BP + DI + %dh]", num)); // D8
                    return dest;

                case 0b100:
                    if (flagM)
                        W ? (dest = concatf("word ptr[SI - %dh]", num)) : (dest = concatf("byte ptr[SI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[SI + %dh]", num)) : (dest = concatf("byte ptr[SI + %dh]", num)); // D8
                    return dest;

                case 0b101:
                    if (flagM)
                        W ? (dest = concatf("word ptr[DI - %dh]", num)) : (dest = concatf("byte ptr[DI - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[DI + %dh]", num)) : (dest = concatf("byte ptr[DI + %dh]", num)); // D8
                    return dest;

                case 0b110:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BP - %dh]", num)) : (dest = concatf("byte ptr[BP - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BP + %dh]", num)) : (dest = concatf("byte ptr[BP + %dh]", num)); // D8
                    return dest;

                case 0b111:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX - %dh]", num)) : (dest = concatf("byte ptr[BX - %dh]", num)); // D8
                    else
                        W ? (dest = concatf("word ptr[BX + %dh]", num)) : (dest = concatf("byte ptr[BX + %dh]", num)); // D8
                    return dest;

                default:
                    exit(-101);
            }
            break; // 0b01

        case 0b10:
            switch (R_M) {
                case 0b000:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX + SI - %dh]", num)) : (dest = concatf("byte ptr[BX + SI - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[BX + SI + %dh]", num)) : (dest = concatf("byte ptr[BX + SI + %dh]", num)); // D16
                    return dest;

                case 0b001:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX + SI - %dh]", num)) : (dest = concatf("byte ptr[BX + SI - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[BX + DI + %dh]", num)) : (dest = concatf("byte ptr[BX + DI + %dh]", num)); // D16
                    return dest;

                case 0b010:
                case 0b011:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BP + SI - %dh]", num)) : (dest = concatf("byte ptr[BP + SI - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[BP + SI + %dh]", num)) : (dest = concatf("byte ptr[BP + SI + %dh]", num)); // D16
                    return dest;

                case 0b100:
                    if (flagM)
                        W ? (dest = concatf("word ptr[SI - %dh]", num)) : (dest = concatf("byte ptr[SI - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[SI + %dh]", num)) : (dest = concatf("byte ptr[SI + %dh]", num)); // D16
                    return dest;

                case 0b101:
                    if (flagM)
                        W ? (dest = concatf("word ptr[DI - %dh]", num)) : (dest = concatf("byte ptr[DI - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[DI + %dh]", num)) : (dest = concatf("byte ptr[DI + %dh]", num)); // D16
                    return dest;

                case 0b110:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BP - %dh]", num)) : (dest = concatf("byte ptr[BP - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[BP + %dh]", num)) : (dest = concatf("byte ptr[BP + %dh]", num)); // D16
                    return dest;

                case 0b111:
                    if (flagM)
                        W ? (dest = concatf("word ptr[BX - %dh]", num)) : (dest = concatf("byte ptr[BX - %dh]", num)); // D16
                    else
                        W ? (dest = concatf("word ptr[BX + %dh]", num)) : (dest = concatf("byte ptr[BX + %dh]", num)); // D16
                    return dest;

                default:
                    exit(-102);
            }
            break; // 0b10

        case 0b11:
            switch (R_M) {
                case 0b000:
                    W ? (dest = "ax") : (dest = "al");
                    return dest;

                case 0b001:
                    W ? (dest = "cx") : (dest = "cl");
                    return dest;

                case 0b010:
                    W ? (dest = "dx") : (dest = "dl");
                    return dest;

                case 0b011:
                    W ? (dest = "bx") : (dest = "bl");
                    return dest;

                case 0b100:
                    W ? (dest = "sp") : (dest = "ah");
                    return dest;

                case 0b101:
                    W ? (dest = "bp") : (dest = "ch");
                    return dest;

                case 0b110:
                    W ? (dest = "si") : (dest = "dh");
                    return dest;

                case 0b111:
                    W ? (dest = "di") : (dest = "dh");
                    return dest;

                default:
                    exit(-103);
            }
            break; // 0b11

        default:
            exit(-104);
    }
}
#define DEF_OR_ARG(value,...) value
#define chooseSet(MOD,R_M,W,num,...) _chooseSet(MOD,R_M,W,num, DEF_OR_ARG(__VA_ARGS__ __VA_OPT__(,) def))

char* chooseSegmentReg(int REG) {
    switch (REG) {
        case 0b00:
            return "es";

        case 0b01:
            return "cs";

        case 0b10:
            return "ss";

        case 0b11:
            return "ds";

        default:
            exit(-400);
    }
}

char* chooseBaseReg(int R_M) {
    switch (R_M) {
        case 0b000:
            return "ax";

        case 0b001:
            return "cx";

        case 0b010:
            return "dx";

        case 0b011:
            return "bx";

        case 0b100:
            return "sp";

        case 0b101:
            return "bp";

        case 0b110:
            return "si";

        case 0b111:
            return "di";

        default:
            exit(-500);
    }
}

char* get_reg(int REG, int W) {
    switch (W) {
        case 0:
            switch (REG) {
                case 0b000:
                    return "al";

                case 0b001:
                    return "cl";

                case 0b010:
                    return "dl";

                case 0b011:
                    return "bl";

                case 0b100:
                    return "ah";

                case 0b101:
                    return "ch";

                case 0b110:
                    return "dh";

                case 0b111:
                    return "bh";

                default:
                    exit(-200);
            }
            break; // byte

        case 1:
            switch (REG) {
                case 0b000:
                    return "ax";

                case 0b001:
                    return "cx";

                case 0b010:
                    return "dx";

                case 0b011:
                    return "bx";

                case 0b100:
                    return "sp";

                case 0b101:
                    return "bp";

                case 0b110:
                    return "si";

                case 0b111:
                    return "di";

                default:
                    exit(-201);
            }
            break; // word

        default:
            exit(-202);

    }
}

//
// функция вывода одной дизассемблиованной инструкции на экран
//
void PrintInstruction(MZHeaders* mz, DWORD pos, DWORD inst_len, char* inst) {
    unsigned int i;

    printf("%04X:%04X ", mz->doshead->e_cs + (SIZE_OF_SEG_P * (pos / SIZE_OF_SEG_B)) + 0x1000, pos % SIZE_OF_SEG_B);
    for (i = 0; i < inst_len; i++) {
        printf("%02X", (mz->code + pos)[i]);
    }
    for (i = inst_len; i < INSTRUCTION_LEN; i++) {
        printf("  ");
    }
    printf(" %s\n", inst);
}

DWORD g_pos = 0, g_inst_len;
//
// функция дизассембли ования сегмента кода
//
void DisasCodeSeg(MZHeaders* mz) {
    DWORD pos = 0, inst_len;
    char inst[256];

    printf("Disassembled code segment\n");

    ApplyRelocs(mz, IMAGE_BASE_SEG);
    while (pos < mz->code_size) {
        inst_len = opcodes[*(mz->code + pos)](mz, pos, inst);
        PrintInstruction(mz, pos, inst_len, inst);
        pos += inst_len;

        g_pos = pos;
        g_inst_len = inst_len;
    }
    RemoveRelocs(mz, IMAGE_BASE_SEG);
}

//
// функции обработки команд
//
DWORD d_aaa(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "aaa");
    return 1;
}

DWORD d_aad(MZHeaders* mz, DWORD pos, char* inst) {
    if (*(mz->code + pos + 1) == 10) {
        strcpy(inst, "aad");
    } else {
        sprintf(inst, "aad    %u", *(mz->code + pos + 1));
    }
    return 2;
}

DWORD d_aam(MZHeaders* mz, DWORD pos, char* inst) {
    if (*(mz->code + pos + 1) == 10) {
        strcpy(inst, "aam");
    } else {
        sprintf(inst, "aam    %u", *(mz->code + pos + 1));
    }
    return 2;
}

DWORD d_aas(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "aas");
    return 1;
}

DWORD d_adc(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x10:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "adc    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x11:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "adc    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x12:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "adc    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x13:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "adc    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x14:
            result = num2;
            sprintf(inst, "adc    al, %2Xh", result);
            return 2;

        case 0x15:
            result = num1 * 0x100 + num2;
            sprintf(inst, "adc    ax, %4Xh", result);
            return 3;

        case 0x80:
            //D_S ? dest : (dest = concatf("byte ptr[%s]", dest), num2 = mz->code+pos+3);
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "adc    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            //D_S ? dest : (dest = concatf("word ptr[%s]", dest), num2 = mz->code + pos + 3, num1 = mz->code+pos+4);
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "adc    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            //D_S ? dest : (dest = concatf("word ptr[%s]", dest), num2 = mz->code + pos + 3);
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "adc    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_add(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x00:
            //D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "add    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x01:
            //D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "add    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x02:
            //D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "add    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x03:
            //D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "add    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x04:
            result = num2;
            sprintf(inst, "add    al, %2Xh", result);
            return 2;

        case 0x05:
            result = num1 * 0x100 + num2;
            sprintf(inst, "add    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "add    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "add    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "add    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_and(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x20:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "adc    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x21:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "adc    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x22:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "adc    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x23:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "adc    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x24:
            result = num2;
            sprintf(inst, "adc    al, %2Xh", result);
            return 2;

        case 0x25:
            result = num1 * 0x100 + num2;
            sprintf(inst, "adc    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "adc    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "adc    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "adc    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_call(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 2),
         offs2 = *(mz->code + pos + 1);
    switch (opc)
    {
        case 0xE8:
            offs1  = offs1 * 0x100 + offs2;
            result = (g_pos + 3)   + offs1;
            tmp = mz->doshead->e_cs + 0x1000;
            sprintf(inst, "call   %X:%04X", tmp, result);
            return 3;

        case 0xFF:
            offs1 = *(mz->code + pos + 3);
            offs2 = *(mz->code + pos + 2);
            tmp = offs1 * 0x100 + offs2;
            sprintf(inst, "call   dword ptr[%X]", result, tmp);
            return 4;

        case 0x9A:
            offs1 = *(mz->code + pos + 4);
            offs2 = *(mz->code + pos + 3);
            result = offs1 * 0x100 + offs2;
            offs1 = *(mz->code + pos + 2);
            offs2 = *(mz->code + pos + 1);
            tmp = offs1 * 0x100 + offs2;
            sprintf(inst, "call   far ptr    %X:%X", result, tmp);
            return 5;

        default:
            exit(-100000);
    }
}

DWORD d_cbw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cbw");
    return 1;
}

DWORD d_clc(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "clc");
    return 1;
}

DWORD d_cld(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cld");
    return 1;
}

DWORD d_cli(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cli");
    return 1;
}

DWORD d_cmc(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cmc");
    return 1;
}

DWORD d_cmp(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x38:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "cmp    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x39:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "cmp    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x3A:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "cmp    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x3B:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "cmp    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x3C:
            result = num2;
            sprintf(inst, "cmp    al, %2Xh", result);
            return 2;

        case 0x3D:
            result = num1 * 0x100 + num2;
            sprintf(inst, "cmp    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "cmp    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "cmp    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "cmp    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_cmpsb(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cmpsb");
    return 1;
}

DWORD d_cmpsw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cmpsw");
    return 1;
}

DWORD d_cwd(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "cwd");
    return 1;
}

DWORD d_daa(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "das");
    return 1;
}

DWORD d_das(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "das");
    return 1;
}

DWORD d_dec(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x48:
            strcpy(inst, "dec   ax");
            return 2;

        case 0x49:
            strcpy(inst, "dec   cx");
            return 2;

        case 0x4A:
            strcpy(inst, "dec   dx");
            return 2;

        case 0x4B:
            strcpy(inst, "dec   bx");
            return 2;

        case 0x4C:
            strcpy(inst, "dec   sp");
            return 2;

        case 0x4D:
            strcpy(inst, "dec   bp");
            return 2;

        case 0x4E:
            strcpy(inst, "dec   si");
            return 2;

        case 0x4F:
            strcpy(inst, "dec   di");
            return 2;

        case 0xFE:
            result = num1 * 0x100 + num2;
            sprintf(inst, "dec    byte ptr[%04Xh]", result);
            return 4;

        case 0xFF:
            result = num1 * 0x100 + num2;
            sprintf(inst, "dec    word ptr[%04Xh]", result);
            return 4;

        default:
            exit(-1900);
    }
}

DWORD d_div(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xF6:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "div    %s", dest);

            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        case 0xF7:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "div    %s", dest);

            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        default:
            exit(2003);
    }
}

DWORD d_hlt(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "hlt");
    return 1;
}

DWORD d_idiv(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xF6:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "idiv   %s", dest);

            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        case 0xF7:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "idiv   %s", dest);

            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        default:
            exit(2001);
    }
}

DWORD d_imul(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xF6:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "imul   %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        case 0xF7:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "imul   %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        default:
            exit(2001);
    }
}

DWORD d_in(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0xE4:
            result = num2;
            sprintf(inst, "in     al, %04Xh", result);
            return 2;

        case 0xE5:
            result = num1 * 0x100 + num2;
            sprintf(inst, "in     ax, %04Xh", result);
            return 2;

        case 0xEC:
            strcpy(inst, "in     al, dx");
            return 1;

        case 0xED:
            strcpy(inst, "in     ax, dx");
            return 1;

        default:
            exit(-1500);
    }
}

DWORD d_inc(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x40:
            strcpy(inst, "inc    ax");
            return 1;

        case 0x41:
            strcpy(inst, "inc    cx");
            return 1;

        case 0x42:
            strcpy(inst, "inc    dx");
            return 1;

        case 0x43:
            strcpy(inst, "inc    bx");
            return 1;

        case 0x44:
            strcpy(inst, "inc    sp");
            return 1;

        case 0x45:
            strcpy(inst, "inc    bp");
            return 1;

        case 0x46:
            strcpy(inst, "inc    si");
            return 1;

        case 0x47:
            strcpy(inst, "inc    di");
            return 1;

        case 0xFE:
            result = num1 * 0x100 + num2;
            sprintf(inst, "inc    byte ptr[%04Xh]", result);
            return 4;

        case 0xFF:
            result = num1 * 0x100 + num2;
            sprintf(inst, "inc    word ptr[%04Xh]", result);
            return 4;

        default:
            exit(-1700);
    }
}

DWORD d_int(MZHeaders* mz, DWORD pos, char* inst) {
    sprintf(inst, "int    %2Xh", *(mz->code + pos + 1));
    return 2;
}

DWORD d_int3(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "int    3");
    return 1;
}

DWORD d_into(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "into");
    return 1;
}

DWORD d_iret(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "iret");
    return 1;
}

DWORD d_ja(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "ja    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jae(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jae    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jb(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jb    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jbe(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jbe    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jg(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jg    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jge(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jge    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jl(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jl    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jle(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jle    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jno(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jno    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jns(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jns    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jnz(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jnz    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jo(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jo    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jpe(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jpe    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jpo(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jpo    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_js(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "js    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jz(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jz    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jcxz(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);

    result = (g_pos + 2) + offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "jcxz    %X:00%2X", tmp, result);
    return 2;
}

DWORD d_jmp(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result =0;
    WORD offs1 = *(mz->code + pos + 2),
            offs2 = *(mz->code + pos + 1);
    switch (opc)
    {
        case 0xEB:
            result = (g_pos + 2) + offs1;
            tmp = mz->doshead->e_cs + 0x1000;
            sprintf(inst, "jmp    %X:00%2X", tmp, result);
            return 2;

        case 0xE9:
            offs1 = offs1 * 0x100 + offs2;
            result = (g_pos + 3) + offs1;
            tmp = mz->doshead->e_cs + 0x1000;
            sprintf(inst, "jmp %X:%4X", tmp, result);
            return 3;

        case 0xFF:
            offs1 = *(mz->code + pos + 3);
            offs2 = *(mz->code + pos + 2);
            tmp = offs1 * 0x100 + offs2;
            sprintf(inst, "jmp    dword ptr[%X]", result, tmp);
            return 4;

        case 0xEA:
            offs1 = *(mz->code + pos + 4);
            offs2 = *(mz->code + pos + 3);
            result = offs1 * 0x100 + offs2;
            offs1 = *(mz->code + pos + 2);
            offs2 = *(mz->code + pos + 1);
            tmp = offs1 * 0x100 + offs2;
            sprintf(inst, "jmp    far ptr    %X:%X", result, tmp);
            return 5;

        default:
            exit(-1290);
    }
}

DWORD d_lahf(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "lahf");
    return 1;
}

DWORD d_lds(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    BYTE addr = *(mz->code + pos + 1);

    BYTE num1 = *(mz->code + pos + 2); // byte3
    BYTE num2 = *(mz->code + pos + 1); // byte2
    WORD result;

    int D_S = (opc & 0b000000010) / 0b10;
    int W = opc & 0b000000001;

    int MOD = (addr & 0b11000000) / 0b1000000;
    int REG = (addr & 0b00111000) / 0b1000;
    int R_M = addr & 0b00000111;



    char* dest = chooseSet(MOD, R_M, W, num1);
    char* reg = chooseBaseReg(REG);
    sprintf(inst, "lds    %s, %s", reg, dest);
    if (MOD == 0) return 2; else return 4;
}

DWORD d_lea(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    sprintf(inst, "lea    %s, %s", reg, dest); // swap required
    if (MOD == 0b00 && R_M != 0b110)
        return 2;
    if (MOD == 0b01)
        return 4;
    if (MOD == 0b10)
        return 6;
    if (MOD == 0b00 && R_M == 0b110)
        return 4;

}

DWORD d_les(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    BYTE addr = *(mz->code + pos + 1);

    BYTE num1 = *(mz->code + pos + 2); // byte3
    BYTE num2 = *(mz->code + pos + 1); // byte2
    WORD result;

    int D_S = (opc & 0b000000010) / 0b10;
    int W = opc & 0b000000001;

    int MOD = (addr & 0b11000000) / 0b1000000;
    int REG = (addr & 0b00111000) / 0b1000;
    int R_M = addr & 0b00000111;

    W = 0b1;
    char* dest = chooseSet(MOD, R_M, W, num1);
    char* reg = chooseBaseReg(REG);
    sprintf(inst, "les    %s, %s", reg, dest);
    if (MOD == 0) return 2; else return 4;

}

DWORD d_lock(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "lock");
    return 1;
}

DWORD d_lodsb(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "lodsb");
    return 1;
}

DWORD d_lodsw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "lodsw");
    return 1;
}

DWORD d_loop(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);
    offs1 = ~offs1;
    offs1 %= 0x80;
    offs1 = offs1 - 0x1;


    result = (g_pos) - offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "loop   %X:%04X", tmp, result);
    return 2;
}

DWORD d_loopnz(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);
    offs1 = ~offs1;
    offs1 %= 0x80;
    offs1 = offs1 - 0x1;


    result = (g_pos)-offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "loopnz  %X:%4X", tmp, result);
    return 2;
}

DWORD d_loopz(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    WORD tmp = 0, result = 0;
    WORD offs1 = *(mz->code + pos + 1);
    offs1 = ~offs1;
    offs1 %= 0x80;
    offs1 = offs1 - 0x1;


    result = (g_pos)-offs1;
    tmp = mz->doshead->e_cs + 0x1000;
    sprintf(inst, "loopz  %X:%4X", tmp, result);
    return 2;
}

DWORD d_mov(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x88:
            //D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "mov    %s, %s", dest, reg);
            return 3;

        case 0x89:
            //D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "mov    %s, %s", dest, reg);
            return 3;

        case 0x8A:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "mov    %s, %s", reg, dest); // swap required
            if (MOD == 0b11) return 2; else return 3;

        case 0x8B:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "mov    %s, %s", reg, dest); // swap required
            if (MOD == 0b11) return 2; else return 3;

        case 0x8C:
            reg = chooseSegmentReg(REG);
            dest = chooseBaseReg(R_M);
            sprintf(inst, "mov    %s, %s", dest, reg);
            return 2;

        case 0x8E:
            dest = chooseSegmentReg(REG);
            reg = chooseBaseReg(R_M);
            sprintf(inst, "mov    %s, %s", dest, reg);
            return 2;

        case 0xA0:
            result = num2;
            sprintf(inst, "mov    al, byte ptr[%02Xh]", result);
            return 2;

        case 0xA1:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    ax, word ptr[%04Xh]", result);
            return 3;

        case 0xA2:
            result = num2;
            sprintf(inst, "mov    byte ptr[%04Xh], al", result);
            return 2;

        case 0xA3:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    word ptr[%04Xh], ax", result);
            return 3;

        case 0xB0:
            result = num2;
            sprintf(inst, "mov    al, %04Xh", result);
            return 2;

        case 0xB1:
            result = num2;
            sprintf(inst, "mov    cl, %04Xh", result);
            return 2;

        case 0xB2:
            result = num2;
            sprintf(inst, "mov    dl, %04Xh", result);
            return 2;

        case 0xB3:
            result = num2;
            sprintf(inst, "mov    bl, %04Xh", result);
            return 2;

        case 0xB4:
            result = num2;
            sprintf(inst, "mov    ah, %04Xh", result);
            return 2;

        case 0xB5:
            result = num2;
            sprintf(inst, "mov    ch, %04Xh", result);
            return 2;

        case 0xB6:
            result = num2;
            sprintf(inst, "mov    dh, %04Xh", result);
            return 2;

        case 0xB7:
            result = num2;
            sprintf(inst, "mov    bh, %04Xh", result);
            return 2;

        case 0xB8:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    ax, %04Xh", result);
            return 3;

        case 0xB9:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    cx, %04Xh", result);
            return 3;

        case 0xBA:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    dx, %04Xh", result);
            return 3;

        case 0xBB:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    bx, %04Xh", result);
            return 3;

        case 0xBC:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    sp, %04Xh", result);
            return 3;

        case 0xBD:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    bp, %04Xh", result);
            return 3;

        case 0xBE:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    si, %04Xh", result);
            return 3;

        case 0xBF:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    di, %04Xh", result);
            return 3;

        case 0xC6:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    %s, %dh", dest, result);
            return 4;

        case 0xC7:
            result = num1 * 0x100 + num2;
            sprintf(inst, "mov    %s, %dh", dest, result);
            return 5;

        default:
            exit(-300);
    }
}

DWORD d_movsb(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "movsb");
    return 1;
}

DWORD d_movsw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "movsw");
    return 1;
}

DWORD d_mul(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    BYTE addr = *(mz->code + pos + 1);

    BYTE num1 = *(mz->code + pos + 2); // byte3
    BYTE num2 = *(mz->code + pos + 1); // byte2
    WORD result;

    int D_S = (opc & 0b000000010) / 0b10;
    int W = opc & 0b000000001;

    int MOD = (addr & 0b11000000) / 0b1000000;
    int R_M = addr & 0b00000111;

    char* dest = chooseSet(MOD, R_M, W, num1);

    switch (opc)
    {
        case 0xF6:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "mul    %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        case 0xF7:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "mul    %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        default:
            exit(2004);
    }
}

DWORD d_neg(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    BYTE addr = *(mz->code + pos + 1);

    BYTE num1 = *(mz->code + pos + 2); // byte3
    BYTE num2 = *(mz->code + pos + 1); // byte2
    WORD result;

    int D_S = (opc & 0b000000010) / 0b10;
    int W = opc & 0b000000001;

    int MOD = (addr & 0b11000000) / 0b1000000;
    int R_M = addr & 0b00000111;

    char* dest = chooseSet(MOD, R_M, W, num1);

    switch (opc)
    {
        case 0xF6:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "neg   %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        case 0xF7:
            if (MOD == 0b00 && R_M == 0b110)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "neg   %s", dest);
            if (MOD == 0b11)
                return 2;
            else if (MOD == 0b10 || (MOD == 0b00 && R_M == 0b110))
                return 4;
            else
                return 3;

        default:
            exit(2007);
    }
}

DWORD d_nop(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "nop");
    return 1;
}

DWORD d_not(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    sprintf(inst, "not    %s", dest);

    if (MOD == 0b11) return 2; else return 3;
}

DWORD d_or(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x08:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "or    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x09:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "or    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x0A:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "or    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x0B:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "or    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x0C:
            result = num2;
            sprintf(inst, "or    al, %2Xh", result);
            return 2;

        case 0x0D:
            result = num1 * 0x100 + num2;
            sprintf(inst, "or    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "or    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "or    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "or    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_out(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0xE6:
            result = num2;
            sprintf(inst, "out    %04Xh, al", result);
            return 2;

        case 0xE7:
            result = num1 * 0x100 + num2;
            sprintf(inst, "out    %04Xh, ax", result);
            return 2;

        case 0xEE:
            strcpy(inst, "out    dx, al");
            return 1;

        case 0xEF:
            strcpy(inst, "out    dx, ax");
            return 1;

        default:
            exit(-1300);
    }
}

DWORD d_pop(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x07:
            strcpy(inst, "pop   es");
            return 2;

        case 0x17:
            strcpy(inst, "pop   ss");
            return 2;

        case 0x58:
            strcpy(inst, "pop   ax");
            return 2;

        case 0x59:
            strcpy(inst, "pop   cx");
            return 2;

        case 0x5A:
            strcpy(inst, "pop   dx");
            return 2;

        case 0x5B:
            strcpy(inst, "pop   bx");
            return 2;

        case 0x5C:
            strcpy(inst, "pop   sp");
            return 2;

        case 0x5D:
            strcpy(inst, "pop   bp");
            return 2;

        case 0x5E:
            strcpy(inst, "pop   si");
            return 2;

        case 0x5F:
            strcpy(inst, "pop   di");
            return 2;

        case 0x1F:
            strcpy(inst, "pop   ds");
            return 2;

        default:
            exit(-1100);
    }
}

DWORD d_popf(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "popf");
    return 1;
}

DWORD d_push(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    if (MOD == 0b10) {
        num2 = *(mz->code + pos + 3);
        WW word = {true, num2};

        dest = chooseSet(MOD, R_M, W, num1, word);
    }

    switch (opc) {
        case 0x06:
            strcpy(inst, "push   es");
            return 1;

        case 0x16:
            strcpy(inst, "push   ss");
            return 1;

        case 0x50:
            strcpy(inst, "push   ax");
            return 1;

        case 0x51:
            strcpy(inst, "push   cx");
            return 1;

        case 0x52:
            strcpy(inst, "push   dx");
            return 1;

        case 0x53:
            strcpy(inst, "push   bx");
            return 1;

        case 0x54:
            strcpy(inst, "push   sp");
            return 1;

        case 0x55:
            strcpy(inst, "push   bp");
            return 1;

        case 0x56:
            strcpy(inst, "push   si");
            return 1;

        case 0x57:
            strcpy(inst, "push   di");
            return 1;

        case 0x0E:
            strcpy(inst, "push   cs");
            return 1;

        case 0x1E:
            strcpy(inst, "push   ds");
            return 1;

        case 0xFF:
            sprintf(inst, "push   %s", dest);
            if (MOD == 0b10)
                return 4;
            else
                return 3;

        default:
            exit(-1100);
    }
}

DWORD d_pushf(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "pushf");
    return 1;
}

DWORD d_rcl(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rcl    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rcl    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rcl    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rcl    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        default:
            exit(141);
    }
}

DWORD d_rcr(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rcr    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rcr    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rcr    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rcr    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        default:
            exit(142);
    }
}

DWORD d_rep(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0xF2:
            switch (addr)
            {
                case 0xA6:
                    sprintf(inst, "repne  cmpsb");
                    return 2;

                case 0xA7:
                    sprintf(inst, "repne  cmpsw");
                    return 2;

                case 0xAE:
                    sprintf(inst, "repne  scasb");
                    return 2;

                case 0xAF:
                    sprintf(inst, "repne  scasw");
                    return 2;

                case 0xAB:
                    sprintf(inst, "repnz  stosw");
                    return 2;

                default:
                    exit(1350);
            }

        case 0xF3:
            switch (addr)
            {
                case 0xA4:
                    sprintf(inst, "rep    movsb");
                    return 2;
                case 0xA5:
                    sprintf(inst, "rep    movsw");
                    return 2;
                case 0xAC:
                    sprintf(inst, "rep    lodsb");
                    return 2;
                case 0xAD:
                    sprintf(inst, "rep    lodsw");
                    return 2;
                case 0xAA:
                    sprintf(inst, "rep    stosb");
                    return 2;
                case 0xAB:
                    sprintf(inst, "rep    stosw");
                    return 2;
                case 0xA6:
                    sprintf(inst, "repe    cmpsb");
                    return 2;
                case 0xA7:
                    sprintf(inst, "repe    cmpsw");
                    return 2;
                case 0xAE:
                    sprintf(inst, "repe    scasb");
                    return 2;
                case 0xAF:
                    sprintf(inst, "repe    scasw");
                    return 2;
                default:
                    exit(1250);
            }

        default:
            exit(-1800);
    }
}

DWORD d_retn(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    if (opc == 0xC2) {
        sprintf(inst, "retn   %u", *((PWORD)(mz->code + pos + 1)));
        return 3;
    }
    else {
        strcpy(inst, "retn");
        return 1;
    }
}

DWORD d_retf(MZHeaders* mz, DWORD pos, char* inst) {
    BYTE opc = *(mz->code + pos);
    if (opc == 0xCA) {
        sprintf(inst, "retf   %u", *((PWORD)(mz->code + pos + 1)));
        return 3;
    }
    else {
        strcpy(inst, "retf");
        return 1;
    }
}

DWORD d_rol(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rol    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rol    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "rol    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "rol    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        default:
            exit(143);
    }
}

DWORD d_ror(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "ror    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "ror    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "ror    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "ror    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        default:
            exit(144);
    }
}

DWORD d_sahf(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "sahf");
    return 1;
}

DWORD d_sal(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sal    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sal    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sal    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sal    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xC0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sal    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        case 0xC1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sal    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        default:
            exit(150);
    }
}


DWORD d_sar(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sar    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sar    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sar    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sar    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xC0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sar    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        case 0xC1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sar    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        default:
            exit(151);
    }
}

DWORD d_sbb(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x18:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "sbb    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x19:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "sbb    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x1A:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sbb    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x1B:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sbb    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x1C:
            result = num2;
            sprintf(inst, "sbb    al, %2Xh", result);
            return 2;

        case 0x1D:
            result = num1 * 0x100 + num2;
            sprintf(inst, "sbb    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "sbb    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "sbb    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "sbb    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }
}

DWORD d_scasb(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "scasb");
    return 1;
}

DWORD d_scasw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "scasw");
    return 1;
}

DWORD d_shl(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shl    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shl    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shl    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shl    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xC0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shl    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        case 0xC1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shl    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        default:
            exit(152);
    }
}

DWORD d_shr(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc)
    {
        case 0xD0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shr    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shr    %s, 1", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD2:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shr    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xD3:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shr    %s, cl", dest);
            if (MOD == 0b11)
                return 2;
            else
                return 4;

        case 0xC0:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "shr    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        case 0xC1:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "shr    %s, %2X", dest, num2);
            if (MOD == 0b11)
                return 3;
            else
                return 4;

        default:
            exit(153);
    }
}

DWORD d_stc(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "stc");
    return 1;
}

DWORD d_std(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "std");
    return 1;
}

DWORD d_sti(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "sti");
    return 1;
}

DWORD d_stosb(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "stosb");
    return 1;
}

DWORD d_stosw(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "stosw");
    return 1;
}

DWORD d_sub(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x28:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "sub    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x29:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "sub    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x2A:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "sub    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x2B:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "sub    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x2C:
            result = num2;
            sprintf(inst, "sub    al, %2Xh", result);
            return 2;

        case 0x2D:
            result = num1 * 0x100 + num2;
            sprintf(inst, "sub    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "sub    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "sub    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "sub    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_test(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x84:
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "test   %s, %s", dest, reg);
            if (MOD == 0b11) return 2; else return 4;

        case 0x85:
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "test   %s, %s", dest, reg);
            if (MOD == 0b11) return 2; else return 4;

        case 0xA8:
            result = num2;
            sprintf(inst, "test   al, %04X", result);
            return 2;

        case 0xA9:
            result = num1 * 0x100 + num2;
            sprintf(inst, "test   ax, %04X", result);
            return 3;

        case 0xF6:
            if (MOD == 0b11)
                result = num1;
            else {
                dest = concatf("byte ptr[%s]", dest);
                result = *(mz->code + pos + 4);
            }
            sprintf(inst, "test   %s, %04X", dest, result);
            if (MOD == 0b11)
                return 3;
            else
                return 5;

        case 0xF7:
            if (MOD == 0b11)
                result = num1;
            else {
                dest = concatf("word ptr[%s]", dest);
                result = *(mz->code + pos + 4);
            }
            sprintf(inst, "test   %s, %04X", dest, result);
            if (MOD == 0b11)
                return 3;
            else
                return 5;

        default:
            exit(-1000);
    }
}

DWORD d_wait(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "wait");
    return 1;
}

DWORD d_xchg(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x86: // 2 or 4
            //D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11)
                dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "xchg   %s, %s", reg, dest);
            if (MOD == 0b11 && R_M != 0b000 && W!= 0b01)
                return 2;
            if (MOD != 0b11)
                return 4;

        case 0x87: // Gb Ev from mov 0x8B
            if (MOD != 0b11)
                dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "xchg   %s, %s", reg, dest); // swap required
            if (MOD == 0b11 && R_M != 0b000 && W != 0b01)
                return 2;
            if (MOD != 0b11)
                return 4;

        case 0x91:
            strcpy(inst, "xchg   cx, ax");
            return 1;

        case 0x92:
            strcpy(inst, "xchg   dx, ax");
            return 1;

        case 0x93:
            strcpy(inst, "xchg   bx, ax");
            return 1;

        case 0x94:
            strcpy(inst, "xchg   sp, ax");
            return 1;

        case 0x95:
            strcpy(inst, "xchg   bp,ax");
            return 1;

        case 0x96:
            strcpy(inst, "xchg   si, ax");
            return 1;

        case 0x97:
            strcpy(inst, "xchg   di, ax");
            return 1;

        default:
            exit(-900);
    }
}

DWORD d_xlat(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "xlat");
    return 1;
}

static char* substring(char* string, int position, int length)
{
    char* pointer;
    int c;

    pointer = malloc(length + 1);

    if (pointer == NULL)
        exit(EXIT_FAILURE);

    for (c = 0; c < length; c++)
        *(pointer + c) = *((string + position - 1) + c);

    *(pointer + c) = '\0';

    return pointer;
}

static void insert_substring(char* a, char* b, int position)
{
    if (position == -1)
        return;
    char* f, * e;
    int length;

    length = strlen(a);

    f = substring(a, 1, position - 1);
    e = substring(a, position, length - position + 1);

    strcpy(a, "");
    strcat(a, f);
    free(f);
    strcat(a, b);
    strcat(a, e);
    free(e);
}

static int get_pos_to_insert(char* inst) {
    int i = 0;
    char tmp = inst[0];
    while (tmp != '[') {
        ++i;
        tmp = inst[i];
        if (tmp == '\0')
        {
            i = -1;
            return i;
        }
    }

    return ++i;
}

DWORD d_xor(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (opc) {
        case 0x30:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            sprintf(inst, "xor    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x31:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            sprintf(inst, "xor    %s, %s", dest, reg);
            if (D_S == 0) return 4; else return 2;

        case 0x32:
            D_S ? dest : (dest = concatf("byte ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("byte ptr[%s]", dest);
            sprintf(inst, "xor    %s, %s", reg, dest);
            if (MOD != 0b11) return 4; else return 2;

        case 0x33:
            D_S ? dest : (dest = concatf("word ptr[%s]", dest));
            if (MOD != 0b11) dest = concatf("word ptr[%s]", dest);
            sprintf(inst, "xor    %s, %s", reg, dest);
            if (MOD == 0) return 4; else return 2;

        case 0x34:
            result = num2;
            sprintf(inst, "xor    al, %2Xh", result);
            return 2;

        case 0x35:
            result = num1 * 0x100 + num2;
            sprintf(inst, "xor    ax, %4Xh", result);
            return 3;

        case 0x80:
            if (MOD != 0b11) {
                dest = concatf("byte ptr[%s]", dest);
                num1 = *(mz->code + pos + 4);
            }
            sprintf(inst, "xor    %s, %2Xh", dest, num1);
            if (MOD != 0b11) return 5; else return 3;

        case 0x81:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
                num1 = *(mz->code + pos + 5);
            } else {
                num2 = *(mz->code + pos + 2);
                num1 = *(mz->code + pos + 3);
            }
            result = num1 * 0x100 + num2;
            sprintf(inst, "xor    %s, %4Xh", dest, result);
            if (MOD != 0b11) return 6; else return 4;

        case 0x83:
            if (MOD != 0b11) {
                dest = concatf("word ptr[%s]", dest);
                num2 = *(mz->code + pos + 4);
            } else {
                num2 = *(mz->code + pos + 2);
            }
            sprintf(inst, "xor    %s, %2Xh", dest, num2);
            if (MOD != 0b11) return 5; else return 3;

        default:
            break;
    }

    return 1;
}

DWORD d_cs(MZHeaders* mz, DWORD pos, char* inst) {
    pos += 1;
    g_inst_len = opcodes[*(mz->code + pos)](mz, pos, inst);
    g_pos += g_inst_len;

    insert_substring(inst, " cs:", get_pos_to_insert(inst));

    return g_pos - pos + 2;
}

DWORD d_ds(MZHeaders* mz, DWORD pos, char* inst) {
    pos += 1;
    g_inst_len = opcodes[*(mz->code + pos)](mz, pos, inst);
    g_pos += g_inst_len;

    insert_substring(inst, " ds:", get_pos_to_insert(inst));

    return g_pos - pos + 2;
}

DWORD d_es(MZHeaders* mz, DWORD pos, char* inst) {
    pos += 1;
    g_inst_len = opcodes[*(mz->code + pos)](mz, pos, inst);
    g_pos += g_inst_len;

    insert_substring(inst, " es:", get_pos_to_insert(inst));

    return g_pos - pos + 2;
}

DWORD d_ss(MZHeaders* mz, DWORD pos, char* inst) {
    pos += 1;
    g_inst_len = opcodes[*(mz->code + pos)](mz, pos, inst);
    g_pos += g_inst_len;

    insert_substring(inst, " ss:", get_pos_to_insert(inst));

    return g_pos - pos + 2;
}

DWORD d_gr80(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (REG) {
        case 0b000:
            return d_add(mz, pos, inst);

        case 0b001:
            return d_or(mz, pos, inst);

        case 0b010:
            return d_adc(mz, pos, inst);

        case 0b011:
            return d_sbb(mz, pos, inst);

        case 0b100:
            return d_and(mz, pos, inst);

        case 0b101:
            return d_sub(mz, pos, inst);

        case 0b110:
            return d_xor(mz, pos, inst);

        case 0b111:
            return d_cmp(mz, pos, inst);

        default:
            return d_unk(mz, pos, inst);
    }
}

DWORD d_grd0(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (REG) {
        case 0b000:
            return d_rol(mz, pos, inst);

        case 0b001:
            return d_ror(mz, pos, inst);

        case 0b010:
            return d_rcl(mz, pos, inst);

        case 0b011:
            return d_rcr(mz, pos, inst);

        case 0b100:
            return d_shl(mz, pos, inst);

        case 0b101:
            return d_shr(mz, pos, inst);

        case 0b111:
            return d_sar(mz, pos, inst);

        default:
            return d_unk(mz, pos, inst);
    }
}

DWORD d_grf6(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (REG) {
        case 0b000:
            return d_test(mz, pos, inst);

        case 0b010:
            return d_not(mz, pos, inst);

        case 0b011:
            return d_neg(mz, pos, inst);

        case 0b100:
            return d_mul(mz, pos, inst);

        case 0b101:
            return d_imul(mz, pos, inst);

        case 0b110:
            return d_div(mz, pos, inst);

        case 0b111:
            return d_idiv(mz, pos, inst);

        default:
            return d_unk(mz, pos, inst);
    }
}

DWORD d_grfe(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (REG) {
        case 0b000:
            return d_inc(mz, pos, inst);

        case 0b001:
            return d_dec(mz, pos, inst);

        default:
            return d_unk(mz, pos, inst);
    }
}

DWORD d_grff(MZHeaders* mz, DWORD pos, char* inst) {
    GET_VARS
    switch (REG) {
        case 0b000:
            return d_inc(mz, pos, inst);

        case 0b001:
            return d_dec(mz, pos, inst);

        case 0b010:
        case 0b011:
            return d_call(mz, pos, inst);

        case 0b100:
        case 0b101:
            return d_jmp(mz, pos, inst);

        case 0b110:
            return d_push(mz, pos, inst);

        default:
            return d_unk(mz, pos, inst);
    }
}

DWORD d_unk(MZHeaders* mz, DWORD pos, char* inst) {
    strcpy(inst, "???");
    return 1;
}