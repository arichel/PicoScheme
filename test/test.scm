(define port (open-output-file "test.txt"))
(write "hallo" port)
(newline port)
(display "Paul" port)

(flush-output-port port)
(close-output-port port)

(call-with-output-file "test.txt"
  (lambda (port)
    (write "Hello Mr. tally man,\ntally me banana...\n" port)
   ))


(do ((i 0 (+ i 1)))
    ((= i 100))
  (write "hallo paul: " port)
;  (write i port)
  (newline port))
