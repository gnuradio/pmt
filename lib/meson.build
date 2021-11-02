# pmt_sources = ['pmt.cpp']

incdir = include_directories('../include')


pmtf_sources = ['base.cpp', 'pmtf_scalar.cpp','pmtf_vector.cpp','string.cpp', 'map.cpp', 'wrap.cpp']
pmtf_lib = library('pmtf', pmtf_sources, include_directories : incdir, install : true, dependencies : [pmt_gen_h_dep])

pmtf_dep = declare_dependency(include_directories : incdir,
					   link_with : pmtf_lib)

# cmake = import('cmake')
# prefix = get_option('prefix')
# # Escape spaces
# prefix_parts = prefix.split(' ')
# prefix = '\ '.join(prefix_parts)

# conf = configuration_data()
# conf.set('prefix', prefix)
# conf.set('exec_prefix', '${prefix}')
# conf.set('libdir', join_paths('${prefix}',get_option('libdir')))
# conf.set('includedir', join_paths('${prefix}',get_option('includedir')))
# conf.set('LIBVER', meson.project_version())

# cmake_conf = configuration_data()
# cmake_conf.set('libdir', join_paths(prefix,get_option('libdir')))
# cmake.configure_package_config_file(
#   name : 'pmtf',
#   input : join_paths(meson.source_root(),'cmake','Modules','pmtfConfig.cmake.in'),
#   install_dir : get_option('prefix') / 'lib' / 'cmake' / 'pmtf',
#   configuration : cmake_conf
# )

# pkg = import('pkgconfig')
# libs = []     # the library/libraries users need to link against
# h = ['.'] # subdirectories of ${prefix}/${includedir} to add to header path
# pkg.generate(libraries : libs,
#              subdirs : h,
#              version : meson.project_version(),
#              name : 'libpmtf',
#              filebase : 'pmtf',
#              install_dir : get_option('prefix') / 'lib' / 'pkgconfig',
#              description : 'PMTF - Polymorphic Types with Flatbuffers')

