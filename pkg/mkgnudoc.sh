#! /bin/sh

# Time-stamp: "2002-08-09 20:00:19 bkorb"
# Version:    "$Revision: 3.4 $

eval "`egrep '^AG_' ../VERSION`"
[ -d autogen-${AG_VERSION} ] && rm -rf autogen-${AG_VERSION}
mkdir autogen-${AG_VERSION} || {
  echo cannot make directory autogen-${AG_VERSION} >&2
  exit 1
}

cd autogen-${AG_VERSION}
mkdir html_mono html_chapter html_node info text dvi ps texi || {
  echo cannot make subdirectories: >&2
  echo html_mono html_chapter html_node info text dvi ps texi >&2
  exit 1
}

echo
echo "Making documentation hierarchy for autogen-${AG_VERSION}"
echo
cd ..
[ -f autogen.info ] || make

texi2html -menu -split=none    -verbose autogen.texi
mv -f autogen.html autogen-${AG_VERSION}/html_mono
(cd autogen-${AG_VERSION}/html_mono
 gzip -c autogen.html > autogen.html.gz )
echo mono done

texi2html -menu -split=chapter -verbose autogen.texi
mv -f autogen*.html autogen-${AG_VERSION}/html_chapter
(cd autogen-${AG_VERSION}/html_chapter
 tar cf - autogen*.html | gzip > autogen_chapter_html.tar.gz )
echo chapter done

texi2html -menu -split=section -verbose autogen.texi
mv -f autogen*.html autogen-${AG_VERSION}/html_node
(cd autogen-${AG_VERSION}/html_node
 tar cf - autogen*.html | gzip > autogen_node_html.tar.gz )
echo node done

for f in autogen*.info*
do gzip -c $f > autogen-${AG_VERSION}/info/$f.gz
done

[ -f autogen.ps  ] || make autogen.ps
[ -f autogen.txt ] || make autogen.txt

gzip -c autogen.dvi  > autogen-${AG_VERSION}/dvi/autogen.dvi.gz
gzip -c autogen.ps   > autogen-${AG_VERSION}/ps/autogen.ps.gz
gzip -c autogen.texi > autogen-${AG_VERSION}/texi/autogen.texi.gz
gzip -c autogen.txt  > autogen-${AG_VERSION}/text/autogen.txt.gz
cp   -f autogen.txt    autogen-${AG_VERSION}/text/.

echo generating doc page
cd autogen-${AG_VERSION}
autogen --no-def -T ../gnudoc.tpl
cd ..
tar cvf - autogen-${AG_VERSION} | gzip > autogen-${AG_VERSION}-doc.tar.gz
