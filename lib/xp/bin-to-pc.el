
(defun bin-to-pc (input output)
  (save-excursion
    (set-buffer (let ((font-lock-mode nil))
		  (find-file-noselect output)))
    (fundamental-mode)
    (erase-buffer)
    (insert-file-contents input)
    (goto-char (point-min))
    (let ((count 0))
      (while (not (eobp))
	(let ((c1 (or (char-after (point)) 0))
	      (c2 (or (char-after (1+ (point))) 0)))
	  (insert (format "0x%02x%02x, " c2 c1))
	  (or (eobp) (delete-char 1))
	  (or (eobp) (delete-char 1))
	  (cond ((> (setq count (1+ count)) 8)
		 (setq count 0)
		 (insert "\n")))
	  )))
    (delete-char -2)
    (insert "\n")
    (save-buffer)
    ))

(defun batch-bin-to-pc ()
  (defvar command-line-args-left)	;Avoid 'free variable' warning
  (if (not noninteractive)
      (error "batch-bin-to-pc is to be used only with -batch"))
  (let ((in  (expand-file-name (nth 0 command-line-args-left)))
	(out (expand-file-name (nth 1 command-line-args-left)))
	(version-control 'never))
    (or (and in out)
	(error
	 "usage: emacs -batch -f batch-bin-to-pc input-file output-file"))
    (setq command-line-args-left (cdr (cdr command-line-args-left)))
    (let ((auto-mode-alist nil))
      (bin-to-pc in out))))
