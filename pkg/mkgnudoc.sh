#! /bin/sh

# Time-stamp: "2003-05-05 20:52:47 bkorb"
# Version:    "$Revision: 3.9 $

MAKE=${MAKE:-make}

eval "`egrep '^AG_' ../VERSION`"
[ -d autogen-${AG_VERSION} ] && rm -rf autogen-${AG_VERSION}
mkdir autogen-${AG_VERSION} || {
  echo cannot make directory autogen-${AG_VERSION} >&2
  exit 1
}

cd autogen-${AG_VERSION}
mkdir html_mono html_chapter html_node info text dvi pdf ps texi || {
  echo cannot make subdirectories: >&2
  echo html_mono html_chapter html_node info text dvi ps texi >&2
  exit 1
}

echo
echo "Making documentation hierarchy for autogen-${AG_VERSION}"
echo
cd ..
[ -f autogen.info ] || ${MAKE}

texi2html -menu -split=none    -verbose autogen.texi
mv -f autogen.html autogen-${AG_VERSION}/html_mono
echo mono done

texi2html -menu -split=chapter -verbose autogen.texi
mv -f autogen*.html autogen-${AG_VERSION}/html_chapter
echo chapter done

texi2html -menu -split=section -verbose autogen.texi
mv -f autogen*.html autogen-${AG_VERSION}/html_node
echo node done

for f in autogen*.info*
do gzip -c $f > autogen-${AG_VERSION}/info/$f.gz
done

[ -f autogen.ps  ] || ${MAKE} autogen.ps
[ -f autogen.txt ] || ${MAKE} autogen.txt
[ -f autogen.pdf ] || ${MAKE} autogen.pdf

gzip -c autogen.dvi  > autogen-${AG_VERSION}/dvi/autogen.dvi.gz
gzip -c autogen.pdf  > autogen-${AG_VERSION}/pdf/autogen.pdf.gz
gzip -c autogen.ps   > autogen-${AG_VERSION}/ps/autogen.ps.gz
gzip -c autogen.texi > autogen-${AG_VERSION}/texi/autogen.texi.gz
gzip -c autogen.txt  > autogen-${AG_VERSION}/text/autogen.txt.gz
cp   -f autogen.txt    autogen-${AG_VERSION}/text/.

echo generating doc page
cd autogen-${AG_VERSION}
cat > TAG <<EOF
<p align="center"><a href="http://www.anybrowser.org/campaign/"
   ><img src="/software/autogen/pix/abrowser.png"
   width="118" height="32" alt="Viewable With Any Browser"
   border="0"></a>
&nbsp;&nbsp;<a href="/software/autogen/"
><img src="/software/autogen/pix/autogen_header.png"
     width="188" height="50" border="0" alt="AutoGen Home"></a></p>
EOF
body-end -i TAG */*.html

(cd html_mono
 gzip -c autogen.html > autogen.html.gz )
(cd html_chapter
 tar cf - autogen*.html | gzip > autogen_chapter_html.tar.gz )
(cd html_node
 tar cf - autogen*.html | gzip > autogen_node_html.tar.gz )

autogen --no-def -T ${pkgsrcdir}/gnudoc.tpl
rm -f TAG
cd ..
tar cvf - autogen-${AG_VERSION} | gzip > autogen-${AG_VERSION}-doc.tar.gz
