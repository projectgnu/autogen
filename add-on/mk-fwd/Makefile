
SRC         = mk-fwd.sh
EXE         = mk-fwd

default     : all
all         : $(EXE)

$(EXE)      : $(SRC)
	cp $(SRC) $@ && chmod 755 $@

clean       :
	rm -f *~

clobber     : clean
	rm -f $(EXE)
