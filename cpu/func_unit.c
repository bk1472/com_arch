#include "common.h"
#include "cpu.h"

UINT32 inst_decoder(BUSWIDTH inst)
{
	#define MK_DA		(cw[DA_IDX] << 20)
	#define MK_AA		(cw[AA_IDX] << 17)
	#define MK_BA		(cw[BA_IDX] << 14)
	#define MK_MB		(cw[MB_IDX] << 13)
	#define MK_FS		(cw[FS_IDX] <<  8)
	#define MK_MD		(cw[MD_IDX] <<  7)
	#define MK_RW		(cw[RW_IDX] <<  6)
	#define MK_MW		(cw[MW_IDX] <<  5)
	#define MK_PL		(cw[PL_IDX] <<  4)
	#define MK_JB		(cw[JB_IDX] <<  3)
	#define MK_BC		(cw[BC_IDX] <<  0)

	UINT32 not_MD, not_MB, not_JB;
	UINT32 cw[CTRL_SIZ];

	cw[DA_IDX] = (UINT32)((inst >>  6) & 0x7);
	cw[AA_IDX] = (UINT32)((inst >>  3) & 0x7);
	cw[BA_IDX] = (UINT32)((inst >>  0) & 0x7);
	cw[MB_IDX] = (UINT32)((inst >> 15) & 0x1);
	cw[MD_IDX] = (UINT32)((inst >> 14) & 0x1);
	cw[JB_IDX] = (UINT32)((inst >> 13) & 0x1);
	cw[BC_IDX] = (UINT32)((inst >>  9) & 0x7);

	not_MD = (~cw[MD_IDX] & 0x1);
	not_MB = (~cw[MB_IDX] & 0x1);
	not_JB = (~cw[JB_IDX] & 0x1);

	/* RW <= (~MD | (JB & ~MB)) */
	cw[RW_IDX] = not_MD | (cw[JB_IDX] & not_MB);

	/* MW <= (~MB & MD & ~JB  ) */
	cw[MW_IDX] = not_MB & cw[MD_IDX] & not_JB;

	/* PL <= ( MD & MB        ) */
	cw[PL_IDX] = cw[MD_IDX] & cw[MB_IDX];

	cw[FS_IDX] = (UINT32)(cw[PL_IDX])?0:((inst >> 9) & 0x1f);

	return(MK_DA|MK_AA|MK_BA|MK_MB|MK_FS|MK_MD|MK_RW|MK_MW|MK_PL|MK_JB|MK_BC);
}

static BUSWIDTH mem_read(BUSWIDTH code[], int size, BUSWIDTH loc)
{
	int index = (int)(loc/sizeof(BUSWIDTH));

	if( index < size)
		return (code[index]);

	return 0;
}

static int mem_write(BUSWIDTH code[], int size, BUSWIDTH loc, BUSWIDTH value)
{
	int index = (int)(loc/sizeof(BUSWIDTH));

	if( index < size)
	{
		code[index] = value;
		return 0;
	}

	return -1;
}

BUSWIDTH inst_fetch(BUSWIDTH code[], int size, BUSWIDTH pc)
{
	return mem_read(code, size, pc);
}

BUSWIDTH data_read(BUSWIDTH code[], int size,  BUSWIDTH loc)
{
	return mem_read(code, size, loc);
}

int data_write(BUSWIDTH code[], int size,  BUSWIDTH loc, BUSWIDTH value)
{
	return mem_write(code, size, loc, value);
}

static BUSWIDTH detec_state(int carry, int msb, BUSWIDTH value)
{
	int v = 0;
	int c = 0;
	int n = 0;
	int z = 0;
	BUSWIDTH ret = 0;

	//carry detect
	c = (carry)?1:0;

	//over flow detect
	v = (carry ^ msb);

	//negative detect
	if (v) n = (msb)?0:1;
	else   n = (msb)?1:0;

	//zero detect
	z = (value)?0:1;

	if (c) ret |= C_BIT;
	if (v) ret |= V_BIT;
	if (z) ret |= Z_BIT;
	if (n) ret |= N_BIT;

	return ret;
}

static BUSWIDTH calculation(UINT32 input, BUSWIDTH *state)
{
	int c, msb;

	c      = (input & 0x10000)?1:0;
	msb    = (input & 0x08000)?1:0;

	input &= 0xFFFF;

	*state = detec_state(c, msb, (BUSWIDTH)input);

	return ((BUSWIDTH)input);
}

