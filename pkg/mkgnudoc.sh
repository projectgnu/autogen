#! /bin/sh

# Time-stamp: "2005-12-15 15:09:26 bkorb"
# Version:    "$Revision: 4.6 $

MAKE=${MAKE:-make}
pkgsrcdir=`dirname $0`
pkgsrcdir=`cd ${pkgsrcdir} ; pwd`

set -x

ddir=${PACKAGE}-${PACKAGE_VERSION}
test -d ${ddir} && rm -rf ${ddir}
mkdir ${ddir} || {
  echo cannot make directory ${ddir} >&2
  exit 1
}

( cd ${ddir}
  dirlist='html_mono info text dvi pdf ps texi'
  mkdir ${dirlist} || {
    echo cannot make subdirectories: >&2
    echo ${dirlist} >&2
    exit 1
  }
) || exit 1

echo
echo "Making documentation hierarchy for ${ddir}"
echo

test -f ${PACKAGE}.info || ${MAKE}

texiargs='--ifinfo -menu -verbose'

texi2html ${texiargs} -split=none ${PACKAGE}.texi
mv -f ${PACKAGE}.html ${ddir}/html_mono/.
echo mono done

texi2html ${texiargs} -split=chapter ${PACKAGE}.texi
if test -d ${PACKAGE}/.
then mv -f ${PACKAGE} ${ddir}/html_chapter
else mkdir ${ddir}/html_chapter
     mv -f ${PACKAGE}*.htm* ${ddir}/html_chapter/.
fi
echo chapter done

texi2html ${texiargs} -split=node ${PACKAGE}.texi
if test -d ${PACKAGE}/.
then mv -f ${PACKAGE} ${ddir}/html_node
else mkdir ${ddir}/html_node
     mv -f ${PACKAGE}*.htm* ${ddir}/html_node/.
fi
echo node done

for f in ${PACKAGE}*.info*
do gzip -c $f > ${ddir}/info/$f.gz
done

test -f ${PACKAGE}.ps  || ${MAKE} ${PACKAGE}.ps  || exit 1
test -f ${PACKAGE}.txt || ${MAKE} ${PACKAGE}.txt || exit 1
test -f ${PACKAGE}.pdf || ${MAKE} ${PACKAGE}.pdf || exit 1

gzip -c ${PACKAGE}.dvi  > ${ddir}/dvi/${PACKAGE}.dvi.gz   || exit 1
gzip -c ${PACKAGE}.pdf  > ${ddir}/pdf/${PACKAGE}.pdf.gz   || exit 1
gzip -c ${PACKAGE}.ps   > ${ddir}/ps/${PACKAGE}.ps.gz     || exit 1
gzip -c ${PACKAGE}.texi > ${ddir}/texi/${PACKAGE}.texi.gz || exit 1
gzip -c ${PACKAGE}.txt  > ${ddir}/text/${PACKAGE}.txt.gz  || exit 1
cp   -f ${PACKAGE}.txt    ${ddir}/text/.              || exit 1

echo generating doc page
cd ${ddir}
cat > TAG <<EOF
<p align="center"><a href="http://www.anybrowser.org/campaign/"
   ><img src="/software/${PACKAGE}/pix/abrowser.png"
   width="118" height="32" alt="Viewable With Any Browser"
   border="0"></a>
&nbsp;&nbsp;<a href="/software/${PACKAGE}/"
><img src="/software/${PACKAGE}/pix/${PACKAGE}_header.png"
     width="188" height="50" border="0" alt="${PACKAGE_NAME} Home"></a></p>
EOF
body-end -i TAG */*.html

(cd html_mono
 gzip -c --best ${PACKAGE}.html > ${PACKAGE}.html.gz )
(cd html_chapter
 tar cf - ${PACKAGE}*.html | gzip --best > ${PACKAGE}_chapter_html.tar.gz )
(cd html_node
 tar cf - ${PACKAGE}*.html | gzip --best > ${PACKAGE}_node_html.tar.gz )

autogen --base-name=${PACKAGE} -T ${pkgsrcdir}/gnudoc.tpl - <<- _EODefs_
	autogen definitions gnudoc;
	title = '${PACKAGE_NAME} - ${*}';
	project = '${PACKAGE_NAME}';
	version = '${PACKAGE_VERSION}';
	_EODefs_

rm -f TAG
cd ..
tar cf - ${ddir} | gzip > ${ddir}-doc.tar.gz

## Local Variables:
## Mode: shell-script
## tab-width: 4
## indent-tabs-mode: nil
## sh-indentation: 2
## sh-basic-offset: 2
## End:
