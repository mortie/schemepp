(define (cstr str)
  (define (rec str ch k is-nl?)
    (if is-nl? (display "\""))
    (cond ((= k (string-length str))
           (display "\""))
          ((char=? ch #\newline)
           (display "\\n\"\n")
           (rec str (string-ref str k) (+ k 1) #t))
          ((char=? ch #\")
           (display "\\\"")
           (rec str (string-ref str k) (+ k 1) #f))
          ((char=? ch #\\)
           (display "\\\\")
           (rec str (string-ref str k) (+ k 1) #f))
          (else
           (display ch)
           (rec str (string-ref str k) (+ k 1) #f))))
  (rec str (string-ref str 0) 1 #t))
