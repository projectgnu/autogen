
AGEXE          = `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
DISTCLEANFILES = autogen.lsm
EXTRA_DIST     = lsm.tpl autogen.spec.in

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/LSM.tpl $(top_builddir)/agen5/opts.def

libopts-src :
	pkgdir=`pwd` ; cd $(top_builddir)/doc ; $(SHELL) $${pkgdir}/mklibopts-src.sh

##  The following targets only work from CVS sources
##
gnudoc :
	pkgdir=`pwd` ; cd $(top_builddir)/doc ; $(SHELL) $${pkgdir}/mkgnudoc.sh