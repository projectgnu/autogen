#! /bin/sh

# Time-stamp: "2005-04-17 12:11:27 bkorb"
# Version:    "$Revision: 4.3 $

MAKE=${MAKE:-make}

eval "`egrep '^AG_' ../VERSION`"
ddir=autogen-${AG_VERSION}
test -d ${ddir} && rm -rf ${ddir}
mkdir ${ddir} || {
  echo cannot make directory ${ddir} >&2
  exit 1
}

cd ${ddir}
dirlist='html_mono info text dvi pdf ps texi'
mkdir ${dirlist} || {
  echo cannot make subdirectories: >&2
  echo ${dirlist} >&2
  exit 1
}

echo
echo "Making documentation hierarchy for ${ddir}"
echo
cd ..
[ -f autogen.info ] || ${MAKE}

texiargs='--ifinfo -menu -verbose'

texi2html ${texiargs} -split=none autogen.texi
mv -f autogen.html ${ddir}/html_mono/.
echo mono done

texi2html ${texiargs} -split=chapter autogen.texi
if test -d autogen/.
then mv -f autogen ${ddir}/html_chapter
else mkdir ${ddir}/html_chapter
     mv -f autogen*.htm* ${ddir}/html_chapter/.
fi
echo chapter done

texi2html ${texiargs} -split=node autogen.texi
if test -d autogen/.
then mv -f autogen ${ddir}/html_node
else mkdir ${ddir}/html_node
     mv -f autogen*.htm* ${ddir}/html_node/.
fi
echo node done

for f in autogen*.info*
do gzip -c $f > ${ddir}/info/$f.gz
done

[ -f autogen.ps  ] || ${MAKE} autogen.ps
[ -f autogen.txt ] || ${MAKE} autogen.txt
[ -f autogen.pdf ] || ${MAKE} autogen.pdf

gzip -c autogen.dvi  > ${ddir}/dvi/autogen.dvi.gz
gzip -c autogen.pdf  > ${ddir}/pdf/autogen.pdf.gz
gzip -c autogen.ps   > ${ddir}/ps/autogen.ps.gz
gzip -c autogen.texi > ${ddir}/texi/autogen.texi.gz
gzip -c autogen.txt  > ${ddir}/text/autogen.txt.gz
cp   -f autogen.txt    ${ddir}/text/.

echo generating doc page
cd ${ddir}
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
tar cvf - ${ddir} | gzip > ${ddir}-doc.tar.gz
