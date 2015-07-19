
#  This file is part of char-mapper.
#  char-mapper Copyright (C) 2003-2015 Bruce Korb - all rights reserved
#
# char-mapper is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# char-mapper is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

SRC := char-mapper.c cm-opt.c map-text.c
HDR := $(SRC:.c=.h)
OBJ := $(SRC:.c=.o)
EXE := char-mapper
GEN := cm-opt.c cm-opt.h map-text.c map-text.h
BASH    := $(shell for f in /usr/bin/bash /usr/local/bin/bash ; do test -x $$f && echo $$f && exit 0 ; done ; command -v bash)
CFLAGS  := -ggdb3 -gdwarf-2 -O0
LDFLAGS :=

default : $(EXE)

$(GEN)  : mk-stamp
mk-stamp : char-mapper.c map-text.def mk-opt-table.sh
	$(BASH) -x mk-opt-table.sh $@

%.o : %.c $(HDR)
	$(CC) -std=gnu99 -c $(CFLAGS) -o $@ $(@:.o=.c)

$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)
