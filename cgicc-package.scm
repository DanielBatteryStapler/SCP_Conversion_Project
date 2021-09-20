(define-module (gnu packages daniel)
	#:use-module (guix packages)
	#:use-module (guix gexp)
	#:use-module ((guix licenses) #:prefix license:)
	#:use-module (guix build-system gnu)
	#:use-module (guix download)
	;#:use-module (guix utils)
	#:use-module (gnu packages autotools))

(package
	(name "cgicc")
	(version "3.2.19")
	(inputs
		`(("autoconf" ,autoconf)))
	(native-inputs '())
	(propagated-inputs '())
	(source (origin
		(method url-fetch)
		(uri "https://ftp.gnu.org/gnu/cgicc/cgicc-3.2.19.tar.gz")
		(sha256
			(base32
				"1w7pczmi7i09xbz2sgjlv289jx40ibhnqvg3z53k9q4d4ivbj5ra"))
		(patches
			`(,(local-file "cgicc-configure.patch")))))
	(build-system gnu-build-system)
	(synopsis "Cgicc: GNU Cgicc is a C++ api for creating CGI applications")
	(description
		"GNU Cgicc is an ANSI C++ compliant class library that greatly simplifies the creation of CGI applications for the World Wide Web.")
	(home-page "https://www.gnu.org/software/cgicc/")
	(license license:lgpl3))

