/*********************************************************
 * include Header file
 *********************************************************/
#include "common.h"
#include "image.h"

/*********************************************************
 * Constant & Type Definitions
 *********************************************************/
#define DBG_ON			 0
#define IMG_ON			 0

#define R_BIT		0x8000 //register instruction
#define I_BIT		0x4000 //constant instruction
#define M_BIT		0x2000 //memory instruction
#define J_BIT		0x1000 //brach instruction
#define N_BIT		0xFFFF // None

//memory operation
#define R_MEM		0x0800 //read from memory
#define W_MEM		0x0400 //read to memory

//jump operation
#define J_CONT		0x0200

#define CODE_SEC	0x0100
#define DATA_SEC	0x0080

#define SEC_TXT		".TEXT"
#define SEC_DAT		".DATA"

typedef struct _inst
{
	char     *s;
	int      num;
	int      op_n;
	BUSWIDTH cod;
	BUSWIDTH in;
} INST_INFO;

#define LINE_BUF_SIZ		( 500)

#define CR					(0x0d)
#define LF					(0x0a)
#define TAB					(0x09)
#define SPACE				(0x20)

#define LABEL_CNT			( 100)

/*********************************************************
 * Static Variables declareration
 *********************************************************/
static char lineBuf[LINE_BUF_SIZ] = {'\0',};

static char *regs[8] = {
	"R0","R1","R2","R3",
	"R4","R5","R6","R7"
};

static INST_INFO inst_tbl[] = {
	{ "LI" , 2, 2, I_BIT,       0x40 },
	{ "ADI", 3, 3, I_BIT,       0x42 },
	{ "INC", 3, 1, R_BIT,       0x01 },
	{ "DEC", 3, 1, R_BIT,       0x06 },
	{ "ADD", 3, 3, R_BIT,       0x02 },
	{ "LD",  2, 2, M_BIT|R_MEM, 0x30 },
	{ "ST",  2, 2, M_BIT|W_MEM, 0x20 },
	{ "JMP", 3, 1, J_BIT,       0x70 },
	{ "BRZ", 3, 1, J_BIT|J_CONT,0x60 },
	{ "EXT", 3, 0, N_BIT,       0x7F },
	{0}
};

#define CODETBL_MAXSIZ		1000
static CODE_ATTR code_tbl[CODETBL_MAXSIZ];
static int       code_cnt = 0;
static int		 section  = CODE_SEC;

/*********************************************************
 * Static Functions Definition
 *********************************************************/

/**************************
 * Strung Edit functions
 */
static void bufClear(void)
{
	int i;

	for(i = 0; i < LINE_BUF_SIZ; i++)
		lineBuf[i] = '\0';
}

#define READ_DECODE			1
#define READ_NORMAL			2
static int readline (char *line, int option, FILE *fp)
{
	static int line_num = 0;
	char *tmp_line = line;
	int  i;

	while(1)
	{
		char c = (char)fgetc(fp);

		if(c == EOF)
		{
			line_num = 0;
			return -1;
		}
		if(c == CR)
		{
			c = (char)fgetc(fp);
			break;
		}

		if(option == READ_DECODE)
			c = (char)toupper(c);

		*(tmp_line++) = c;
	}
	*(tmp_line++) = '\0';

	line_num++;
	return (line_num);
}

static char *strtok2(char *srcStr, char *del, char **ptr)
{
	char	*resStr;
	size_t	srcLen, resLen;

	if (del == NULL)
	{
		del = (char *)"\t ";
	}

	/* Normal Type Of strtok	*/
	if (ptr == NULL)
	{
		return((char*)strtok(srcStr, del));
	}

	if (srcStr && *srcStr)
	{
		srcLen = strlen(srcStr);
		resStr = (char*)strtok(srcStr, del);
		resLen = strlen(resStr);
	}
	else
	{
		srcStr = NULL;
		srcLen = 0;
		resStr = NULL;
		resLen = 0;
	}

	if (resStr && ((srcStr+srcLen) > (resStr+resLen)))
		*ptr = (resStr + resLen + 1);
	else
		*ptr = NULL;
	return(resStr);
}

static int str_cnt(char *s, char c)
{
	int  cnt  = 0;
	char *p = s;

	while(*p != '\0')
	{
		if(*p == c)
			cnt++;
		p++;
	}

	return cnt;
}

/**********************************
 * Program Data Encoding Functions
 */
static int check_inst(char *sCode, char *cmp, int cnt)
{
	char *p = sCode;

	while(*p==SPACE||*p==TAB)
		p++;

	return (strncmp(p, cmp, cnt));
}

