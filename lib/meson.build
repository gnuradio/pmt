incdir = include_directories('../include')


pmtf_sources = [
	'base.cpp', 
	'scalar.cpp',
	'vector.cpp',
	'string.cpp', 
	'map.cpp', 
	'wrap.cpp',
	'base64/base64.c'
]
pmtf_lib = library(
	'pmtf', 
	pmtf_sources, 
	include_directories : incdir, 
	link_language : 'cpp',
	install : true, 
	dependencies : [pmt_gen_h_dep]
)

pmtf_dep = declare_dependency(include_directories : incdir,
					   link_with : pmtf_lib,
					   dependencies : [pmt_gen_h_dep])
