
## This file is part of AutoGen.
## AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# use autoopts-config to determine
# 1) the locatation autogen      (autoopts-config --autogen)
# 2) the data dir for autogen    (autoopts-config --pkgdatadir)
# 3) the include dir for autogen (autoopts-config --cflags)
# 4) the lib dir for autogen     (autoopts-config --libs)

AUTOGEN_PROGRAM=$(shell autoopts-config --autogen)
AUTOGEN_DATADIR=$(shell autoopts-config --pkgdatadir)
AUTOGEN_FLAGS=-L$(AUTOGEN_DATADIR)

AUTOGEN_CFLAGS=$(shell autoopts-config --cflags)
AUTOGEN_LDFLAGS=$(shell autoopts-config --libs)

CFLAGS+=$(AUTOGEN_CFLAGS) -DTEST_CHECK_OPTS
LDFLAGS+=$(AUTOGEN_LDFLAGS)

.PHONY: build
build: main

.PHONY: clean
clean:
	rm -f *.o main checkopt.c checkopt.h

main: checkopt.c main.c
checkopt.c checkopt.h: checkopt.def

# additional rules
%.c: %.def
	$(AUTOGEN_PROGRAM) $(AUTOGEN_FLAGS) $^
