(use-modules (guix packages)
	(guix gexp)
	((guix licenses) #:prefix license:)
	(guix build-system meson)
	(gnu packages pkg-config)
	(gnu packages cmake)
	(gnu packages imagemagick)
	(gnu packages curl)
	(gnu packages ncurses)
	(gnu packages tls))

(package
	(name "Scipnet")
	(version "0.0")
	(inputs 
		`(("imagemagick" ,imagemagick)
		("curl" ,curl)
		("curlpp" ,curlpp)
		("ncurses" ,ncurses)
		("openssl" ,openssl)))
	(native-inputs
		`(("pkg-config" ,pkg-config)
		("cmake" ,cmake)))
	(propagated-inputs '())
	(source (local-file "./src/Scipnet" #:recursive? #t))
	(build-system meson-build-system)
	(synopsis "Scipnet: Terminal-based client for the SCP Conversion Project")
	(description
		"Terminal-based viewer client for the SCP Conversion Project. Works both in-browser and in-terminal featuring text-reflow support and character-based image rendering.")
	(home-page "https://github.com/danielbatterystapler/SCP_Conversion_Project")
	(license license:gpl3))

