#include	"common.h"
#include	"image.h"


#define OPTSTR		"h|g|s|b|L]"

static int	opt_flag = 0;

static void Usage(void)
{
	printf("Usage: com-as [options] file...\n");
	printf("Options:\n");
	printf(" %-20s Print this menu\n", "-h");
}

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
	int  i = 0;
	int  c;
	char *name = argv[argc-1];

	do
	{
		switch(c = findopt(argv[i], OPTSTR))
		{
			case 'g':
				opt_flag |= OPT_DBG;
				break;
			case 'b':
				opt_flag |= OPT_BIN;
				break;
			case 's':
				opt_flag |= OPT_SHOW;
				break;
			case '?':
			case 'h':
				opt_flag |= OPT_HLP;
				printf("fff\n");
				break;
			case 'L':
				opt_flag |= OPT_LIST;
				break;
			default :
				opt_flag = 0;
				return NULL;
		}
		i++;
	} while(i < (argc-1));

	return (name);
}

int main (int argc, char *argv[])
{
	extern int process(char *pName, int debug_flag);
	char *name      = "";
	char *savedname = "";
	int  r_val    = 0;


	if(argc == 1)
	{
		Usage();
		return -1;
	}

	if( (name = get_fname(argc-1, &argv[1])) == NULL)
	{
		opt_flag = OPT_BIN;
	}

	if(opt_flag & OPT_HLP)
	{
		Usage();
		return 0;
	}

	savedname = (char*)malloc(strlen(name)+10);
	if(savedname != NULL)
		strcpy(savedname, name);

	/* if "-b" option enable then debug option is discarded! */
	if(opt_flag & OPT_BIN)
		opt_flag &= ~OPT_DBG;

	r_val = process(name, opt_flag);

	name = savedname;
	if(opt_flag & OPT_DBG)
	{
		extern int mkListfile(char *fname, int print_flag);
		int prnt = 0;

		if(opt_flag & OPT_LIST)
			prnt = 1;
		mkListfile(name, prnt);
	}

	if(name != NULL)
		free(name);

	return r_val;
}
