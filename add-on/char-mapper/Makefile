
# char-mapper Makefile
#
            TOP = ..
         GENSRC = cm-opt.c map-text.c
         GENHDR = map-text.h cm-opt.h
            SRC = char-mapper.c $(GENSRC)
            OBJ = $(SRC:.c=.o)
            EXE = char-mapper
         TARGET = $(EXE)
    LOCAL_CLEAN = lcl-clean
  LOCAL_CLOBBER = lcl-clobber
         LCLHDR = $(GENHDR) char-mapper.h

include $(TOP)/MakeRules

$(GENSRC)       : cm-opt.stamp
char-mapper.o   : cm-opt.stamp
$(OBJ)          : $(HEADER_LIST)
$(HEADER_LIST)  : cm-opt.stamp

cm-opt.stamp    : mk-str2enum.sh mk-opt-table.sh map-text.def
	bash ./mk-opt-table.sh $@

lcl-clobber     : clean
	rm -rf $(EXE) $(GENHDR) $(GENSRC)
	bash ./test.sh --clobber

source-list :
	exec > /dev/null 2>&1 ; \
	rm -f char-mapper ag-char-map* *.o ; \
	bash ./mk-opt-table.sh --clobber cm-opt ; \
	bash ./test.sh --clobber
	echo [a-z]* Makefile

lcl-clean   :
	bash ./test.sh --clean
	rm -f *~ $(OBJ) *.stamp $(EXE).txz

check : lcl-check
lcl-check : $(EXE)
	bash ./test.sh

tarball : $(GENSRC)
	@-test -f $(EXE) && mv $(EXE) $(EXE).exe
	@-rm -f $(EXE).txz ; mkdir $(EXE)
	ln $(SRC) $(HEADER_LIST) map-text.def $(TOP)/MakeRules *.sh $(EXE)/.
	sed '/^ *TOP =/s/=.*/= ./' Makefile > $(EXE)/Makefile
	tar cJvf $(EXE).txz $(EXE)
	cd $(EXE) ; $(MAKE)
	@-rm -rf $(EXE) ; if test -f $(EXE).exe ; then mv $(EXE).exe $(EXE) ; fi
