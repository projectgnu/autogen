
# char-mapper Makefile
#
            TOP = ..
            SRC = char-mapper.c cm-opt.c
            OBJ = $(SRC:.c=.o)
            EXE = char-mapper
         TARGET = $(EXE)
    LOCAL_CLEAN = lcl-clean
  LOCAL_CLOBBER = lcl-clobber
    HEADER_LIST = cm-opt.h

include $(TOP)/MakeRules

cm-opt.c        : cm-opt.stamp
$(OBJ)          : $(HEADER_LIST) cm-usage.c
$(HEADER_LIST)  : cm-opt.stamp

cm-opt.stamp: mk-str2enum.sh mk-opt-table.sh
	bash ./mk-opt-table.sh $@
	@touch $@

lcl-clobber : clean
	rm -f char-mapper ag-char-map*
	bash ./mk-opt-table.sh --clobber cm-opt
	bash ./test.sh --clobber

source-list :
	exec > /dev/null 2>&1 ; \
	rm -f char-mapper ag-char-map* *.o ; \
	bash ./mk-opt-table.sh --clobber cm-opt ; \
	bash ./test.sh --clobber
	echo [a-z]* Makefile

lcl-clean   :
	bash ./test.sh --clean
	bash ./mk-opt-table.sh --clean cm-opt
	bash ./test.sh --clean
