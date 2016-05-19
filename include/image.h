#ifndef __IMAGE_HEADER__
#define __IMAGE_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define FLAG_CODE	(1)
#define FLAG_DATA	(2)
#define FLAG_NONE	(3)
typedef struct _code_tbl
{
	BUSWIDTH code;
	int      cd_flag;
	char     ln_label[28]; // label 과 같다면 label 이름을 저장.
	char     op_label[28]; // constant가 아니라 string일때 그것이 label이므로.
	BUSWIDTH addr;
	int      lineNum;
} __attribute__((packed)) CODE_ATTR;

#define HEADER_MAGIC		0x5a9bf1c3
typedef struct {
	UINT32 magic;
	UINT32 head_sz;
	UINT32 sym_sz;
	UINT32 data_sz;
} IMG_HDR;

#define OPT_DBG		0x80
#define OPT_BIN		0x40
#define OPT_HLP		0x20
#define OPT_SHOW	0x10
#define OPT_LIST	0x08

#ifdef __cplusplus
}
#endif

#endif/*__IMAGE_HEADER__*/
