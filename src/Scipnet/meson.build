project('Scipnet', 'cpp')

magick_config = run_command('Magick++-config', '--cppflags', '--cxxflags')
magick_arguments = magick_config.stdout().strip().split()
add_project_arguments(magick_arguments, language: 'cpp')

magick_link_config = run_command('Magick++-config', '--ldflags', '--libs')
magick_link_arguments = magick_link_config.stdout().strip().split()
add_project_link_arguments(magick_link_arguments, language: 'cpp')

cpp = meson.get_compiler('cpp')

dep = [
	dependency('ncurses'),
	cpp.find_library('ssl'),
	cpp.find_library('crypto'),
	cpp.find_library('curlpp'),
	cpp.find_library('curl')
]

inc = [
	'.'
]

src = [
	'Scipnet/main.cpp',
	'Scipnet/Cli.cpp',
	'Scipnet/ColorTable.cpp',
	'Scipnet/Converter.cpp',
	'Scipnet/Database.cpp',
	'Scipnet/Ncurses.cpp',
	'Scipnet/layout/Layout.cpp'
]

executable(
	'Scipnet',
	sources : src,
	dependencies : dep,
	include_directories : inc,
	install : true,
	override_options : ['cpp_std=c++17']
)

