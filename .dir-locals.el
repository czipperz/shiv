((nil . ((eval .
               (let ((dl (dir-locals-find-file buffer-file-name)))
                 (setq dl (if (consp dl) (car dl) (file-name-directory dl)))
                 (setq cmake-ide-build-dir (expand-file-name "out" dl)))))))
