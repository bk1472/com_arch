#include "common.h"
#include "image.h"
#include "registers.h"
#include "cpu.h"

static void		dsmInst(char *str, BUSWIDTH inst);
static char		regMsg[4096];

static void setTermialMode(int flag, int mode)
{
	struct termios termAttr;

	tcgetattr(0, &termAttr);
	termAttr.c_cc[VERASE] = 0x08; /* Backspace */

	if (mode & ECHO)
	{
		if (flag & ECHO) termAttr.c_lflag |=  ECHO;
		else             termAttr.c_lflag &= ~ECHO;
	}
	if (mode & ICANON)
	{
		if (flag & ICANON) termAttr.c_lflag |=  ICANON;
		else               termAttr.c_lflag &= ~ICANON;
	}

	tcsetattr(0, TCSANOW, &termAttr);
	return;
}

int getChar(FILE *fp)
{
	int inKey;

RescanInput:
	setTermialMode(0     , ICANON);	// stty -icanon
	inKey = getc(fp);
	setTermialMode(ICANON, ICANON);	// stty icanon

	return(inKey);
}

void printRegs(REGISTER_T *pReg)
{
	sprintf(regMsg, "");
	sprintf(regMsg, "%sr0   %04x   r1   %04x   r2   %04x   r3   %04x\n", regMsg,
	                   pReg->r[0], pReg->r[1], pReg->r[2], pReg->r[3]);
	sprintf(regMsg, "%sr4   %04x   r5   %04x   r6   %04x   r7   %04x\n", regMsg,
	                   pReg->r[4], pReg->r[5], pReg->r[6], pReg->r[7]);
    sprintf(regMsg, "%spsr ->0x%04x\n", regMsg, pReg->psr[0]);
    sprintf(regMsg, "%spc  ->0x%04x\n", regMsg, pReg->pc[0]);

	print1n("^g^%s", regMsg);
}

CODE_ATTR *findSymbol(CODE_ATTR  *code_tbl, BUSWIDTH pc, int size)
{
	int        i;
	CODE_ATTR *c = code_tbl;

	if(c == NULL)
		return NULL;

	for(i = 0; i < size; i++)
	{
		if(c[i].addr == pc)
			return &c[i];
	}
	return NULL;
}

int debug_cpu(REGISTER_T *pReg, BUSWIDTH *codeTbl, int size, CODE_ATTR  *code_tbl)
{
	extern BUSWIDTH inst_fetch	(BUSWIDTH code[], int size, BUSWIDTH pc);
	extern int      cpu_main	(BUSWIDTH code[], int size, REGISTER_T *pReg);
	BUSWIDTH  inst;
	char      dsmStr[1024];
	int       err;
	CODE_ATTR *c = NULL;;

	print1n("\n");

	print1n("^y^>>Before<<");
	printRegs(pReg);
	inst  = inst_fetch(codeTbl, size, pReg->pc[0]);
	print1n("[Instruction : %04x]", inst);
	print0n("[pc:%04x]    :", pReg->pc[0]);

	if(	(c = findSymbol(code_tbl, pReg->pc[0], size)) != NULL)
	{
		char strBuf[30];
		char *strSym = NULL;
		char *tmp;

		strcpy(strBuf, c->ln_label);

		if(*strBuf != '\0') strSym = strcat(strBuf, ":");
		else                strSym = "";

		print0n("^g^%8s", strSym);
		dsmInst(dsmStr, inst);

		tmp = c->op_label;
		while(*tmp != '\0')
		{
			if(*tmp < '0'|| *tmp > '9')
				break;
			tmp++;
		}

		if(*tmp != '\0') strSym = c->op_label;
		else             strSym = "";

		print0n("^r^%8s", strSym);
	}
	else
	{
		dsmInst(dsmStr, inst);
	}

	print1n("\n");
	if(getChar(stdin) == 0x1b/*ESC*/)
	{
		fflush(stdout);
		print1n(" [ESC] Key input debug Exit!");
		return -5;
	}

	err = cpu_main(codeTbl, size, pReg);
	print1n("^y^>>After<<");
	printRegs(pReg);

	return err;
}

