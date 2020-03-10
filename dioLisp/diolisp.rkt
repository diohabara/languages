;;; This is a lisp(Scheme) interpreter made by diohabara

;; definition of eval
(define (eval exp env)
  (cond [(self-evaluating? exp) exp]
        [(variable? exp) (lookup-variable-value exp env)]
        [(quoted? exp) (text-of-quotation exp)]
        [(assignment? exp) (eval-assignment exp env)]
        [(definition? exp) (eval-definition exp env)]
        [(if? exp) (eval-if exp env)]
        [(lambda? exp) (make-procedure (lambda-parameters exp)
                                       (lambda-body exp)
                                       env)]
        [(begin? exp)
         (eval-sequence (begin-actions exp) env)]
        [(cond? exp) (eval (cond->if exp) env)]
        [(application? exp)
         (apply (eval (operator exp) env)
                (list-of-values (operands exp) env))]
        [else
         (error "Unknown expression type: EVAL" exp)]))


;; definition of apply
; make it possible to use original apply 
(define apply-in-underlying-scheme apply)
; here apply
(define (apply procedure arguments)
  (cond [(primitive-procedure? procedure)
         (apply-primitive-procedure procedure arguments)]
        [(compound-procedure? procedure)
         (eval-sequence
          (procedure-body procedure)
          (extend-environment
           (procedure-parameters procedure)
           arguments
           (procedure-environment procedure)))]
        [else
         (error
          "Unknown procedure type: APPLY" procedure)]))

;; Procedure arguments
(define (list-of-values exps env)
  (if (no-operands? exps)
      '()
      (cons (eval (first-operand exps) env)
            (list-of-values (rest-operands exps) env))))

;; evaluation of if-statements
(define (eval-if exp env)
  (if (true? (eval (if-predicate exp) env))
      (eval (if-consequent exp) env)
      (eval (if-alternative exp) env)))

;; evaluation of sequence
(define (eval-sequence exps env)
  (cond [(last-exp? exps)
         (eval (first-exp exps) env)]
        [else
         (eval (first-exp exps) env)
         (eval-sequence (rest-exps exps) env)]))

