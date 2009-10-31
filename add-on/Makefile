
SUBDIRS := $(shell for f in */Makefile ; do dirname $$f ; done)
SUBCMD  := for d in $(SUBDIRS);do (cd $$d;$(MAKE) $$targ) ; done

default: all

all :
	targ=$@;$(SUBCMD)

clean :
	targ=$@;$(SUBCMD)

clobber :
	targ=$@;$(SUBCMD)
