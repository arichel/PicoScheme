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
      `((lambda ,(map car bindings) . ,body) . ,(map cadr bindings))) )

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

;;
;; return a function which cycles between arguments at each call:
;;   ex.: (define flip-flop (make-cycle 1 2)) => 1, 2, 1, 2, ....
(define (make-cycle . args)
  (if (null? args)
      (lambda () args)
      (let ((nxt args)(arg ()))
        (lambda ()
          (set! arg (car nxt))
          (set! nxt (if (null? (cdr nxt)) args (cdr nxt)))
          arg))))
