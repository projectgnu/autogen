
;;;  AutoGen copyright 1992-2004 Bruce Korb
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
;;;            59 Temple Place - Suite 330,
;;;            Boston,  MA  02111-1307, USA.
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

;;; /*=sfunc   make_header_guard
;;;  *
;;;  * what:   create ifndef/define guard
;;;  *
;;;  * exparg: name , header group name
;;;  *
;;;  * doc:    This function will create a @code{#ifndef}/@code{#define}
;;;  *         sequence for protecting a header from multiple evaluation.
;;;  *         It will also set the Scheme variable @code{header-file}
;;;  *         to the name of the file being protected and it will set
;;;  *         @code{header-guard} to the name of the @code{#define} being
;;;  *         used to protect it.  It is expected that this will be used
;;;  *         as follows:
;;;  *         @example
;;;  *         [+ (make-header-guard "group_name") +]
;;;  *         ...
;;;  *         #endif /* [+ (. header-guard) +]
;;;  *
;;;  *         #include "[+ (. header-file)  +]"
;;;  *         @end example
;;;  *         @noindent
;;;  *         The @code{#define} name is composed as follows:
;;;  *
;;;  *         @enumerate
;;;  *         @item
;;;  *         The first element is the string argument and a separating
;;;  *         underscore.
;;;  *         @item
;;;  *         That is followed by the name of the header file with
;;;  *         illegal characters mapped to underscores.
;;;  *         @item
;;;  *         The end of the name is always, "@code{_GUARD}".
;;;  *         @end enumerate
;;;  *         
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

(define-macro (defined-as predicate symbol)
  `(and (defined? ',symbol) (,predicate ,symbol)))

;;; /*=sfunc   html_escape_encode
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

(use-modules (ice-9 debug))

(read-enable 'positions)

;;; end of agen5/schemedef.scm
