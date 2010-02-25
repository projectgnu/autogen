
SOURCES         := $(wildcard *.def)
INTERMEDIATE    := $(SOURCES:.def=.h)
TARGETS         := $(SOURCES:.def=.c)
OBJECTS         := $(SOURCES:.def=.o)
TESTS           := $(SOURCES:%.def=test-%)
HANDLERSRC      := $(SOURCES:%.def=%-hdlr.c)

default     : $(TARGETS)
all         : $(TESTS)

$(TARGETS)  : %.c   : %.def dispatch.tpl
	@-rm -f *-hdlr.c
	autogen $<

install-local :
	cp dispatch.tpl ../bin/.

clean       :
	rm -f $(OBJECTS) $(TESTS) $(INTERMEDIATE) $(TARGETS)
	bash ./test-all.sh -cleanup

clobber     : clean
	rm -f *~ $(TARGETS) $(INTERMEDIATE)

source-list : clobber
	echo [a-z]* Makefile

tests       : $(TESTS)

$(HANDLERSRC) : %-hdlr.c : %.def
	EMIT_DISPATCH=true autogen $<

$(TESTS)    : test-% : %-hdlr.c %.o
	cc -g -o $@ $^

objects     : $(OBJECTS)

$(OBJECTS)  : %.o : %.c
	cc -g -c -o $@ $<
