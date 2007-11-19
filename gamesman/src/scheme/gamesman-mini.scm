;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                        
;; NAME:        gamesman-mini.scm
;; 
;; DESCRIPTION: GAMESMAN Mini (Scheme) -- a solver for finite, two-person
;;              perfect-information abstract strategy games
;; 
;; AUTHORS:     2007Sp UC Berkeley GamesCrafters
;;              Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
;;              http://GamesCrafters.berkeley.edu
;;
;; DATE:        2007-01-31
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Three main functions [P is a position, M is a move]
;;
;; (primitive P)      ==> {win, lose, tie, #f}
;; (do-move P M)      ==> P
;; (generate-moves P) ==> (M1 M2 ... Mn)
;; (solve P)          ==> {win, lose, tie}
;;
;; DEFINITIONS OF VALUES (assume alternating turns)
;;
;; win  <=> there exists a losing child
;; lose <=> all children are wins
;; tie  <=> there is at least one tie child and no loses

(define (solve P) ;; smallest possible solver
  (or (primitive P)
      (let ((values (map (lambda (M) (solve (do-move P M))) (generate-moves P))))
	(cond ((member 'lose values) 'win)
	      ((member 'tie  values) 'tie)
     	      (else 'lose)))))


(define (solve game P) ;; loop-free memoized solver!
  ; (solve 'nim '(1 3 5 7)) ==> lose
  (or (get game P)
      (begin (put game P (or (primitive P)
			   (let ((values (map (lambda (move) 
						(solve game (do-move P move)))
					      (generate-moves P))))
			     (cond ((member 'lose values) 'win)
				   ((member 'tie  values) 'tie)
				   (else 'lose)))))
	     (get game P))))

(define (test-game game Ps)
  (display game)
  (display " solved and tested on the following positions:\n")
  (for-each (lambda (P result)
	      (display P)
	      (display " : ")
	      (display result)
	      (display "\n"))
	    Ps
	    (map (lambda (P) (solve game P)) Ps)))
