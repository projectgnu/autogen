
# char-mapper Makefile
#

           TOP = ../..
           SRC = char-mapper.c cm-opt.c
           OBJ = $(SRC:.c=.o)
           EXE = char-mapper
        TARGET = $(EXE)
        LCLHDR = cm-opt.h

default     : verbose-test
all         : $(EXE)

$(EXE)      : $(OBJ)
	$(CC) -g -o $@ $(OBJ)

install-local : all
	cp $(EXE) ../bin/.

.c.o:
	$(CC) $(DBG) $(INC) -c $*.c

$(SRC)      : cm-opt.stamp
cm-opt.stamp: mk-str2enum.sh mk-opt-table.sh
	bash ./mk-opt-table.sh $@
	@touch $@

clobber     : clean
	rm -f char-mapper ag-char-map*
	bash ./mk-opt-table.sh --clobber cm-opt
	bash ./test.sh --clobber

source-list :
	exec > /dev/null 2>&1 ; \
	rm -f char-mapper ag-char-map* *.o ; \
	bash ./mk-opt-table.sh --clobber cm-opt ; \
	bash ./test.sh --clobber
	echo [a-z]* Makefile

clean       :
	rm -f *.o core* *~ ; bash ./test.sh --clean
	bash ./mk-opt-table.sh --clean cm-opt
	bash ./test.sh --clean

test        : $(EXE)
	@test "X$$VERBOSE" = X && opts=-e || opts='-x -e' ; set -x ; \
	bash $$opts ./test.sh

check       : test

verbose-test : $(EXE)
	bash -x ./test.sh >/dev/null 2>&1 ; rm -f test-cmap.c
