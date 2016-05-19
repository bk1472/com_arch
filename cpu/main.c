#include "common.h"
#include "image.h"
#include "registers.h"
#include "cpu.h"

void initRegs(REGISTER_T *pRegs)
{
	pRegs->r[0] = 0x0000;
	pRegs->r[1] = 0x1111;
	pRegs->r[2] = 0x2222;
	pRegs->r[3] = 0x3333;
	pRegs->r[4] = 0x4444;
	pRegs->r[5] = 0x5555;
	pRegs->r[6] = 0x6666;
	pRegs->r[7] = 0x7777;
	pRegs->psr[0] = 0;
	pRegs->pc[0]  = 0;
}

#define OPTSTR		"d|h|s]"

static UINT08	opt_flag = 0;

int findopt(char *arg, char *pOpt)
{
	char *p_opt = pOpt;

	if(arg[0] == '?')
		return ((int)arg[0]);
	if(arg[0] != '-')
		return 0;

	while(*p_opt != ']')
	{
		if(*p_opt == '|') //seperator
			p_opt++;

		if(arg[1] == *p_opt)
			return ((int)arg[1]);

		p_opt++;
	}

	return 0;
}

char *get_fname(int argc, char *argv[])
{
	int  i;
	int  c;
	char *name = argv[argc-1];

	if(argc == 1)
	{
		opt_flag = 0;
		return name;
	}


	for(i = 0; i < (argc-1); i++)
	{
		switch(c = findopt(argv[i], OPTSTR))
		{
			case 'd':
				opt_flag |= OPT_DBG;
				break;
			case 's':
				opt_flag |= OPT_SHOW;
				break;
			case '?':
			case 'h':
				opt_flag |= OPT_HLP;
				break;
			default :
				opt_flag = 0;
				return NULL;
		}
	}

	return (name);
}


static BUSWIDTH *code_data = NULL;
static int    code_size  = 0;

static int file_operation(char* name, BUSWIDTH **ppCodebuf, int *pSize)
{
	FILE	*fp;
	int		err = 0;

	if((fp = fopen(name, "rb")) == NULL)
	{
		print1n("^R^[%s]file open error!", name);
		err = -1;
		goto _Err_Handle;
	}

	fseek(fp, 0, SEEK_END);
	*pSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if( (*ppCodebuf = (BUSWIDTH*)malloc(*pSize)) == NULL )
	{
		print1n("^R^Code memory allocation Error!");
		err = -2;
		goto _Err_Handle;
	}

	if(fread((void*)(*ppCodebuf), 1, *pSize, fp) != *pSize)
	{
		print1n("^R^code read to memory error!");
		err = -3;
	}

_Err_Handle:
	if(err)
	{
		if(err == -3)
			free(*ppCodebuf);
		*pSize = 0;
		*ppCodebuf = NULL;
	}
	fclose(fp);

	return err;
}

int main (int argc, char *argv[])
{
	extern int  cpu_main( BUSWIDTH code[],  int size, REGISTER_T *pReg);
	extern void hexdump(const char *name, void *vcp, int width, int size);
	extern void printRegs(REGISTER_T *pReg);
	extern int  debug_cpu(REGISTER_T *pReg, BUSWIDTH *codeTbl, int size, CODE_ATTR  *code_tbl);

	FILE       *fp;
	char       *name;
	int        err;
	int        iter, size;
	int        dbg_mode = 0;
	BUSWIDTH   *pBinPtr = NULL;
	REGISTER_T regs;
	IMG_HDR    *pHdr = NULL;
	CODE_ATTR  *tbl  = NULL;

	if(argc == 1)
	{
		print1n("^R^Input parameter error!");
		return -1;
	}

	initRegs(&regs);

	if( (name = get_fname(argc-1, &argv[1])) == NULL)
	{
		print1n("^R^Input parameter style is error!");
		return -1;
	}

	if(opt_flag & OPT_DBG)
		dbg_mode = 1;

	if(file_operation(name, &code_data, &size))
	{
		exit(1);
	}

	code_size = size/sizeof(BUSWIDTH);


	pHdr = (IMG_HDR*)code_data;

	if(pHdr->magic == HEADER_MAGIC)
	{
		int image_size;
		int cod_tbl_st;

		image_size  = (int)(pHdr->head_sz + pHdr->sym_sz);
		image_size /= sizeof(BUSWIDTH);
		code_size  -= image_size;

		cod_tbl_st  = pHdr->head_sz / sizeof(BUSWIDTH);

		pBinPtr = &code_data[image_size];

		tbl = (CODE_ATTR*)&code_data[cod_tbl_st];
	}
	else
	{
		opt_flag &= ~OPT_SHOW;
		pBinPtr = &code_data[0];
	}

	if(opt_flag & OPT_SHOW)
	{

		int cod_cnt = pHdr->sym_sz  / sizeof(CODE_ATTR);

		for( iter = 0; iter < cod_cnt; iter++ )
		{
			CODE_ATTR *c = &tbl[iter];

			print1n("^g^[%2d]:0x%04x, %s link, operand: %8s, %8s, lineNum: %3d",
				c->addr,
				c->code,
				(c->cd_flag == FLAG_CODE)?"code":"data",
				(*c->ln_label == '\0')?"null":c->ln_label,
				(*c->op_label == '\0')?"null":c->op_label,
				c->lineNum
			  );
		}
		print1n("");
	}

	printRegs(&regs);
	hexdump("Before", pBinPtr, sizeof(BUSWIDTH), code_size*sizeof(BUSWIDTH));

	while(1)
	{
		if(dbg_mode) err = debug_cpu(&regs, pBinPtr, code_size + 2, tbl);
		else         err = cpu_main(pBinPtr, code_size + 2, &regs);

		if      (err == -5) //debug mode exit
			dbg_mode = 0;
		else if (err) //end condition
			break;
	}

	if(err == RET_END)
	{
		printRegs(&regs);
		hexdump("After", pBinPtr, sizeof(BUSWIDTH), code_size*sizeof(BUSWIDTH));
	}

	free(code_data);
	return 0;
}
