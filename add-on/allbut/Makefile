
EXE         = allbut
SRC         = allbut.c

default     : all

all         : $(EXE)

$(EXE)      : $(SRC)
	$(CC) -o $@ $(SRC)

clean       :
	rm -f *~

clobber     : clean
	rm -f $(EXE)
