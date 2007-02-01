;; The game of 1,2,...,10 in gamesman-mini
;;
;; Setup: Start with 0 total
;; ToMove: Increment total by 1 or 2
;; ToWin: Bring total over GOAL (default 10) first
;;
;; Position encoded as simply a number (the total)
;; Move encoded as simply the increment

(define *goal* 10)
(define (primitive P)      (if (>= P *goal*) 'lose #f))
(define (do-move P M)      (+ P M))
(define (generate-moves P) '(1 2))

(test-game 1210 '(0 1 2 3 4 5 6 7 8 9 10))
