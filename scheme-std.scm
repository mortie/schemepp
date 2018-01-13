(define (cstr str)
  (define (strify ch)
    (cond ((char=? ch #\newline)
           "\\n\"\n\"")
          ((char=? ch #\")
            "\\\"")
          ((char=? ch #\\)
           "\\\\")
          (else
           (make-string 1 ch))))

  (define (rec ch k acc)
    (cond ((>= k (string-length str))
           (string-append acc (strify ch) "\""))
          (else
           (rec (string-ref str k)
                (+ k 1)
                (string-append acc (strify ch))))))

  (rec (string-ref str 0) 1 "\""))
