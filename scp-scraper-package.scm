(use-modules (guix packages)
	(guix gexp)
	((guix licenses) #:prefix license:)
	(guix build-system meson)
	(gnu packages pkg-config)
	(gnu packages cmake)
	(gnu packages cpp)
	(gnu packages boost)
	(gnu packages curl)
	(gnu packages tls)
	(gnu packages certs))

(package
	(name "ScpScraper")
	(version "0.0")
	(inputs 
		`(("json-modern-cxx" ,json-modern-cxx)
		("boost" ,boost)
		("curl" ,curl)
		("curlpp" ,curlpp)
		("openssl" ,openssl)))
	(native-inputs
		`(("pkg-config" ,pkg-config)
		("cmake" ,cmake)))
	(propagated-inputs 
		`(("nss-certs" ,nss-certs)))
	(source (local-file "./src/ScpScraper" #:recursive? #t))
	(build-system meson-build-system)
	(synopsis "ScpScraper: Scraper utility for the SCP-Wiki")
	(description
		"Scraper Utility for the SCP-Wiki that collects and packages all information for importation using the ScpArchive program")
	(home-page "https://github.com/danielbatterystapler/SCP_Conversion_Project")
	(license license:gpl3))

