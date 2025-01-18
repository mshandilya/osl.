(defvar v 1)

(defun foo ()
  (print v))

(defun bar (a)
  (if a (let ((v 10)) (foo)) (foo)))

(bar nil)  ; 1
(bar t)    ; 10