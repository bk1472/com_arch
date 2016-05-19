$(PRODUCTS): $(OBJECTS)
	@$(CCDV) $(CC) -o $@ $(OBJECTS) $(LIB_OPT) -lz

$(OBJDIR)/%.o : %.c
	-@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	@$(CC_CMD) -o $@ $<

%.o : %.c
	@$(CC_CMD) -o $@ $<

# LIB files
$(LIBDIR)/hexdump.o   : $(LIBDIR)/hexdump.c $(HFILES)
$(LIBDIR)/dbgprint.o  : $(LIBDIR)/dbgprint.c $(HFILES)

clean:
	@$(RM) $(OBJDIR)/*.o

clobber: clean
	@$(RM) -rf $(OBJDIR)
	@$(RM) $(LIBDIR)/*.o
	@$(RM) $(PRODUCTS).exe


$(CCDV): $(TOPDIR)/util/ccdv.src/ccdv.c \
		 $(TOPDIR)/util/ccdv.src/sift-warn.c
	@$(MAKE) -C $(TOPDIR)/util/ccdv.src
