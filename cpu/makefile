#============================================================================#
#                                                                            #
#   LGE. DTV RESEARCH LABORATORY                                             #
#   COPYRIGHT(c) LGE CO.,LTD. 2005. SEOUL, KOREA.                            #
#   All rights are reserved.                                                 #
#   No part of this work covered by the copyright hereon may be              #
#   reproduced, stored in a retrieval system, in any form or                 #
#   by any means, electronic, mechanical, photocopying, recording            #
#   or otherwise, without the prior permission of LG Electronics.            #
#                                                                            #
#----------------------------------------------------------------------------#
#                                                                            #
#                                                                            #
#   Author : bk1472                                                          #
#   Date   : 2007. 01. 24                                                    #
#                                                                            #
#============================================================================#
TOPDIR		:= ..
OBJDIR		 = ./objs

include	$(TOPDIR)/inc.mk

SRCS		 = 
SRCS		+= main.c 
SRCS		+= cpu_main.c 
SRCS		+= func_unit.c 
SRCS		+= dbg.c

PRODUCTS	 = $(IMGDIR)/cpu_sim

all: $(CCDV) $(PRODUCTS)

$(OBJDIR)/main.o      : main.c $(HFILES)
$(OBJDIR)/cpu_main.o  : cpu_main.c $(HFILES)
$(OBJDIR)/func_unit.o : func_unit.c $(HFILES)
$(OBJDIR)/dbg.o       : dbg.c $(HFILES)

include $(TOPDIR)/rule.mk
