;; The game of nim in gamesman-mini
;;
;; Setup: A set of piles of beans
;; ToMove: Take as many as you want (at least 1) from any pile 
;; ToWin: Take the last bean
;;
;; Position encoded as simply a list of bean pile totals. E.g., (1 3 5 7)
;; Move encoded as list: (from take) 
;;   "From _from_ (0-indexed) pile, take _take_ beans away"

;; Abstract data type: move

(define (make-move from take) (list from take))
(define (get-from M) (car M))
(define (get-take M) (cadr M))

(define (primitive P)
  (if (null? P) 'lose #f))

(define (do-move P M)      
  ; (do-move '(1 3 5 7) '(2 1)) ==> (1 3 4 7)
  (if (= (get-from M) 0) ; pile we want is at head of list
      (let ((pile (- (car P) (get-take M)))) ; new pile size
	(if (= pile 0)
	    (cdr P) ; remove pile if you've taken all beans away
	    (cons pile (cdr P))))
      (cons (car P) ; skip head of list, we don't want this one
	    (do-move (cdr P) ; recurse on pile location
		     (make-move (- (get-from M) 1) (get-take M))))))

(define (generate-moves P) 
  ; (generate-moves '(1 2)) ==> ((0 1) (1 1) (1 2))
  (reduce append (map (lambda (pile index) 
			;; pile = 3, index = i
			;; (i 1) (i 2) (i 3)
			(map (lambda (take) (make-move index take))
			     (list-start-through-end 1 pile)))
		      P
		      (make-indices P))))

(define (make-indices P) 
  ; (make-indices '(5 6 3)) ==> (0 1 2)
  (list-start-through-end 0 (- (length P) 1)))

(define (list-start-through-end start end)
  ; (list-start-through-end 0 2) ==> (0 1 2)
  (if (= start end)
      (list end)
      (cons start (list-start-through-end (+ 1 start) end))))

(test-game 'nim
	   '(()
	     (1)
	     (2)
	     (1 1)
	     (1 2)
	     (1 3)
	     (2 2)
	     (2 3)
	     (1 1 1)
	     (1 1 2)
	     (1 2 2)
	     (1 2 3)
	     (1 3 5 7)))

