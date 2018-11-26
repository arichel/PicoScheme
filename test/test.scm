(define port (open-input-file "test.txt"))

(write "hallo" port)
(newline port)
(display "Paul" port)

(flush-output-port port)
(close-output-port port)

(call-with-output-file "test.txt"
  (lambda (port)
    (write "Hello Mr. tally man,\ntally me banana...\n" port)
   ))


(with-exception-handler
 (lambda (obj)
   (display obj)(newline)
   (display (error-object? obj))(newline)
;;   (display (error-object-message obj))(newline)
;;   (display (error-object-irritants obj)) (newline)
   obj)

 (lambda ()
   (+  (error "hallo" 100) 1 2 3 4 5)))

(define (values . args)
  (call-with-current-continuation
   (lambda (cont)
     (display args)(newline)
     (apply cont args))))

(values 1 2 3)


(call-with-values
    (lambda ()
      (when (> 3 4)
      (values 1 2 3)))
  (lambda (x y z)
    (+ x y z)))



(do ((i 0 (+ i 1)))
    ((= i 100))
  (write "hallo paul: " port)
;  (write i port)
  (newline port))
