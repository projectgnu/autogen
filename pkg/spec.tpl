[= AutoGen5 Template spec =]
Summary: AutoGen - [=prog-title=]
Name: [= prog-name =]
Version: [= version =]
Vendor: [= copyright.owner =] http://autogen.sf.net
Release: [=`echo $AG_MAJOR_VERSION`=]
Copyright: GPL
Group: Development/Tools
Source: http://prdownload.sourceforge.net/autogen/autogen-[= version =].tar.gz
BuildRoot: [=`cd ${top_builddir}/AGPKG > /dev/null && pwd`
           =]/BUILD/ROOT
BuildRequires: gzip texinfo libtool > 1.3 guile

%description
AutoGen is a tool designed for generating program files that contain
repetitive text with varied substitutions.  Its goal is to simplify the
maintenance of programs that contain large amounts of repetitious text.
This is especially valuable if there are several blocks of such text
that must be kept synchronized in parallel tables.

Some parts are released under different licensing:

libopts LGPL  This is a tear-off, redistributable option processing library
autofsm BSD   This is a template for producing finite state machine programs

The Copyright itself is privately held by Bruce Korb.

%prep
%setup
chmod -R +rw *

%build
./configure --prefix=[=`echo $prefix`=]
make CFLAGS="$RPM_OPT_FLAGS"

if [ `id -u` -eq 0 ] && egrep -q ^nobody /etc/passwd; then
	echo "switching to user nobody to run 'make check'"
	chown -R nobody . ; su -c "umask 002; make check || touch FAIL" nobody
else
	make check
fi
[ -f FAIL ] && exit 1

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf ${RPM_BUILD_ROOT}
mkdir -p $RPM_BUILD_ROOT
make install DESTDIR=${RPM_BUILD_ROOT}

( cd $RPM_BUILD_ROOT && find . ! -type d 
) | sed "s,^\./,/,g" > [= prog-name =]-filelist

%clean
rm -rf ${RPM_BUILD_ROOT}

%files -f [= prog-name =]-filelist
%defattr(-,root,root)
%doc [A-T][A-Z]* ChangeLog

%changelog
* Sat Mar 15 2003 Bruce Korb <bkorb@gnu.org>
- Rework as a template to automatically produce a properly configured RPM
* Fri Aug 9 2002 Bruce Korb <bkorb@gnu.org>
- Pull stuff from Thomas Steudten's version of this file[= #'

## Local Variables:
## mode: shell-script
## minor-mode: rpm
## indent-tabs-mode: nil
## tab-width: 4
## End:
## end of spec.tpl =]
