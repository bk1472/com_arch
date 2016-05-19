#ifndef __CPU_HEADER__
#define __CPU_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

/*CPU Status*/
#define Z_BIT	0x1
#define N_BIT	0x2
#define C_BIT	0x4
#define V_BIT	0x8

/* instruction decoder */
#define CTRL_ST		0
#define DA_IDX		(CTRL_ST +  0)
#define AA_IDX		(CTRL_ST +  1)
#define BA_IDX		(CTRL_ST +  2)
#define MB_IDX		(CTRL_ST +  3)
#define FS_IDX		(CTRL_ST +  4)
#define MD_IDX		(CTRL_ST +  5)
#define RW_IDX		(CTRL_ST +  6)
#define MW_IDX		(CTRL_ST +  7)
#define PL_IDX		(CTRL_ST +  8)
#define JB_IDX		(CTRL_ST +  9)
#define BC_IDX		(CTRL_ST + 10)
#define CTRL_END	(BC_IDX      )
#define CTRL_SIZ	(CTRL_END + 1)

#define GET_DA(x)	((x>>20) & 0x07)
#define GET_AA(x)	((x>>17) & 0x07)
#define GET_BA(x)	((x>>14) & 0x07)
#define GET_MB(x)	((x>>13) & 0x01)
#define GET_FS(x)	((x>> 8) & 0x1F)
#define GET_MD(x)	((x>> 7) & 0x01)
#define GET_RW(x)	((x>> 6) & 0x01)
#define GET_MW(x)	((x>> 5) & 0x01)
#define GET_PL(x)	((x>> 4) & 0x01)
#define GET_JB(x)	((x>> 3) & 0x01)
#define GET_BC(x)	((x>> 0) & 0x07)


#define END_CODE	(0xFFFF)
#define RET_END		(1)

#ifdef __cplusplus
}
#endif

#endif/*__CPU_HEADER__*/
