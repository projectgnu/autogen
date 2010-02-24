
EXE         = allbut
SRC         = allbut.c

default     : all

all         : $(EXE)

install-local : all
	cp $(EXE) ../bin/.

$(EXE)      : $(SRC)
	$(CC) -o $@ $(SRC)

clean       :
	rm -f *~

clobber     : clean
	rm -f $(EXE)
