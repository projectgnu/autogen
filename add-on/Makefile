
#  This file is part of AutoGen
#  AutoGen Copyright (C) 1992-2015 Bruce Korb - all rights reserved
#
# AutoGen is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# AutoGen is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

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
