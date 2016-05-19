#include "common.h"
#include "registers.h"
#include "cpu.h"

extern UINT32   inst_decoder (BUSWIDTH inst);
extern BUSWIDTH ALU_unit 	 (UINT32 fs_type, BUSWIDTH arg[], BUSWIDTH *state, int *err);
extern BUSWIDTH inst_fetch	 (BUSWIDTH code[], int size, BUSWIDTH pc );
extern BUSWIDTH data_read	 (BUSWIDTH code[], int size, BUSWIDTH loc);
extern int      data_write	 (BUSWIDTH code[], int size, BUSWIDTH loc, BUSWIDTH value);


int cpu_main( BUSWIDTH code[],  int siz, REGISTER_T *pReg)
{
	BUSWIDTH input[2], output;
	BUSWIDTH br = sizeof(BUSWIDTH);
	BUSWIDTH inst;
	UINT32   cword, fs;
	int      mb, md, rw, mw, pl, jb, bc;
	int      da, aa, ba;
	int      size;
	int      err = 0;

	size = siz;

	//print1n(">>PC:0x%x", pReg->pc[0]);
	inst  = inst_fetch(code, size, pReg->pc[0]);

	if(inst == END_CODE) //end condition
		return RET_END;

	cword = inst_decoder(inst);

	da =  GET_DA(cword);
	aa =  GET_AA(cword);
	ba =  GET_BA(cword);
	mb = (GET_MB(cword))?1:0;
	md = (GET_MD(cword))?1:0;
	rw = (GET_RW(cword))?1:0;
	mw = (GET_MW(cword))?1:0;
	pl = (GET_PL(cword))?1:0;
	jb = (GET_JB(cword))?1:0;
	fs =  GET_FS(cword);
	bc =  GET_BC(cword);

	if(pl)       // branch condition
	{
		int      sign_bit = 0;
		BUSWIDTH reg_val = pReg->r[aa];

		if      (!jb && !reg_val)
		{
			br = (BUSWIDTH)((da << 3)|(ba));// conditional   jump
			if(bc & 1) //음수 이면.
			{
				br = br | 0xFFC0; //음수로 만들어준다.
			}
		}
		else if (jb)
			br = (BUSWIDTH)((da<<6)|(aa<<3)|(ba) );   // unconditional jump
		else
			br = sizeof(BUSWIDTH);                    // other case

		br &= 0xFFFF;

		if(br & 0x8000) //음수
		{
			sign_bit = 1; //negative;
			br |= 0xFFFF0000; // 32bit 의 수로 바꾼 후.
			br = ~br + 1;     // 2의 보수를 취한다
		}

		if (jb) //unconditional jump (direct address)
		{
			pReg->pc[0] = br;
		}
		else
		{
			if(sign_bit) pReg->pc[0] -= br;
			else         pReg->pc[0] += br;
		}

		goto _process_exit;
	}

	if      (mw) //memory write mode
	{
		input[0] = pReg->r[ba];
	}
	else if (md) //memory read mode
	{
		input[1] = data_read(code, size, pReg->r[aa]);
	}
	else if (mb) //constant mode
	{
		if( fs & 0x2) // add  instruction
		{
			input[0] = pReg->r[aa];
			input[1] = (BUSWIDTH)ba;
		}
		else          // load constant
		{
			input[0] = (aa << 3) | ba;
		}

	}
	else         //register mode
	{
		input[0] = pReg->r[aa];
		input[1] = pReg->r[ba]; //register read
	}

	output = ALU_unit(fs, input, pReg->psr, &err);
	if (mw)
	{
		pReg->r[ba] = output;
		data_write(code, size, pReg->r[aa]/*index*/, pReg->r[ba]/*value*/);
	}
	else
		pReg->r[da] = output;

	pReg->pc[0] += br;

	if(err)
		print1n("^R^Invalid instruction!");

_process_exit:
	return err;
}
