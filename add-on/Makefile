
SUBDIRS := $(shell for f in */Makefile ; do dirname $$f ; done)
SUBCMD  := set -e;for d in $(SUBDIRS);do cd $$d;$(MAKE) $$targ;cd ..;done

default: install-local

all :
	targ=$@;$(SUBCMD)

install-local :
	@-rm -rf bin ; mkdir bin
	targ=$@;$(SUBCMD)

clean :
	targ=$@;$(SUBCMD)

clobber :
	rm -rf bin
	targ=$@;$(SUBCMD)
