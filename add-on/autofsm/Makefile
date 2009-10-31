
SRCDIR  = ../../agen5
TPL     = fsm-macro.tpl fsm-trans.tpl fsm.tpl
DEF     = example.def
EXTRA   = run.sh Makefile README
CLEAN   = *-fsm.[cho] [cn]*-*.def *~ *-stamp *.samp *.out .fsm.* $(BASE)*
BASE    = test

default : example-fsm.c
all     : check-stamp
check   : check-stamp
example : example-fsm.c

check-stamp : sample.shar
	@DEF=$(DEF) sh run.sh testing && \
	touch $@ && \
	echo all tests passed
	@rm -f $(CLEAN)

clobber : clean
	rm -rf autofsm-[0-9]* *.html example-fsm.[ch] $(TPL) sample.shar

clean :
	rm -f $(CLEAN)

bootstrap : clean sample.shar

sample.shar : $(TPL) $(DEF) run.sh
	DEF=$(DEF) bash run.sh bootstrap

example-fsm.c : example.def $(TPL)
	autogen -L. example.def

%.tpl : $(SRCDIR)/%.tpl
	cp $(SRCDIR)/$@ .
