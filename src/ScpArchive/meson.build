project('ScpArchive', 'cpp')

cpp = meson.get_compiler('cpp')

dep = [
	dependency('boost', modules : ['filesystem']),
	dependency('nlohmann_json'),
	cpp.find_library('cgicc'),
	dependency('fcgi'),
	cpp.find_library('fcgi++'),
	cpp.find_library('soci_core'),
	cpp.find_library('soci_mysql'),
	dependency('mariadb')
]

inc = [
	'.'
]

src = [
	'ScpArchive/main.cpp',
	'ScpArchive/Config.cpp',
	'ScpArchive/Database/Batch.cpp',
	'ScpArchive/Database/Database.cpp',
	'ScpArchive/Database/Importer.cpp',
	'ScpArchive/Database/Json.cpp',
	'ScpArchive/HTTP/entities/entities.cpp',
	'ScpArchive/HTTP/HtmlEntity.cpp',
	'ScpArchive/HTTP/MarkupOutStream.cpp',
	'ScpArchive/Parser/DatabaseUtil.cpp',
	'ScpArchive/Parser/HTMLConverter.cpp',
	'ScpArchive/Parser/Parser.cpp',
	'ScpArchive/Parser/Rules/RuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/AlignRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/AnchorRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/ARuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/BasicTextRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/CenterTextRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/CodeRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/CollapsibleRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/CommentRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/CSSRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/DividerRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/DivRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/EntityEscapeRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/FootNoteRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/ForumRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/HeadingRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/HTMLRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/HyperLinkRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/IFrameRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/IftagsRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/ImageRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/IncludeRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/InlineFormatRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/ListPagesRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/ListRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/LiteralTextRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/NullRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/QuoteBoxRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/RateRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/RedirectRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/SCPConversionProjectInfoBoxRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/SectionRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/SizeRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/SpanRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/TableOfContentsRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/TableRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/TabViewRuleSet.cpp',
	'ScpArchive/Parser/Rules/RuleSets/UserRuleSet.cpp',
	'ScpArchive/Parser/Templater.cpp',
	'ScpArchive/Parser/Treer.cpp',
	'ScpArchive/PDF/PDFConverter.cpp',
	'ScpArchive/Tests/AutomatedTester.cpp',
	'ScpArchive/Tests/Database/DatabaseTests.cpp',
	'ScpArchive/Tests/Database/ImporterTests.cpp',
	'ScpArchive/Tests/Database/JsonTests.cpp',
	'ScpArchive/Tests/HTTP/HtmlEntityTests.cpp',
	'ScpArchive/Tests/HTTP/MarkupOutStreamTests.cpp',
	'ScpArchive/Tests/Parser/HTMLConverterTests.cpp',
	'ScpArchive/Tests/Parser/ParserTests.cpp',
	'ScpArchive/Tests/Parser/TemplaterTests.cpp',
	'ScpArchive/Tests/Parser/TreerTests.cpp',
	'ScpArchive/Tests/Tests.cpp',
	'ScpArchive/UntestedUtils/UntestedUtils.cpp',
	'ScpArchive/Website/Gateway.cpp',
	'ScpArchive/Website/Website.cpp'
]

exe = executable(
	'ScpArchive',
	sources : src,
	dependencies : dep,
	include_directories : inc,
	install : true,
	override_options : ['cpp_std=c++17']
)

#test('ScpArchive Automated Test', exe, args : ['--runTests'])

