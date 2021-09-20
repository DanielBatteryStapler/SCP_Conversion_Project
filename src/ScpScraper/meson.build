project('ScpScraper', ['cpp', 'c'])

cpp = meson.get_compiler('cpp')

dep = [
	dependency('boost', modules : ['filesystem']),
	dependency('nlohmann_json'),
	dependency('threads'),
	cpp.find_library('ssl'),
	cpp.find_library('crypto'),
	cpp.find_library('curlpp'),
	cpp.find_library('curl')
]

inc = [
	'.'
]

src = [
	'ScpScraper/main.cpp',
	'ScpScraper/Scraper/Scraper.cpp',
	'ScpScraper/Scraper/Helpers.cpp',
	'ScpScraper/Scraper/entities/entities.c',
]

executable(
	'ScpScraper',
	sources : src,
	dependencies : dep,
	include_directories : inc,
	install : true,
	override_options : ['cpp_std=c++17']
)

