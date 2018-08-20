;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; PicoScheme initialization file to be loaded on each start-up
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Quasiquote macro copied from Peter Norvig's JScheme, see http://www.norvig.com
;;
(define-macro  (quasiquote x)

  (define (constant? exp)
    (if (pair? exp)
        (eq? (car exp) 'quote)
        (not (symbol? exp))))

  (define (combine-skeletons left right exp)
    (cond
     ((and (constant? right) (constant? left))
      (if (and (eqv? (eval left)  (car exp))
               (eqv? (eval right) (cdr exp)))
          (list 'quote exp)
          (list 'quote (cons (eval left) (eval right)))))

     ((null? right)
      (list 'list left))

     ((and (pair? right) (eq? (car right) 'list))
      (cons 'list (cons left (cdr right))))

     (else
      (list 'cons left right))))

  (define (expand-quasiquote exp nesting)
    (cond
     ((vector? exp)
      (list 'apply 'vector (expand-quasiquote (vector->list exp) nesting)))

     ((not (pair? exp))
      (if (constant? exp)
          exp (list 'quote exp)))

     ((and (eq? (car exp) 'unquote) (= (length exp) 2))
      (if (= nesting 0)
          (cadr exp)
          (combine-skeletons ''unquote
                             (expand-quasiquote (cdr exp) (- nesting 1))
                             exp)))

     ((and (eq? (car exp) 'quasiquote) (= (length exp) 2))
      (combine-skeletons ''quasiquote
                         (expand-quasiquote (cdr exp) (+ nesting 1))
                         exp))

     ((and (pair? (car exp))
           (eq? (caar exp) 'unquote-splicing)
           (= (length (car exp)) 2))

      (if (= nesting 0)
          (list 'append (cadr (car exp))
                (expand-quasiquote (cdr exp) nesting))
          (combine-skeletons (expand-quasiquote (car exp) (- nesting 1))
                             (expand-quasiquote (cdr exp) nesting)
                             exp)))
     (else
      (combine-skeletons (expand-quasiquote (car exp) nesting)
                         (expand-quasiquote (cdr exp) nesting) exp))))

  (expand-quasiquote x 0))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-macro (let bindings . body)
  (define (varval v)
    (string->symbol (string-append (symbol->string v) "=")))

  (define (named-let name bindings body)
    ((lambda (new-bindings)
       `(let ,(cons `(,name #f) new-bindings)
          (set! ,name (lambda ,(map car bindings) . ,body))
          (,name . ,(map car  new-bindings))))
     (map (lambda (b)
            `(,(varval (car b)) ,(cadr b))) bindings)))

  (if (symbol? bindings)
      (named-let bindings (car body) (cdr body))
      `((lambda ,(map car bindings) . ,body) . ,(map cadr bindings))))

(define-macro  (let* bindings . body)
  (if (null? bindings) `((lambda () . ,body))
      `(let (,(car bindings))
         (let* ,(cdr bindings) . ,body))))

(define-macro (letrec bindings . body)
  (let ((vars (map car  bindings))
        (vals (map cadr bindings)))

    `(let ,(map (lambda (var)
                  `(,var #f)) vars)

       ,@(map (lambda (var val)
                `(set! ,var ,val)) vars vals)
       . ,body)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-macro (do bindings test-and-result . body)
  (let ((variables (map car bindings))
        (inits     (map cadr bindings))
        (steps     (map (lambda (clause)
                          (if (null? (cddr clause))
                              (car clause)
                              (caddr clause)))
                        bindings))
        (test   (car test-and-result))
        (result (cdr test-and-result))
        (loop   (gensym)))

    `(letrec ((,loop (lambda ,variables
                       (if ,test
                           ,(if (null? result) #t
                                `(begin . ,result))
                           (begin
                             ,@body
                             (,loop . ,steps))))))
       (,loop . ,inits)) ))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-macro (case expr . cases)
  (let ((item (gensym)))

    (define (do-case case)
      (cond ((not (pair? case)) (error "bad syntax in case" case))
            ((eq? (car case) 'else) case)
            (else `((member ,item ',(car case)) . ,(cdr case)))))

    `(let ((,item ,expr))
       (cond . ,(map do-case cases)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; define-struct - macro from 'Teach Yourself Scheme in Fixnum Days' by Dorai Sitram
;;                 to define a structure data type and procedures for modifying
;;                 and accessing its fields.
;;
;; example:   (define-struct person height weight (age 0))
;;
;;            => make-person, person.height, person.weight, person.age
;;               set!person.height, ...
;;
(define-macro (define-struct name . prop)
  (let ((s-s (symbol->string name))
        (n (length prop)))

    (let* ((n+1 (+ n 1))
           (vv (make-vector n+1)))

      (let loop ((i 1) (prop prop))
        (if (<= i n)
            (let ((f (car prop)))
              (vector-set! vv i (if (pair? f) (cadr f) '(if #f #f)))
              (loop (+ i 1) (cdr prop)))))

      (let ((prop (map (lambda (f) (if (pair? f) (car f) f)) prop)))
        `(begin
           (define ,(string->symbol (string-append "make-" s-s))
             (lambda fvfv
               (let ((st (make-vector ,n+1)) (prop ',prop))
                 (vector-set! st 0 ',name)
                 ,@(let loop ((i 1) (r '()))
                     (if (>= i n+1) r
                         (loop (+ i 1) (cons `(vector-set! st ,i ,(vector-ref vv i)) r))))
                 (let loop ((fvfv fvfv))
                   (if (not (null? fvfv))
                       (begin
                         (vector-set! st (+ (list-position (car fvfv) prop) 1) (cadr fvfv))
                         (loop (cddr fvfv)))))
                   st)))

           ,@(let loop ((i 1) (procs '()))
               (if (>= i n+1) procs
                   (loop (+ i 1)
                         (let ((f (symbol->string (list-ref prop (- i 1)))))
                           (cons
                            `(define ,(string->symbol (string-append s-s "." f))
                               (lambda (x) (vector-ref x ,i)))
                            (cons
                             `(define ,(string->symbol (string-append "set!" s-s "." f))
                                (lambda (x v)
                                  (vector-set! x ,i v)))
                             procs))))))

           (define ,(string->symbol (string-append s-s "?"))
             (lambda (x)
               (and (vector? x) (eqv? (vector-ref x 0) ',name)))) ',name)))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Basic format string support, author Scott G. Miller
;;
;; Syntax: (format format-string [obj ...]) -> string
;;
;; Escape sequence:
;;  ~a - The corresponding value is inserted into the string as if printed with display.
;;  ~s - The corresponding value is inserted into the string as if printed with write.
;;  ~% - A newline is inserted.
;;  ~~ - A tilde '~' is inserted.
;;
;; Example: (format "This is a list: ~s~%" '(one "two" 3))
;;
(define (format format-string . objects)
    (let ((buffer (open-output-string)))
      (let loop ((format-list (string->list format-string))
                 (objects objects))
        (cond ((null? format-list) (get-output-string buffer))
              ((char=? (car format-list) #\~)
               (if (null? (cdr format-list))
                   (error 'format " - incomplete escape sequence")
                   (case (cadr format-list)
                     ((#\a)
                      (if (null? objects)
                          (error 'format " - no value for escape sequence")
                          (begin
                            (display (car objects) buffer)
                            (loop (cddr format-list) (cdr objects)))))

                     ((#\s)
                      (if (null? objects)
                          (error 'format " - no value for escape sequence")
                          (begin
                            (write (car objects) buffer)
                            (loop (cddr format-list) (cdr objects)))))

                     ((#\%)
                      (newline buffer)
                      (loop (cddr format-list) objects))

                     ((#\~)
                      (write-char #\~ buffer)
                      (loop (cddr format-list) objects))

                     (else
                      (error 'format " - unrecognized escape sequence")))))

              (else (write-char (car format-list) buffer)
                    (loop (cdr format-list) objects))))))

(define-macro (print fmt . vals)
  `(display (format ,fmt ,@vals)))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; While loop with non-local exit.
;;
;; Example:
;;
;;    (let ((i 0)
;;          (nmax 100))
;;      (while (< i nmax)
;;             (display i)
;;             (newline)
;;             (unless (< i 10) break)
;;             (set! i (+ i 1))))
;;
(define-macro (while pred . body)
  (letrec ((replace (lambda (symb new expr)
                      (cond ((null?   expr) ())
                            ((symbol? expr) (if (eq? expr symb) new expr))
                            ((pair?   expr) (cons (replace symb new (car expr))
                                                  (replace symb new (cdr expr))))
                            (else expr)))))
    (let ((body (replace 'break '(break (if #f #f)) body))
          (loop (gensym)))

      `(call-with-current-continuation
        (lambda (break)
          ((lambda (,loop)(when ,pred ,@body (,loop ,loop)))
           (lambda (,loop)(when ,pred ,@body (,loop ,loop)))) )))))
