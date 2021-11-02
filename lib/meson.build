# pmt_sources = ['pmt.cpp']

incdir = include_directories('../include')


pmtf_sources = ['base.cpp', 'pmtf_scalar.cpp','pmtf_vector.cpp','string.cpp', 'map.cpp', 'wrap.cpp']
pmtf_lib = library('pmtf', pmtf_sources, include_directories : incdir, install : true, dependencies : [pmt_gen_h_dep])

pmtf_dep = declare_dependency(include_directories : incdir,
					   link_with : pmtf_lib)