#define I_BIT			(0x8000)
#define R_BIT			(0x4000)
#define M_BIT			(0x2000)
#define J_BIT			(0x1000)

typedef void (*pPRNTFUNC)(char *, BUSWIDTH);

typedef struct {
	char      *inst;
	int       op;
	int       mode;
	pPRNTFUNC func;
} INST_DECOD_TBL;

static void op_load_iarg2(char *str, BUSWIDTH value)
{
	int r, ct;

	r  = (value >>6) & 0x7;
	ct = value & 0x3F;

	sprintf(str, "%sr%d, #%d", str,  r, ct);
}

static void op_load_iarg3(char *str, BUSWIDTH value)
{
	int r1, r2, ct;

	r1 = (value >>6) & 0x7;
	r1 = (value >>3) & 0x7;
	ct = value & 0x7;

	sprintf(str, "%sr%d, r%d, #%d", str, r1, r2, ct);
}

static void op_load_rarg1(char *str, BUSWIDTH value)
{
	int r;

	r = (value >>3) & 0x7;

	sprintf(str, "%sr%d", str, r);
}

static void op_load_rarg3(char *str, BUSWIDTH value)
{
	int r1, r2, r3;

	r1 = (value >>6) & 0x7;
	r2 = (value >>3) & 0x7;
	r3 =  value & 0x7;

	sprintf(str, "%sr%d, r%d, r%d", str, r1, r2, r3);
}

static void op_load_rmarg2(char *str, BUSWIDTH value)
{
	int r1, r2;

	r1 = (value >>6) & 0x7;
	r2 = (value >>3) & 0x7;

	sprintf(str, "%sr%d, [r%d]", str, r1, r2);
}

static void op_load_wmarg2(char *str, BUSWIDTH value)
{
	int r1, r2;

	r1 = (value) & 0x7;
	r2 = (value >>3) & 0x7;

	sprintf(str, "%sr%d, [r%d]", str, r1, r2);
}

static void op_load_jarg1(char *str, BUSWIDTH value)
{
	int addr;

	addr = (value) & 0x1ff;
	sprintf(str, "%saddr(pc <= 0x%04x)", str, addr);
}

static void op_load_jarg2(char *str, BUSWIDTH value)
{
	UINT32 addr;
	int    r;
	int sign = (value & 0x0200)?1:0;

	r     = (value>>6) & 0x7;

	addr  = (UINT32)(value >>3) & 0x38;
	addr |= (UINT32)(value & 0x7);

	if(sign)
		addr = (~0x3F) | addr;

	sprintf(str, "%sr%d, addr(pc+0x%04x)", str, r, addr);


}

static INST_DECOD_TBL decode_tbl[] =
{
	{ "li     ", 0x40, I_BIT , op_load_iarg2 },
	{ "adi    ", 0x42, I_BIT , op_load_iarg3 },
	{ "inc    ", 0x01, R_BIT,  op_load_rarg1 },
	{ "dec    ", 0x06, R_BIT , op_load_rarg1 },
	{ "add    ", 0x02, R_BIT , op_load_rarg3 },
	{ "ld     ", 0x30, M_BIT , op_load_rmarg2},
	{ "st     ", 0x20, M_BIT , op_load_wmarg2},
	{ "jmp    ", 0x70, J_BIT , op_load_jarg1 },
	{ "brz    ", 0x60, J_BIT , op_load_jarg2 },
	{ "ext    ", 0x7F, 0xFFFF, NULL          },
	{ 0}
};

static void dsmInst(char *str, BUSWIDTH inst)
{
	char dsmBuf[1024] = {'\0',};
	int  idx = 0;

	while(decode_tbl[idx].inst != 0)
	{
		int cmp = (int)(inst >> 9);

		if(cmp == decode_tbl[idx].op)
		{
			sprintf(dsmBuf, "%s", decode_tbl[idx].inst);

			if(decode_tbl[idx].func != NULL)
				decode_tbl[idx].func(dsmBuf, inst);
			break;
		}
		idx++;
	}

	if(dsmBuf[0] != '\0')
		print0n("^y^ %s ", dsmBuf);

	return;
}