static int isCode(char *sCode, int *idx)
{
	int i =0;
	int cnt;
	char *check = NULL;

	while(inst_tbl[i].s != 0)
	{
		cnt = inst_tbl[i].num;
		if(!check_inst(sCode, inst_tbl[i].s, cnt))
		{
			*idx = i;
			if(section != CODE_SEC)
				return 0;
			return FLAG_CODE;
		}
		else
		{
			cnt = 5;
			if(!check_inst(sCode, ".WORD", cnt))
			{
				*idx = 0xf;
				if(section != DATA_SEC)
					return 0;
				return FLAG_DATA;
			}
		}
		i++;
	}

	*idx = 0xf;
	return 0;
}

static int get_reg(char *sCode, char **sCode2)
{
	char tok = 'R';
	char *p  = sCode;
	int i;

	while(*p != '\0')
	{
		if(p != NULL && *p == tok)
		{
			if(++p != NULL)
			{
				int num;
				num = *p -'0';
				if(num >= 0 && num <= 7)
				{
					*sCode2 = p+1;
					return num;
				}
			}
		}
		p++;
	}

	return -1;
}

static int get_num(char *sCode, char **sCode2)
{
	char tok = '#';
	char *p = sCode;

	while(*p != '\0')
	{
		if(p != NULL && *p == tok)
		{
			*sCode2 = p+1;
			return 0;
		}
		p++;
	}

	return -1;
}

static int encode_txt(char *sCode, char *s_label, INST_INFO inst, int idx, int lineNum, int *err)
{
	BUSWIDTH  _code = 0;
	BUSWIDTH  _nm = 0;
	CODE_ATTR *pCodAttr = NULL;
	int       i;
	char      *str;
	int       rn[3], cons, loop;


	if( idx >=CODETBL_MAXSIZ)
	{
		print1n("^R^Code size is over!");
		exit(4);
	}

	pCodAttr = &code_tbl[idx];

	str = sCode;

	_code = (inst.in << 9);

	if(!strncmp("EXT", str, 3))
	{
		_code = 0xFFFF;
		pCodAttr->code    = _code;
		goto _endcode_end;
	}

	if      (inst.cod & I_BIT) // constant mode
	{
		for(loop = 0; loop < inst.op_n-1; loop++)
		{
			rn[loop] = get_reg(str, &str);
			if(rn[loop] == -1)
			{
				print1n("^R^Invalid operand!");
				exit(5);
			}
		}

		if(get_num(str, &str)) //number 가 아닐때
		{
			print1n("^R^Invalid operand!");
			exit(5);
		}

		cons = (int)strtoul(str, 0, 10);

		if(inst.op_n == 3) _nm = (rn[0] << 6) | (rn[1] << 3) | cons;
		else               _nm = (rn[0] << 6) | cons;

		_code |= _nm;
		pCodAttr->code    = _code;
		strcpy(pCodAttr->op_label, str);
	}
	else if (inst.cod & R_BIT) // register mode
	{
		for(loop = 0; loop < inst.op_n; loop++)
		{
			rn[loop] = get_reg(str, &str);
			if(rn[loop] == -1)
			{
				print1n("^R^Invalid operand!");
				exit(5);
			}
		}

		if     (inst.op_n == 3) { _nm = (rn[0] << 6) | (rn[1] << 3) | rn[2]; }
		else if(inst.op_n == 2) { _nm = (rn[0] << 6) | (rn[1] << 3);         }
		else if(inst.op_n == 1) { _nm = (rn[0] << 6) | (rn[0] << 3);         }
		else                    { print1n("^R^Invalid operand!"); exit(10);  }


		_code |= _nm;
		pCodAttr->code    = _code;
		pCodAttr->op_label[0] = '\0';

	}
	else if (inst.cod & M_BIT) // memory mode
	{
		for(loop = 0; loop < inst.op_n; loop++)
		{
			if(loop == 1) //memory 인지 check 즉 '[' , ']' 를 체크
			{
				char *chk = NULL;
				chk = strchr(str, '[');
				if(chk == NULL)
				{
					print1n("^R^Invalid operand(not a memory!");
					exit(5);
				}
				chk = strchr(str, ']');
				if(chk == NULL)
				{
					print1n("^R^Invalid operand(not a memory!");
					exit(5);
				}
			}
			rn[loop] = get_reg(str, &str);
			if(rn[loop] == -1)
			{
				print1n("^R^Invalid operand!");
				exit(5);
			}
		}

		if (inst.cod & R_MEM) //memory read
		{
			_nm = (rn[0] << 6)|(rn[1] << 3);
		}
		else                  //memory write
		{
			_nm = (rn[1] << 3)|(rn[0] << 0);
		}
		_code |= _nm;
		pCodAttr->code    = _code;
		pCodAttr->op_label[0] = '\0';
	}
	else if (inst.cod & J_BIT) // jump mode
	{
		if(inst.cod & J_CONT) //conditional jump
		{
			rn[0] = get_reg(str, &str);
			if(rn[0] == -1)
			{
				*err = -1;
				return idx;
			}

			while(*str == ','||*str == TAB||*str == SPACE)
				str++;

			if(*str == '\0')
			{
				print1n("^R^Operand is not label");
				exit(5);
			}
			strcpy(pCodAttr->op_label, str);
		}
		else
		{
			for(loop = 0; loop < inst.num; loop++, str++);

			while(*str == ','||*str == TAB||*str == SPACE)
				str++;

			if(*str == '\0')
			{
				print1n("^R^Operand is not label");
				exit(5);
			}
			strcpy(pCodAttr->op_label, str);
		}
		pCodAttr->code    = _code;
	}
	else
	{
		print1n("^R^Invalid instruction");
		exit(6);
	}

_endcode_end:
	if(s_label != NULL) strcpy(pCodAttr->ln_label, s_label);
	else                pCodAttr->ln_label[0] = '\0';

	pCodAttr->cd_flag = FLAG_CODE;
	pCodAttr->addr    = idx*sizeof(BUSWIDTH);
	pCodAttr->lineNum = lineNum;

	*err = 0;
	return (idx+1);
}

