/*

Заголовочный файл модуля disas.c.

Панасенко Дмитрий Игоревич, Маткин Илья Александрович     02.05.2020

*/


#ifndef _DISAS_H_
#define _DISAS_H_


#include <windows.h>

#include "mz.h"


#define INSTRUCTION_LEN  7

//----------------------------------------

void DisasCodeSeg(MZHeaders *mz);


// указатель на функцию обработки команды
typedef DWORD (*d_func)(MZHeaders *mz, DWORD pos, char *inst);

// функции обработки каждой команды
DWORD d_aaa(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_aad(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_aam(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_aas(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_adc(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_add(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_and(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_call(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cbw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_clc(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cld(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cli(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cmc(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cmp(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cmpsb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cmpsw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_cwd(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_daa(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_das(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_dec(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_div(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_hlt(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_idiv(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_imul(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_in(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_inc(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_int(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_int3(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_into(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_iret(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_ja(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jae(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jbe(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jc(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_je(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jg(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jge(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jl(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jle(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jna(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnae(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnb(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnbe(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnc(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jne(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jng(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnge(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnl(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnle(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jno(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jnp(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jns(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jnz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jo(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_jp(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jpe(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jpo(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_js(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jcxz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_jmp(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lahf(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lds(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lea(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_les(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lock(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lodsb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_lodsw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_loop(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_loope(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_loopne(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_loopnz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_loopz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_mov(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_movsb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_movsw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_mul(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_neg(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_nop(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_not(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_or(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_out(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_pop(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_popf(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_push(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_pushf(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_rcl(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_rcr(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_rep(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_repe(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_repne(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_repnz(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_pepz(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_retn(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_retf(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_rol(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_ror(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_sahf(MZHeaders *mz, DWORD pos, char *inst);
//DWORD d_sal(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_sar(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_sbb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_scasb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_scasw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_shl(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_shr(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_stc(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_std(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_sti(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_stosb(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_stosw(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_sub(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_test(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_wait(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_xchg(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_xlat(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_xor(MZHeaders *mz, DWORD pos, char *inst);

// функции обработки префикса замены сегмента
DWORD d_cs(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_ds(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_es(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_ss(MZHeaders *mz, DWORD pos, char *inst);

// функции обработки групп команд
DWORD d_gr80(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_grd0(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_grf6(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_grfe(MZHeaders *mz, DWORD pos, char *inst);
DWORD d_grff(MZHeaders *mz, DWORD pos, char *inst);

// функция обработки неизвестной команды
DWORD d_unk(MZHeaders *mz, DWORD pos, char *inst);

//----------------------------------------

#endif  // _DISAS_H_