
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

SRCDIR  = ../../agen5
TPL     = fsm-macro.tlib fsm-trans.tlib fsm.tpl
DEF     = example.def
EXTRA   = run.sh Makefile README
CLEAN   = *-fsm.[cho] [cn]*-*.def *~ *-stamp *.samp *.out .fsm.* $(BASE)*
BASE    = test

default : example-fsm.c
all     : check-stamp
check   : check-stamp
example : example-fsm.c

install-local : all
	@:

$(TPL)  : tpl-stamp

tpl-stamp :
	@rm -f $(TPL) ; ln $(SRCDIR)/fsm*.t* .

check-stamp : sample.shar
	@DEF=$(DEF) sh run.sh testing && \
	touch $@ && \
	echo all tests passed
	@rm -f $(CLEAN)

clobber : clean
	rm -rf autofsm-[0-9]* *.html example-fsm.[ch] $(TPL) sample.shar

clean :
	rm -f $(CLEAN)

bootstrap : clean sample.shar

sample.shar : $(TPL) $(DEF) run.sh
	DEF=$(DEF) bash run.sh bootstrap

example-fsm.c : example.def $(TPL)
	autogen -L. example.def
