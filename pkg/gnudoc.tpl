[= AutoGen5 Template -*- Mode: html -*-

html=autogen.html

# Time-stamp: "2002-02-02 13:10:02 bkorb"
# Version:    "$Revision: 3.1 $

=]
<!DOCTYPE html PUBLIC "-//IETF//DTD HTML 2.0//EN">
[=(dne "  ==  " "<!-- ")=]

  ***  THEREFORE  *** if you make changes to this file, please
  email the author so it will not be overwritten  :-)

  --><html>
<head>
  <title>AutoGen - Table of Contents</title>
  <link rev="made" href="mailto:webmasters@www.gnu.org">
  <meta name="generator" content="AutoGen [=(. autogen-version)=]">
</head>
<body bgcolor="#FFFFFF" text="#000000" link="#1F00FF" alink="#FF0000"
      vlink="#9900DD">
<h1>AutoGen - Table of Contents</H1>
<address>Free Software Foundation</address>
<address>last updated [=`date '+%B %e, %Y'`=]</address>
<p>
<a href="/graphics/gnu-head-sm.jpg"><img src="/graphics/gnu-head-sm.jpg"
   alt=" [image of the Head of a GNU] "
   width="129" height="122">&#32;(jpeg 7k)</a>
<a href="/graphics/gnu-head.jpg">(jpeg 21k)</a>
<a href="/philosophy/gif.html">no gifs due to patent problems</a>
<p>
<p>
<p><hr><p>
<p>
This manual is available in the following formats:
<p>[=

(define (compute-size f)
  (begin
     (define fsiz (if (access? f R_OK)  (stat:size (stat f)) 0 ))
     (if (< fsiz 4096)
         (set! fsiz (number->string fsiz))
         (if (< fsiz (* 2 1024 1024))
             (set! fsiz (sprintf "%dK"  (inexact->exact (/ fsiz 1024))))
             (set! fsiz (sprintf "%dMB" (inexact->exact (/ fsiz 1048576))))
     )   )
     fsiz
) )

=]<ul>
  <li>formatted in <a href="html_mono/autogen.html">HTML ([=
      (compute-size "html_mono/autogen.html")
      =] characters)</a> entirely on one web page.
  <p>
  <li>formatted in <a href="html_mono/autogen.html.gz">HTML ([=
      (compute-size "html_mono/autogen.html.gz")
      =] gzipped bytes)</a> entirely on one web page.
  <p>
  <li> formatted in <a href="html_chapter/autogen_toc.html">HTML</a>
       with one web page per chapter.
  <p>
  <li> formatted in <a href="html_chapter/autogen_chapter_html.tar.gz">HTML ([=
      (compute-size "html_chapter/autogen_chapter_html.tar.gz")
      =] gzipped tar file)</a> with one web page per chapter.
  <p>
  <li> formatted in <a href="html_node/autogen_toc.html">HTML</a>
       with one web page per node.
  <p>
  <li> formatted in <a href="html_node/autogen_node_html.tar.gz">HTML ([=
      (compute-size "html_node/autogen_node_html.tar.gz")
      =] gzipped tar file)</a> with one web page per node.
  <p>
  <li>formatted as an <a href="info/autogen.info.gz">Info document ([=
      (compute-size "info/autogen.info.gz")
      =] bytes gzipped tar file)</a>.
  <p>
  <li>formatted as <a href="text/autogen.txt">ASCII text ([=
      (compute-size "text/autogen.txt")
      =] characters)</a>.
  <p>
  <li>formatted as <a href="text/autogen.txt.gz">ASCII text ([=
      (compute-size "text/autogen.txt.gz")
      =] gzipped bytes)</a>.
  <p>
  <li>formatted as <a href="dvi/autogen.dvi.gz">a TeX dvi file ([=
      (compute-size "dvi/autogen.dvi.gz")
      =] gzipped bytes)</a>.
  <p>
  <li>formatted as <a href="ps/autogen.ps.gz">a PostScript file ([=
      (compute-size "ps/autogen.ps.gz")
      =] gzipped bytes)</a>.
  <p>
  <li>the original <a href="texi/autogen.texi.gz">Texinfo source ([=
      (compute-size "texi/autogen.texi.gz")
      =] gzipped bytes)</a>
<p></ul>
<p><hr>

Return to <a href="/home.html">GNU's home page</a>.
<p>
FSF &amp; GNU inquiries &amp; questions to
<a href="mailto:gnu@gnu.org"><em>gnu@gnu.org</em></a>.
Other <a href="/home.html#ContactInfo">ways to contact</a> the FSF.
<p>
Comments on these web pages to
<a
href="mailto:webmasters@www.gnu.org"><em>webmasters@www.gnu.org</em></a>,
send other questions to
<a href="mailto:gnu@gnu.org"><em>gnu@gnu.org</em></a>.
<p>
Copyright (C) 2002 Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111,  USA
<p>
Verbatim copying and distribution of this entire article is
permitted in any medium, provided this notice is preserved.<hr>
</body>
</html>
