###########################################################
# makefile for com. arch. cpu source
###########################################################
TOPDIR		:= ..
AS			 = $(TOPDIR)/img/com-as
RM			 = rm -f

all: com add

com: com.asm
	$(AS) -g -s $^

add: add.asm
	$(AS) -g -s $^

clean:
	@$(RM) *.img
	@$(RM) *.bin
	@$(RM) *.list
