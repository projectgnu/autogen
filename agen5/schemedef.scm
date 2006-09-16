
;;;  AutoGen copyright 1992-2006 Bruce Korb
;;;
;;; AutoGen is free software.
;;; You may redistribute it and/or modify it under the terms of the
;;; GNU General Public License, as published by the Free Software
;;; Foundation; either version 2, or (at your option) any later version.
;;;
;;; AutoGen is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with AutoGen.  See the file "COPYING".  If not,
;;; write to:  The Free Software Foundation, Inc.,
;;;            51 Franklin Street, Fifth Floor,
;;;            Boston, MA  02110-1301, USA.
;;;
;;; This module defines all the scheme initialization for AutoGen.
;;; It gets sucked up into directives.h as a single ANSI-C string.
;;; Comments, blank lines and leading white space are omitted.

(use-modules (ice-9 common-list))

(define identifier?
  (lambda (x) (or (string? x) (symbol? x))))

(define normalize-identifier
  (lambda (x)
    (if (string? x) (string->symbol x) x)))

(define coerce->string
  (lambda (x)
    (let ((char->string (lambda (x) (make-string 1 x)))
          (coercable? (lambda (x)
            (or (string? x) (boolean? x) (char? x)
                (symbol? x) (list? x) (number? x)) )) )

      (if (not (coercable? x))
          (error "Wrong type to coerce->string" x))

      (cond
        ((string? x)  (string-append
            (char->string #\") x (char->string #\")  ))

        ; Probably not what was wanted, but fun
        ((boolean? x) (if x "#t" "#f"))
        ((char? x)    (char->string x))
        ((number? x)  (number->string x))
        ((symbol? x)  (symbol->string x))
        ((list? x)    (if (every coercable? x)
            (apply string-append (map coerce->string x))  ))
) ) ) )


;;; alist->autogen-def:
;;; take a scheme alist of values, and create autogen assignments.
;;; recursive alists are handled. Using a bare list as a value to be
;;; assigned is not a terribly good idea, though it should work if it
;;; doesn't look too much like an alist The returned string doesn't
;;; contain opening and closing brackets.

(define alist->autogen-def
  (lambda (lst . recursive)
    (if (null? recursive) (set! recursive #f)
        (set! recursive #t))
    (let ((res (if recursive "{\n" ""))
          (list-nnul? (lambda (x) (and (list? x) (not (null? x))))))
      (do ((i lst (cdr i)))
          ((null? i) (if recursive
                          (string-append res "}")
                           res))
        (let* ((kvpair (car i))
               (value (cdr kvpair))
               (value-is-alist (if (and (list-nnul? value)
                                        (list-nnul? (car value))
                                        (list-nnul? (caar value))
                                        (identifier? (caaar value)))
                                   #t #f)))
          (set! res (string-append res
                (coerce->string (normalize-identifier (car kvpair)))
                " = "
                (if value-is-alist
                    (alist->autogen-def (car value) 1)
                    (coerce->string (cdr kvpair)))
                ";\n"
) ) ) ) ) )         )

(define shell-cleanup "")

;;; /*=gfunc   make_header_guard
;;;  *
;;;  * what:   make self-inclusion guard
;;;  *
;;;  * exparg: name , header group name
;;;  *
;;;  * doc:
;;;  *   This function will create a @code{#ifndef}/@code{#define}
;;;  *   sequence for protecting a header from multiple evaluation.
;;;  *   It will also set the Scheme variable @code{header-file}
;;;  *   to the name of the file being protected and it will set
;;;  *   @code{header-guard} to the name of the @code{#define} being
;;;  *   used to protect it.  It is expected that this will be used
;;;  *   as follows:
;;;  *   @example
;;;  *   [+ (make-header-guard "group_name") +]
;;;  *   ...
;;;  *   #endif /* [+ (. header-guard) +]
;;;  *
;;;  *   #include "[+ (. header-file)  +]"
;;;  *   @end example
;;;  *   @noindent
;;;  *   The @code{#define} name is composed as follows:
;;;  *
;;;  *   @enumerate
;;;  *   @item
;;;  *   The first element is the string argument and a separating underscore.
;;;  *   @item
;;;  *   That is followed by the name of the header file with illegal
;;;  *   characters mapped to underscores.
;;;  *   @item
;;;  *   The end of the name is always, "@code{_GUARD}".
;;;  *   @item
;;;  *   Finally, the entire string is mapped to upper case.
;;;  *   @end enumerate
;;;  *
;;;  *   The final @code{#define} name is stored in an SCM symbol named
;;;  *   @code{header-guard}.  Consequently, the concluding @code{#endif} for the
;;;  *   file should read something like:
;;;  *
;;;  *   @example
;;;  *   #endif /* [+ (. header-guard) +] */
;;;  *   @end example
;;;  *
;;;  *   The name of the header file (the current output file) is also stored
;;;  *   in an SCM symbol, @code{header-file}.  Therefore, if you are also
;;;  *   generating a C file that uses the previously generated header file,
;;;  *   you can put this into that generated file:
;;;  *
;;;  *   @example
;;;  *   #include "[+ (. header-file) +]"
;;;  *   @end example
;;;  *
;;;  *   Obviously, if you are going to produce more than one header file from
;;;  *   a particular template, you will need to be careful how these SCM symbols
;;;  *   get handled.
;;; =*/
;;;
(define header-file  "")
(define header-guard "")
(define (make-header-guard hdr-pfx)
  (begin
    (set! header-file  (out-name))
    (set! header-guard (string-upcase! (string->c-name! (string-append
      (if (string? hdr-pfx) hdr-pfx "HEADER") "_" header-file "_GUARD"
      ))))
    (sprintf "#ifndef %1$s\n#define %1$s" header-guard)
) )

(define autogen-version "AUTOGEN_VERSION")
(define c-file-line-fmt "#line %2$d \"%1$s\"\n")

(define-macro (defined-as predicate symbol)
  `(and (defined? ',symbol) (,predicate ,symbol)))

;;; /*=gfunc   html_escape_encode
;;;  *
;;;  * what:   encode html special characters
;;;  * general-use:
;;;  *
;;;  * exparg: str , string to make substitutions in
;;;  *
;;;  * doc:    This function will replace replace the characters @code{'&'},
;;;  *         @code{'<'} and @code{'>'} characters with the HTML/XML
;;;  *         escape-encoded strings (@code{"&amp;"}, @code{"&lt;"}, and
;;;  *         @code{"&gt;"}, respectively).
;;; =*/
;;;
(define html-escape-encode (lambda (str)
    (string-substitute str
          '("&"      "<"     ">")
          '("&amp;"  "&lt;"  "&gt;") ) ))

(define stt-table   (make-hash-table 31))
(define stt-curr    stt-table)
(define stt-idx-tbl stt-table)
(define stt-idx     0)

;;; /*=gfunc   string_table_new
;;;  *
;;;  * what:   create a string table
;;;  * general-use:
;;;  *
;;;  * exparg: st-name , the name of the array of characters
;;;  *
;;;  * doc:
;;;  *   This function will create an array of characters.  The companion
;;;  *   functions, (@xref{SCM string-table-add}, and @pxref{SCM
;;;  *   emit-string-table}) will insert text and emit the populated table,
;;;  *   respectively.
;;;  *
;;;  *   With these functions, it should be much easier to construct structures
;;;  *   containing string offsets instead of string pointers.  That can be very
;;;  *   useful when transmitting, storing or sharing data with different address
;;;  *   spaces.
;;;  *
;;;  *   @noindent
;;;  *   Here is a brief example copied from the strtable.test test:
;;;  *
;;;  *   @example
;;;  *      [+ (string-table-new "scribble")
;;;  *    `'   (out-push-new)
;;;  *    `'   (define ix 0)
;;;  *    `'   (define ct 1)  +][+
;;;  *
;;;  *      FOR str IN that was the week that was +][+
;;;  *    `'  (set! ct (+ ct 1))
;;;  *    `'  (set! ix (string-table-add "scribble" (get "str")))
;;;  *      +]
;;;  *    `'    scribble + [+ (. ix) +],[+
;;;  *      ENDFOR  +]
;;;  *    `'    NULL @};
;;;  *      [+ (out-suspend "main")
;;;  *    `'   (emit-string-table "scribble")
;;;  *    `'   (ag-fprintf 0 "\nconst char *ap[%d] = @{" ct)
;;;  *    `'   (out-resume "main")
;;;  *    `'   (out-pop #t) +]
;;;  *   @end example
;;;  *
;;;  *   @noindent
;;;  *   Some explanation:
;;;  *
;;;  *   @noindent
;;;  *   I added the @code{(out-push-new)} because the string table text is
;;;  *   diverted into an output stream named, ``scribble'' and I want to have
;;;  *   the string table emitted before the string table references.  The string
;;;  *   table references are also emitted inside the @code{FOR} loop.  So, when
;;;  *   the loop is done, the current output is suspended under the
;;;  *   name, ``main'' and the ``scribble'' table is then emitted into the
;;;  *   primary output.  (@code{emit-string-table} inserts its output directly
;;;  *   into the current output stream.  It does not need to be the last
;;;  *   function in an AutoGen macro block.)  Next I @code{ag-fprintf} the
;;;  *   array-of-pointer declaration directly into the current output.
;;;  *   Finally I restore the ``main'' output stream and @code{(out-pop #t)}-it
;;;  *   into the main output stream.
;;;  *
;;;  *   Here is the result.  Note that duplicate strings are not repeated
;;;  *   in the string table:
;;;  *
;;;  *   @example
;;;  *      static const char scribble[18] =
;;;  *    `'    "that\0" "was\0"  "the\0"  "week\0";
;;;  *
;;;  *      const char *ap[7] = @{
;;;  *    `'    scribble + 0,
;;;  *    `'    scribble + 5,
;;;  *    `'    scribble + 9,
;;;  *    `'    scribble + 13,
;;;  *    `'    scribble + 0,
;;;  *    `'    scribble + 5,
;;;  *    `'    NULL @};
;;;  *   @end example
;;;  *
;;;  *   These functions use the global name space @code{stt-*} in addition to
;;;  *   the function names.
;;; =*/
;;;
(define string-table-new (lambda (st-name) (begin
   (set! stt-curr (make-hash-table 31))
   (hash-create-handle! stt-table st-name stt-curr)
   (out-push-new)
   (out-suspend st-name)
   (set! stt-idx-tbl (make-hash-table 31))
   (hash-create-handle! stt-curr "string-indexes" stt-idx-tbl)
   (hash-create-handle! stt-curr "current-index"  0)
   ""
)))

;;; /*=gfunc   string_table_add
;;;  *
;;;  * what:   Add an entry to a string table
;;;  * general-use:
;;;  *
;;;  * exparg: st-name , the name of the array of characters
;;;  * exparg: str-val , the (possibly) new value to add
;;;  *
;;;  * doc:    Check for a duplicate string and, if none, then insert a new
;;;  *         string into the string table.  In all cases, returns the
;;;  *         character index of the beginning of the string in the table.
;;;  *
;;;  *         The returned index can be used in expressions like:
;;;  *         @example
;;;  *         string_array + <returned-value>
;;;  *         @end example
;;;  *         @noindent
;;;  *         that will yield the address of the first byte of the inserted
;;;  *         string.  See the @file{strtable.test} AutoGen test for a usage
;;;  *         example.
;;; =*/
;;;
(define string-table-add (lambda (st-name str-val) (begin
   (set! stt-curr    (hash-ref stt-table   st-name))
   (set! stt-idx-tbl (hash-ref stt-curr    "string-indexes"))
   (set! stt-idx     (hash-ref stt-idx-tbl str-val))
   (if (not (number? stt-idx))
       (begin
          (ag-fprintf st-name "%s \"\\0\"\n" (c-string str-val))
          (set! stt-idx (hash-ref stt-curr "current-index"))
          (hash-create-handle! stt-idx-tbl str-val stt-idx)
          (hash-set! stt-curr "current-index"
                    (+ stt-idx (string-length str-val) 1)  )
   )   )
   stt-idx
)))

;;; /*=gfunc   emit_string_table
;;;  *
;;;  * what:   output a string table
;;;  * general-use:
;;;  *
;;;  * exparg: st-name , the name of the array of characters
;;;  *
;;;  * doc:    Emit into the current output stream a
;;;  *         @code{static const char} array named @code{st-name}
;;;  *         that will have @code{NUL} bytes between each inserted string.
;;; =*/
;;;
(define emit-string-table (lambda (st-name) (begin
   (set! stt-curr (hash-ref stt-table   st-name))
   (set! stt-idx  (hash-ref stt-curr "current-index"))
   (ag-fprintf 0 "\nstatic const char %s[%d] =\n" st-name stt-idx)
   (out-resume st-name)

   ;; Columnize the output.
   ;; Remove any leading spaces -- columns adds them itself.
   ;; Glue the "\0" string to its preceding text.
   ;; End the last line with a semi-colon
   ;;
   (emit (shell (string-append
      "(sed 's/^ *//;s/\" \"\\\\0\"/\\\\0\"/' | \
      columns -I4 --spread=1
      ) <<\\_EndStringTable_\n" (out-pop #t) "_EndStringTable_")))
   (emit ";\n")
)))

(use-modules (ice-9 debug))

(read-enable 'positions)

;;; end of agen5/schemedef.scm