static BUSWIDTH calc_add (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 + a2;

	return (calculation(v, state));
}

static BUSWIDTH calc_add_plus (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 + a2 + 1;

	return (calculation(v, state));
}

static BUSWIDTH calc_nadd (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	a2  = ~a2 & 0xFFFF;

	v   = a1 + a2;

	return (calculation(v, state));
}

static BUSWIDTH calc_nadd_plus (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	a2  = ~a2 & 0xFFFF;

	v   = a1 + a2 +1;

	return (calculation(v, state));
}

static BUSWIDTH calc_and (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 & a2;

	return (calculation(v, state));
}

static BUSWIDTH calc_or (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 | a2;

	return (calculation(v, state));
}

static BUSWIDTH calc_xor (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 ^ a2;

	return (calculation(v, state));
}

static BUSWIDTH calc_sr (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;
	UINT32 c;

	c = (a1 & 1)?0x10000:0;
	v   = a1 >>1;
	v  += c;

	return (calculation(v, state));
}

static BUSWIDTH calc_sl (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1 <<1;

	return (calculation(v, state));
}

static BUSWIDTH calc_mova (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a1;
	v  &= 0xFFFF;

	return (calculation(v, state));
}

static BUSWIDTH calc_movb (UINT32 a1, UINT32 a2, BUSWIDTH *state)
{
	UINT32 v;

	v   = a2;
	v  &= 0xFFFF;

	return (calculation(v, state));
}

typedef struct _fs_func
{
	UINT32 fs;
	UINT32 arg[2];
	BUSWIDTH (*func)(UINT32, UINT32, BUSWIDTH*);
} FS_FUNC;


#define ARG_MOVE		(0xa5)
#define ARG_NOT			(0x3c)
#define ARG_NONE		(0xFF)

#define FS_FUNCTION_TBL									\
		{0x00, ARG_MOVE, ARG_NONE, calc_mova     },		\
		{0x01, ARG_MOVE,        1, calc_add      },		\
		{0x02, ARG_MOVE, ARG_MOVE, calc_add      },		\
		{0x03, ARG_MOVE, ARG_MOVE, calc_add_plus },		\
		{0x04, ARG_MOVE, ARG_MOVE, calc_nadd     },		\
		{0x05, ARG_MOVE, ARG_MOVE, calc_nadd_plus},		\
		{0x06, ARG_MOVE,   0xFFFF, calc_add      },		\
		{0x07, ARG_MOVE, ARG_NONE, calc_mova     },		\
		{0x08, ARG_MOVE, ARG_MOVE, calc_and      },		\
		{0x0A, ARG_MOVE, ARG_MOVE, calc_or       },		\
		{0x0C, ARG_MOVE, ARG_MOVE, calc_xor      },		\
		{0x0E, ARG_NOT , ARG_NONE, calc_mova     },		\
		{0x10, ARG_NONE, ARG_MOVE, calc_movb     },		\
		{0x14, ARG_MOVE, ARG_NONE, calc_sr       },		\
		{0x18, ARG_MOVE, ARG_NONE, calc_sl       },		\
		{0xFF      }									\

BUSWIDTH ALU_unit(UINT32 fs_type, BUSWIDTH arg[], BUSWIDTH *state, int *err)
{
	FS_FUNC fsFunc[] = {FS_FUNCTION_TBL};
	BUSWIDTH value;
	int    idx = 0;

	while(fsFunc[idx].fs != 0xFF)
	{
		int cnt;
		UINT32 arg1, arg2;

		if(fsFunc[idx].fs == fs_type)
		{
			for(cnt = 0; cnt < 2; cnt++)
			{
				switch(fsFunc[idx].arg[cnt])
				{
					case ARG_MOVE:
						fsFunc[idx].arg[cnt] = (UINT32)arg[cnt];
						break;
					case ARG_NOT :
						fsFunc[idx].arg[cnt] = (UINT32)(~arg[cnt] & 0xFFFF);
						break;
				}

			}

			arg1  = fsFunc[idx].arg[0];
			arg2  = fsFunc[idx].arg[1];

			value = fsFunc[idx].func(arg1, arg2, state);
			*err = 0;

			return value;
		}
		idx++;
	}

	*err = -1;
	return 0;
}