static int encode_data(char *sCode, char *s_label, int idx, int lineNum, int *err)
{
	char       *str = sCode;
	CODE_ATTR *pCodAttr = NULL;

	while (*str == TAB||*str == SPACE||*str == ',')
		str++;

	if(strncmp(".WORD", str, 5))
	{
		print1n("^R^Invalid Data Type!");
		exit(3);
	}
	str += 5;
	while (*str == TAB||*str == SPACE||*str == ',')
		str++;

	pCodAttr = &code_tbl[idx];

	pCodAttr->code = (BUSWIDTH)strtoul(str, 0, 16);

_endcode_end:
	if(s_label != NULL) strcpy(pCodAttr->ln_label, s_label);
	else                pCodAttr->ln_label[0] = '\0';

	pCodAttr->cd_flag = FLAG_DATA;
	pCodAttr->addr    = idx*sizeof(BUSWIDTH);
	pCodAttr->lineNum = lineNum;

	*err = 0;
	return (idx+1);
}

static int parse(char *line, int line_num)
{
	char *second_line, *delete_str;
	static char *sLabel = NULL;
	static char savdLabel[10];
	char *sCode, *sCode2;
	int  index;
	int  err;

	/* Null line */
	while(*line == TAB|| *line == SPACE)
		line++;
	if(*line == '\0')
		return 0;

	/* Comment line */
	if(line[0] == '#' || line[0] == ';')
		return 0;
	/* comment delete */
	strtok2(line, ";", &delete_str);

	/* trim space */
	delete_str = line;
	while(*delete_str != '\0')
		delete_str++;
	delete_str--;

	while(*delete_str == TAB||*delete_str == SPACE)
	{
		*(delete_str--) = '\0';
	}

	if(!strcmp(line, SEC_TXT))
	{
		section = CODE_SEC;
		return 0;
	}

	if(!strcmp(line, SEC_DAT))
	{
		section = DATA_SEC;
		return 0;
	}

	strtok2(line, ":", &second_line);

	if( second_line != NULL)
	{
		strcpy(savdLabel, line);
		sLabel = savdLabel;
		sCode  = second_line;
	}
	else
	{
		sCode  = line;
	}

	while(*sCode==SPACE||*sCode==TAB)
		sCode++;

	switch(isCode(sCode, &index))
	{
		case FLAG_CODE:
			code_cnt = encode_txt(sCode, sLabel, inst_tbl[index], code_cnt, line_num, &err);
			sLabel = NULL;
			break;
		case FLAG_DATA:
			code_cnt = encode_data(sCode, sLabel, code_cnt,line_num,  &err);
			sLabel = NULL;
			break;
	}

	return 0;
}