;; evaluation of assignments
(define (eval-assignment exp env)
  (set-variable-value! (assignment-variable exp)
                       (eval (assignment-value exp) env)
                       env)
  'ok)

;; definition of eval-definition
(define (eval-definition exp env)
  (define-variable! (definition-variable exp)
    (eval (definition-value exp) env)
    env)
  'ok)

; self-evaluating items are numbers and strings
(define (self-evaluating? exp)
  (cond [(number? exp) #t]
        [(string? exp) #t]
        [else #f]))

;; see if exp is symbol
(define (variable? exp) (symbol? exp))

;; quotation
; see if exp has a tag of quote
(define (quoted? exp) (tagged-list? exp 'quote))
; get expression
(define (text-of-quotation exp) (cadr exp))

;; see if expression has a tag
(define (tagged-list? exp tag)
  (if (pair? exp)
      (eq? (car exp) tag)
      #f))

;; assignment
; see if exp is an assignment
(define (assignment? exp) (tagged-list? exp 'set!))
; get variable
(define (assignment-variable exp) (cadr exp))
; get value
(define (assignment-value exp) (caddr exp))

;; definition
; see if exp is define
(define (definition? exp) (tagged-list? exp 'define))
; get variable 
(define (definition-variable exp)
  (if (symbol? (cadr exp))
      (cadr exp)
      (caadr exp)))
; get value
(define (definition-value exp)
  (if (symbol? (cadr exp))
      (caddr exp)
      (make-lambda (cdadr exp) ; formal parameters
                   (cddr exp)))) ; body

; lambda
; see if exp is lambda
(define (lambda? exp) (tagged-list? exp 'lambda ))
; get parameters of lambda
(define (lambda-parameters exp) (cadr exp))
; get body of lambda
(define (lambda-body exp) (cddr exp))
; make lambda with parameters and body
(define (make-lambda parameters body)
  (cons 'lambda (cons parameters body)))

;; conditional
; see if it's exp
(define (if? exp) (tagged-list? exp 'if))
; get the predicate
(define (if-predicate exp) (cadr exp))
; get the consequence
(define (if-consequent exp) (caddr exp))
; get the alternative consequence
(define (if-alternative exp)
  (if (not (null? (cdddr exp)))
      (cadddr exp)
      'false))
; make an if-statement with predicate, consequent and alternative
(define (make-if predicate consequent alternative)
  (list 'if predicate consequent alternative))

;; sequence
; see if exp is begin
(define (begin? exp) (tagged-list? exp 'begin))
; get the cdr of exp (when the car of exp is begin)
(define (begin-actions exp) (cdr exp))
; see if seq is the last expression
(define (last-exp? seq) (null? (cdr seq)))
; get the first expression of seq
(define (first-exp seq) (car seq))
; get the rest (not first) expression of seq
(define (rest-exps seq) (cdr seq))
; make seq an expression
(define (sequence->exp seq)
  (cond [(null? seq) seq]
        [(last-exp? seq) (first-exp seq)]
        [else (make-begin seq)]))
; attach a tag to a seq
(define (make-begin seq) (cons 'begin seq))

;; procedure
; see if exp is pair
(define (application? exp) (pair? exp))
; get the car of exp
(define (operator exp) (car exp))
; get the cdr of exp
(define (operands exp) (cdr exp))
; see if ops is null
(define (no-operands? ops) (null? ops))
; get the first operand from ops
(define (first-operand ops) (car ops))
; get the rest operands from ops
(define (rest-operands ops) (cdr ops))

;; Derived opressions
; see if exp is cond
(define (cond? exp) (tagged-list? exp 'cond))
; get cond-clauses of cond
(define (cond-clauses exp) (cdr exp))
; see if clause is else-clause
(define (cond-else-clause? clause)
  (eq? (cond-predicate clause) 'else))
; get predicate
(define (cond-predicate clause) (car clause))
; get actioins
(define (cond-actions clause) (cdr clause))
; expand if-clauses
(define (cond-if exp) (expand-clauses (cond-clauses exp)))
; expand procedure
(define (expand-clauses clauses)
  (if (null? clauses)
      'false ; no else clause
      (let ((first (car clauses))
            (rest (cdr clauses)))
        (if (cond-else-clause? first)
            (if (null? rest)
                (sequence->exp (cond-actions first))
                (error "ELSE clause isn't last: COND->IF"
                       clauses))
            (make-if (cond-predicate first)
                     (sequence->exp (cond-actions first))
                     (expand-clauses rest))))))

;; Testing of predicates
(define (true? x) (not (eq? x false)))
(define (false? x) (eq? x false))

;;; Procedure
; make a procedure with parameters, body and env
(define (make-procedure parameters body env)
  (list 'procedure parameters body env))
; see if p is compound procedure
(define (compound-procedure? p)
  (tagged-list? p 'procedure))
; get parameters
(define (procedure-parameters p) (cadr p))
; get body
(define (procedure-body p) (caddr p))
; get environment
(define (procedure-environment p) (cadddr p))
;; list procedures
; pairs as procedure
(define (cons x y) (lambda (m) (m x y)))
(define (car z) (z (lambda (p q) p)))
(define (cdr z) (z (lambda (p q) q)))
;; the last item of the list
(define (list-ref items n)
  (if (= n 0)
    (car items)
    (list-ref (cdr items) (- n 1))))
;; mapping procedure to items
(define (map proc items)
  (if (null? items)
    `()
    (cons (proc (car items)) (map proc (cdr items)))))
;
(define (add-lists list1 list2)
  (cond [(null? list1) list2]
        [(null? list2) list1]
        [else (cons (+ (car list1) (car list2))
                    (add-lists (cdr list1) (cdr list2)))]))
;; make list of ones
(define ones (cons 1 ones))
; make list of integers
(define integers (cons 1 (add-lists ones integers)))

;;; Operations on Environments
; get the cdr of env
(define (enclosing-environment env) (cdr env))
; get the car of env
(define (first-frame env) (car env))
; empty environment is '()
(define the-empty-environment '())
; make a frame with variables and values
(define (make-frame variables values)
  (cons variables values))
; get variables from frame
(define (frame-variables frame) (car frame))
; get values from frame
(define (frame-values frame) (cdr frame))
; set frame's variables and values as var and val
(define (add-binding-to-frame! var val frame)
  (set-car! frame (cons var (car frame)))
  (set-cdr! frame (cons val (cdr frame))))
; get the length of list
(define (length lis)
  (if (null? lis)
      0
      (+ 1 (length (cdr lis)))))
; extend base-env with a new environment consisting of vars and vals
(define (extend-environment vars vals base-env)
  (if (= (length vars) (length vals))
      (cons (make-frame vars vals) base-env)
      (if (< (length var) (length vals))
          (error "Too many arguments supplied" vars vals)
          (error "Too few arguments supplied" vars vals))))
; look up var from env
(define (lookup-variable-value var env)
  ; see if env has var
  (define (env-loop env)
    ; if env has var, then return the coressponding value
    (define (scan vars vals)
      (cond [(null? vars)
             (env-loop (enclosing-environment env))]
            [(eq? var (car vars)) (car vals)]
            [else (scan (cdr vars) (cdr vals))]))
    ; see if env is empty
    ; then return error
    (if (eq? env the-empty-environment)
        (error "Unbound variable" var)
        (let ((frame (first-frame env)))
          (scan (frame-variables frame)
                (frame-values frame)))))
  (env-loop env))
; set var and val in env
(define (set-variable-value! var val env)
  (define (env-loop env)
    (define (scan vars vals)
      (cond [(null? vars)
             (env-loop (enclosing-environment env))]
            [(eq? var (car vars)) (set-car! vals val)]
            [else (scan (cdr vars) (cdr vals))]))
    (if (eq? env the-empty-environment)
        (error "Unbound variable: SET!" var)
        (let ((frame (first-frame env)))
          (scan (frame-variables frame)
                (frame-values frame)))))
  (env-loop env))
; set var whose value is val in env
(define (define-variable! var val env)
  (let ((frame (first-frame env)))
    (define (scan vars vals)
      (cond [(null? vars)
             (add-binding-to-frame! var val frame)]
            [(eq? var (car vars)) (set-car! vals val)]
            [else (scan (cdr vars) (cdr vals))]))
    (scan (frame-variables frame) (frame-values frame))))
; see if proc is a primitive procedure
(define (primitive-procedure? proc)
  (tagged-list? proc 'primitive))
; get primitive implementation from proc
(define (primitive-implementation proc) (cadr proc))
; set primitive procedures
(define primitive-procedures
  (list (list 'car car)
        (list 'cdr cdr)
        (list 'cons cons)
        (list 'null? null?)
        (list '+ +)
        (list '+ -)
        (list '* *)
        (list '/ /)
        (list 'list-ref list-ref)
        (list 'map map)
        (list 'scale-list scale-list)
        (list 'length length))
; get the name of primitive procedures
(define (primitive-procedure-names)
  (map car primitive-procedures))
; attach 'primitive to primitive procedures
(define (primitive-procedure-objects)
  (map (lambda (proc) (list 'primitive (cadr proc)))
       primitive-procedures))
; apply proc to args
(define (apply-primitive-procedure proc args)
  (apply-in-underlying-scheme
   (primitive-implementation proc) args))

;; print prompt for input and output
(define (prompt-for-input string)
  (newline) (newline) (display string) (newline))
(define (announce-output string)
  (newline) (display string) (newline))
(define input-prompt ";;; M-Eval input:")
(define output-prompt ";;; M-Eval value:")

; 
(define (user-print object)
  (if (compound-procedure? object)
      (display (list 'compound-procedure
                     (procedure-parameters object)
                     (procedure-body object)
                     '<procedure-env>))
      (display object)))
;; This procedure computes input and print output infinitely.
;; TODO: If you would like to stop, type "quit".
(define (driver-loop)
  (prompt-for-input input-prompt)
  (let ((input (read)))
    (let ((output (eval input the-global-environment)))
      (announce-output output-prompt)
      (user-print output)))
  (driver-loop))

;; setting up an environment
(define (setup-environment)
  (let ((initial-env
         (extend-environment (primitive-procedure-names)
                             (primitive-procedure-objects)
                             the-empty-environment)))
    (define-variable! 'true #t initial-env)
    (define-variable! 'false #f initial-env)
    initial-env))

;; set up an initial environment and start the scheme interpreter
(define the-global-environment (setup-environment))
(driver-loop)