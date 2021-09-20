(use-modules (guix packages)
	(guix gexp)
	((guix licenses) #:prefix license:)
	(srfi srfi-1)
	(gnu packages databases))

;we need to modify the soci package in Giux's gnu/packages/database.scm
;because it is built with mariadb, not mysql, and soci doesn't like that
;I believe this needs to either be upstreamed to Guix or soci needs to be modified to build with mariadb
;for now, we're going to modify it for us

(package
	(inherit soci)
	(name "soci-pero-mysql")
	(propagated-inputs
		(cons `("mysql" ,mysql)
			(alist-delete "mariadb"
				(package-propagated-inputs soci)))))