static void first_pass(char *fname)
{
	FILE *fp;
	int  line_num = 0;

	if((fp = fopen(fname, "r")) == NULL)
	{
		print1n("^R^%s can not open!", fname);
		exit(1);
	}

	//bufClear();
	while(1)
	{
		if( (line_num = readline(lineBuf, READ_DECODE, fp)) == -1)
			break;

		parse(lineBuf, line_num);

		//bufClear();
	}
	fclose(fp);
}

static void prgmAlign(CODE_ATTR *attr, int cnt)
{
	BUSWIDTH idx = 0;
	int      iter, iter2;

	/*find .text section*/
	for(iter = 0; iter < cnt; iter++)
	{
		CODE_ATTR *c = &attr[iter];
		if(c->cd_flag == FLAG_CODE)
		{
			c->addr = idx*sizeof(BUSWIDTH);
			idx++;
		}
	}

	/*find .data section*/
	for(iter = 0; iter < cnt; iter++)
	{
		CODE_ATTR *c = &attr[iter];
		if(c->cd_flag == FLAG_DATA)
		{
			c->addr = idx*sizeof(BUSWIDTH);
			idx++;
		}
	}

	/*sort*/
	for(iter = 0; iter < cnt; iter++)
	{
		for(iter2 = 1; iter2 < cnt; iter2++)
		{
			CODE_ATTR tmp;
			CODE_ATTR *c1, *c2;

			c1 = &attr[iter2-1];
			c2 = &attr[iter2];

			if(c1->addr > c2->addr)
			{
				/*swap*/
				//(1)
				tmp.code    = c1->code;
				tmp.addr    = c1->addr;
				tmp.cd_flag = c1->cd_flag;
				tmp.lineNum = c1->lineNum;
				strcpy(tmp.ln_label, c1->ln_label);
				strcpy(tmp.op_label, c1->op_label);
				//(2)
				c1->code    = c2->code;
				c1->addr    = c2->addr;
				c1->cd_flag = c2->cd_flag;
				c1->lineNum = c2->lineNum;
				strcpy(c1->ln_label, c2->ln_label);
				strcpy(c1->op_label, c2->op_label);
				//(3)
				c2->code    = tmp.code;
				c2->addr    = tmp.addr;
				c2->cd_flag = tmp.cd_flag;
				c2->lineNum = tmp.lineNum;
				strcpy(c2->ln_label, tmp.ln_label);
				strcpy(c2->op_label, tmp.op_label);
			}
		}
	}

	return;
}

static int second_pass(CODE_ATTR *attr, int cnt)
{
	int       loop1, loop2;
	CODE_ATTR *c, *d;

	prgmAlign(attr, cnt);

	for(loop1 = 0; loop1 < cnt; loop1++)
	{
		c = &attr[loop1];

		for(loop2 = 0; loop2 < cnt; loop2++)
		{
			d = &attr[loop2];
			//operand의 label과 link label이 같으면...
			if( (c->op_label[0] != '\0') && !strcmp(d->ln_label, c->op_label) )
			{
				int    sloop = 0;
				BUSWIDTH addr  = d->addr;

				switch((c->code >> 9) & 0x7F)
				{
					case 0x40: /* LI*/
						c->code |= (addr & 0x3F);
						break;
					case 0x42: /*ADI*/
						print1n("^R^invalid operand cna't located label!");
						exit(7);
					case 0x60: /*BRZ*/
						{
							BUSWIDTH tadr;

							if      (c->addr > addr)
							{
								c->code |= 0x0200; //bc의 최하위 bit를 set한다.
								tadr = c->addr - addr;
								tadr = (~tadr & 0xFFFF) + 1;
							}
							else if (c->addr == addr)
							{
								tadr = 0;
							}
							else
							{
								tadr = (addr - c->addr);
							}

							addr  = (tadr & 0x38) <<3;
							addr |= (tadr & 0x7);
							c->code |= addr;
						}
						break;
					case 0x70: //JMP
						c->code |= addr & 0x1FF;
						break;
					default:
						print1n("^R^Invalid label!");
						exit(8);
				}
			}
		}
	}

	return 0;
}

/*********************************************************
 * Global Functions Definition
 *********************************************************/
