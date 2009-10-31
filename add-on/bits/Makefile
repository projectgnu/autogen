
DEFS := $(shell echo *.def)
SRC  := $(DEFS:.def=.c)
HDR  := $(DEFS:.def=.h)
EXE  := $(DEFS:.def=)
CFL  := $(shell test -z "$(CFLAGS)" && echo -g || echo $(CFLAGS))
CPF  := $(shell test -z "$(CPPFLAGS)" || echo $(CPPFLAGS))
TSH  := $(DEFS:.def=.sh)

default : gen

gen     : $(SRC)

%.c %.h : %.def
	autogen $<

%.sh    : %.def bit-test.tpl
	autogen -T bit-test.tpl $<

% : %.c
	$(CC) -o $@ $(CFL) -DTEST_BITS $(CPF) $<

test    : $(EXE) $(TSH)
	for f in $(TSH) ; do $(SHELL) $(DASHX) ./$$f ; rm -f $$f ; done

clobber : clean
	rm -f b-test.[ch]

source-list : clobber
	echo [a-z]* Makefile

clean   :
	rm -f *~ $(SRC) $(HDR) $(TSH) $(EXE) core
