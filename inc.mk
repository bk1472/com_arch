IMGDIR		 = $(TOPDIR)/img
LIBDIR		 = $(TOPDIR)/lib
HFILES		 = $(LINC_DIR)/common.h $(LINC_DIR)/image.h

LIBSRCS		 = 
LIBSRCS		+= $(LIBDIR)/hexdump.c 
LIBSRCS		+= $(LIBDIR)/dbgprint.c

OBJECTS		 = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
OBJECTS		+= $(LIBSRCS:.c=.o)

################################
# TOOL
################################
CCDV_OPTS	?= -p
CCDV		?= $(TOPDIR)/util/ccdv $(CCDV_OPTS)
RM			?= rm -f

INC_TOP		 =
USR_TOP		 = /usr
LIB_OPT		 =

INC_DIR		 = .
LINC_DIR	 = $(TOPDIR)/include
INC_LST		 = -I$(INC_DIR)  -I$(LINC_DIR) -I.
CFLAGS		 = -O2

CC			 = gcc
CC_CMD		 = $(CCDV) $(CC) -c $(CFLAGS) $(DEF_LST) $(INC_LST)

ENDIAN_TYPE	 = END_L

DEF_LST		 = -DENDIAN_TYPE=$(ENDIAN_TYPE)