int process (char *pName, int opt_flag)
{
	BUSWIDTH *buf = NULL, *pTbuf = NULL;
	FILE     *fp;
	char     *out, *p_tmp;
	int      i, cnt;
	int      buf_size;
	IMG_HDR  hdr;
	BUSWIDTH *phdr, *pCd;

	first_pass(pName);
	second_pass(&code_tbl[0], code_cnt);

	out = pName;
	cnt = str_cnt(out, '/');

	for(i = 0; i < cnt; i++)
		strtok2(out, "/", &out);
	strtok2(out, ".", &p_tmp);

	if(opt_flag & OPT_DBG)
	{
		strcat(out, ".img");
		buf_size = sizeof(IMG_HDR)+(sizeof(CODE_ATTR)+sizeof(BUSWIDTH))*code_cnt;

		hdr.magic   = HEADER_MAGIC;
		hdr.head_sz = sizeof(IMG_HDR);
		hdr.sym_sz  = sizeof(CODE_ATTR)*code_cnt;
		hdr.data_sz = sizeof(BUSWIDTH)*code_cnt;
	}
	else
	{
		strcat(out, ".bin");
		buf_size = sizeof(BUSWIDTH)*code_cnt;
	}

	if((fp = fopen(out, "wb")) == NULL)
	{
		print1n("^R^%s can not open!", out);
		exit(1);
	}

	if(!code_cnt)
	{
		print1n("^R^Code can't create from %s!", pName);
		return -1;
	}

	pTbuf = buf = (BUSWIDTH*)malloc(buf_size);

	/*
	print1n("^B^code count:%2d", code_cnt);
	*/
	if(opt_flag & OPT_DBG)
	{
		int hdr_sz    = sizeof(IMG_HDR)/sizeof(BUSWIDTH);
		int cd_tbl_sz = (sizeof(CODE_ATTR)*code_cnt)/sizeof(BUSWIDTH);

		phdr = (BUSWIDTH*)&hdr;

		for(i = 0; i < hdr_sz; i++)
			*(pTbuf++) = phdr[i];

		pCd  = (BUSWIDTH*)&code_tbl[0];

		for(i = 0; i < cd_tbl_sz; i++)
			*(pTbuf++) = pCd[i];
	}

	for(i = 0; i < code_cnt; i++)
	{
		CODE_ATTR *c = &code_tbl[i];

		*(pTbuf++) = c->code;
		if(opt_flag & OPT_SHOW)
		{
			print1n("^g^[%2d]:0x%04x, %s link, operand: %8s, %8s, lineNum: %3d",
				c->addr,
				c->code,
				(c->cd_flag == FLAG_CODE)?"code":"data",
				(*c->ln_label == '\0')?"null":c->ln_label,
				(*c->op_label == '\0')?"null":c->op_label,
				c->lineNum
			  );
		}
	}

	#if (IMG_ON > 0)
	print1n("^Y^header size: (%x,%d), code_tbl(%x,%d), code(%x,%d)",
	        sizeof(IMG_HDR),
	        sizeof(IMG_HDR),
	        sizeof(CODE_ATTR),
	        sizeof(CODE_ATTR),
			code_cnt,
			code_cnt
			);
	for(i = 0; i < buf_size/sizeof(BUSWIDTH); i++)
		print1n("[%03d]:0x%04x", i, buf[i]);
	#endif

	fwrite((void*)buf, 1, buf_size, fp);

	free(buf);
	fclose(fp);

	return 0;
}

static int getAddrfromLineNum(int lineNum)
{
	int       i, count = code_cnt;

	for (i = 0; i < count; i++)
	{
		CODE_ATTR *c = &code_tbl[i];

		if(c->lineNum == lineNum)
			return ((int)c->addr);
	}

	return -1;
}

int mkListfile(char *fname, int print_flag)
{
	FILE *fp, *op;
	char *out, *p_tmp;
	int  cnt, i;

	if((fp = fopen(fname, "r")) == NULL)
	{
		print1n("^R^%s can not open!", fname);
		exit(1);
	}

	out = fname;
	cnt = str_cnt(out, '/');

	for(i = 0; i < cnt; i++)
		strtok2(out, "/", &out);
	strtok2(out, ".", &p_tmp);

	strcat(out, ".list");

	if((op = fopen(out, "w")) == NULL)
	{
		print1n("^R^%s can not open!", out);
		fclose(fp);
		exit(1);
	}

	while(1)
	{
		char num_prnt[10];
		int  line_num;
		int  addr;

		if( (line_num = readline(lineBuf, READ_NORMAL, fp)) == -1)
			break;

		addr = getAddrfromLineNum(line_num);
		if(addr == -1) sprintf(num_prnt, "        ");
		else           sprintf(num_prnt, ">%03d : ", addr);

		fprintf(op, "%8s%s\n", num_prnt, lineBuf);

		if(print_flag)
			print1n("%8s%s", num_prnt, lineBuf);

		//bufClear();
	}

	//bufClear();

	fclose(op);
	fclose(fp);

	return 0;
}
