;;; tex.scm -- implement Scheme output routines for TeX
;;;
;;;  source file of the GNU LilyPond music typesetter
;;; 
;;; (c) 1998--2001 Jan Nieuwenhuizen <janneke@gnu.org>
;;; Han-Wen Nienhuys <hanwen@cs.uu.nl>



;;
;; todo: this dispatch is totally LAME
 
(define (tex-scm action-name)
  (define (unknown) 
    "%\n\\unknown%\n")


  (define (select-font name-mag-pair)
    (let*
	(
	 (c (assoc name-mag-pair font-name-alist))
	 )

      (if (eq? c #f)
	  (begin
	    (display "FAILED\n")
	    (display (object-type (car name-mag-pair)))
	    (display (object-type (caaar font-name-alist)))

	    (ly-warn (string-append
		      "Programming error: No such font known "
		      (car name-mag-pair) " "
		      (ly-number->string (cdr name-mag-pair))
		      ))
	    "") ; issue no command
	  (string-append "\\" (cddr c)))
      
      
      ))
  
  (define (beam width slope thick)
    (embedded-ps ((ps-scm 'beam) width slope thick)))

  (define (bracket arch_angle arch_width arch_height height arch_thick thick)
    (embedded-ps ((ps-scm 'bracket) arch_angle arch_width arch_height height arch_thick thick)))

  (define (dashed-slur thick dash l)
    (embedded-ps ((ps-scm 'dashed-slur)  thick dash l)))

  (define (hairpin thick w sh eh)
    (embedded-ps ((ps-scm 'hairpin) thick w sh eh)))

  (define (char i)
    (string-append "\\char" (inexact->string i 10) " "))
  
  (define (dashed-line thick on off dx dy)
    (embedded-ps ((ps-scm 'dashed-line) thick on off dx dy)))

  (define (font-load-command name-mag command)
    (string-append
     "\\font\\" command "="
     (car name-mag)
     " scaled "
     (ly-number->string (inexact->exact (* 1000  (cdr name-mag))))
     "\n"))

  (define (ez-ball c l b)
    (embedded-ps ((ps-scm 'ez-ball) c  l b)))
  (define (embedded-ps s)
    (string-append "\\embeddedps{" s "}"))

  (define (comment s)
    (string-append "% " s))
  
  (define (end-output) 
	(begin
; uncomment for some stats about lily memory	  
;		(display (gc-stats))
    (string-append "\n\\EndLilyPondOutput"
		   ; Put GC stats here.
		   )))
  
  (define (experimental-on)
    "")

  (define (repeat-slash w a t)
    (embedded-ps ((ps-scm 'repeat-slash) w a t)))
  
  (define (font-switch i)
    (string-append
     "\\" (font i) "\n"))

  (define (font-def i s)
    (string-append
     "\\font" (font-switch i) "=" s "\n"))

  (define (header-end)
    (string-append
     "\\special{\\string! "
     
     ;; URG: ly-gulp-file: now we can't use scm output without Lily
     (if use-regex
	 ;; fixed in 1.3.4 for powerpc -- broken on Windows
	 (regexp-substitute/global #f "\n"
				   (ly-gulp-file "music-drawing-routines.ps") 'pre " %\n" 'post)
	 (ly-gulp-file "music-drawing-routines.ps"))
     (if (defined? 'ps-testing) "/testing true def%\n" "")
     "}"
     "\\input lilyponddefs \\outputscale=\\lilypondpaperoutputscale pt\\turnOnPostScript"))

  ;; Note: this string must match the string in ly2dvi.py!!!
  (define (header creator generate) 
    (string-append
     "% Generated automatically by: " creator generate "\n"))

  (define (invoke-char s i)
    (string-append 
     "\n\\" s "{" (inexact->string i 10) "}" ))

  (define (invoke-dim1 s d)
    (string-append
     "\n\\" s "{" (number->dim d) "}"))
  (define (pt->sp x)
    (* 65536 x))
  
  ;;
  ;; need to do something to make this really safe.
  ;;
  (define (output-tex-string s)
      (if security-paranoia
	  (if use-regex
	      (regexp-substitute/global #f "\\\\" s 'pre "$\\backslash$" 'post)
	      (begin (display "warning: not paranoid") (newline) s))
	  s))
      
  (define (lily-def key val)
    (let ((tex-key
	   (if use-regex
	       ;; fixed in 1.3.4 for powerpc -- broken on Windows
	       (regexp-substitute/global
		#f "_" (output-tex-string key) 'pre "X" 'post)
	       (output-tex-string key)))
	  (tex-val (output-tex-string val)))
      (if (equal? (sans-surrounding-whitespace tex-val) "")
	  (string-append "\\let\\" tex-key "\\undefined\n")
	  (string-append "\\def\\" tex-key "{" tex-val "}\n"))))
		
  (define (number->dim x)
    (string-append
     ;;ugh ly-* in backend needs compatibility func for standalone output
     (ly-number->string x) " \\outputscale "))

  (define (placebox x y s) 
    (string-append 
     "\\placebox{"
     (number->dim y) "}{" (number->dim x) "}{" s "}\n"))

  (define (bezier-sandwich l thick)
    (embedded-ps ((ps-scm 'bezier-sandwich) l thick)))

  (define (start-line ht)
      (string-append"\\vbox to " (number->dim ht) "{\\hbox{%\n"))

  (define (stop-line) 
    "}\\vss}\\interscoreline\n")
  (define (stop-last-line)
    "}\\vss}")
  
  (define (filledbox breapth width depth height)
    (if (defined? 'ps-testing)
	(embedded-ps
	 (string-append (numbers->string (list breapth width depth height))
			" draw_box" ))
	(string-append 
	 "\\kern" (number->dim (- breapth))
	 "\\vrule width " (number->dim (+ breapth width))
	 "depth " (number->dim depth)
	 "height " (number->dim height) " ")))

  (define (text s)
    (string-append "\\hbox{" (output-tex-string s) "}"))
  
  (define (tuplet ht gapx dx dy thick dir)
    (embedded-ps ((ps-scm 'tuplet) ht gapx dx dy thick dir)))

  (define (volta h w thick vert_start vert_end)
    (embedded-ps ((ps-scm 'volta) h w thick vert_start vert_end)))

  (define (define-origin file line col)
    (if (procedure? point-and-click)
	(string-append "\\special{src\\string:"
		       (point-and-click line col file)
		       "}" )
	"")
  )

  ; no-origin not yet supported by Xdvi
  (define (no-origin) "")
  
  ;; TeX
  ;; The procedures listed below form the public interface of TeX-scm.
  ;; (should merge the 2 lists)
  (cond ((eq? action-name 'all-definitions)
	 `(begin
	    (define font-load-command ,font-load-command)
	    (define beam ,beam)
	    (define bezier-sandwich ,bezier-sandwich)
	    (define bracket ,bracket)
	    (define char ,char)
	    (define dashed-line ,dashed-line) 
	    (define dashed-slur ,dashed-slur) 
	    (define hairpin ,hairpin) 
	    (define end-output ,end-output)
	    (define experimental-on ,experimental-on)
	    (define filledbox ,filledbox)
	    (define font-def ,font-def)
	    (define font-switch ,font-switch)
	    (define header-end ,header-end)
	    (define lily-def ,lily-def)
	    (define ez-ball ,ez-ball)
	    (define header ,header) 
	    (define invoke-char ,invoke-char) 
	    (define invoke-dim1 ,invoke-dim1)
	    (define placebox ,placebox)
	    (define select-font ,select-font)
	    (define start-line ,start-line)
	    (define stop-line ,stop-line)
	    (define stop-last-line ,stop-last-line)
	    (define text ,text)
	    (define tuplet ,tuplet)
	    (define volta ,volta)
	    (define define-origin ,define-origin)
	    (define no-origin ,no-origin)
	    (define repeat-slash ,repeat-slash)
	    ))

	((eq? action-name 'beam) beam)
	((eq? action-name 'tuplet) tuplet)
	((eq? action-name 'bracket) bracket)
	((eq? action-name 'hairpin) hairpin)
	((eq? action-name 'dashed-line) dashed-line) 
	((eq? action-name 'dashed-slur) dashed-slur) 
	((eq? action-name 'end-output) end-output)
	((eq? action-name 'experimental-on) experimental-on)
	((eq? action-name 'font-def) font-def)
	((eq? action-name 'font-switch) font-switch)
	((eq? action-name 'header-end) header-end)
	((eq? action-name 'lily-def) lily-def)
	((eq? action-name 'header) header) 
	((eq? action-name 'invoke-char) invoke-char) 
	((eq? action-name 'invoke-dim1) invoke-dim1)
	((eq? action-name 'placebox) placebox)
	((eq? action-name 'bezier-sandwich) bezier-sandwich)
	((eq? action-name 'start-line) start-line)
	((eq? action-name 'stem) stem)
	((eq? action-name 'stop-line) stop-line)
	((eq? action-name 'stop-last-line) stop-last-line)
	((eq? action-name 'volta) volta)
	(else (error "unknown tag -- PS-TEX " action-name))
	)
  )

(define (scm-tex-output)
  (primitive-eval (tex-scm 'all-definitions)))
