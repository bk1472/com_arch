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
TOPDIR		:= .

all:
	@$(MAKE) -C $(TOPDIR)/binutil
	@$(MAKE) -C $(TOPDIR)/cpu

clean:
	@$(MAKE) -C $(TOPDIR)/binutil clean
	@$(MAKE) -C $(TOPDIR)/cpu clean

clobber:
	@$(MAKE) -C $(TOPDIR)/binutil clobber
	@$(MAKE) -C $(TOPDIR)/cpu clobber
