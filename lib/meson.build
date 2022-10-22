incdir = include_directories('../include')


pmt_sources = [
	'base64/base64.c'
]
pmt_lib = library(
	'pmt', 
	pmt_sources, 
	include_directories : incdir, 
	link_language : 'cpp',
	install : true, 
	dependencies : []
)

pmt_dep = declare_dependency(include_directories : incdir,
					   link_with : pmt_lib)
