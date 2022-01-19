project('pmt', 'cpp', 
  version : '0.0.2',
  license : 'GPLv3',
  default_options : ['cpp_std=c++17'])

cc = meson.get_compiler('cpp')
rt_dep = cc.find_library('rt', required : false)
fmt_dep = dependency('fmt', method: 'cmake', modules: ['fmt::fmt'])
spdlog_dep = dependency('spdlog', method: 'cmake', modules : ['spdlog::spdlog'])
python3_dep = dependency('python3', required : get_option('enable_python'))
# Import python3 meson module which is used to find the Python dependencies.
py3_inst = import('python').find_installation('python3')
pybind11_dep = dependency('pybind11', required : get_option('enable_python'))
flatc = find_program('flatc', version : '>=2.0')
gtest_dep = dependency('gtest', main : true, version : '>=1.10', required : get_option('enable_testing'))

libCLI11 = subproject('CLI11')
CLI11_dep = libCLI11.get_variable('CLI11_dep')

subdir('include/pmtf')
subdir('lib')
subdir('test')
subdir('bench')
subdir('python/pmtf')

# cmake = import('cmake')

# conf = configuration_data()
# # conf.set_quoted('VAR', 'variable value')

# cmake.configure_package_config_file(
#     name: 'pmtf',
#     input: 'cmake/Modules/pmtf.cmake.in',
#     configuration: conf,
#     install_dir : 'lib/cmake/pmtf'
# )

# cmake.write_basic_package_version_file(name: 'pmtf', version: meson.project_version())

pkg = import('pkgconfig')
libs = [pmtf_lib]     # the library/libraries users need to link against
h = ['.'] # subdirectories of ${prefix}/${includedir} to add to header path
pkg.generate(libraries : libs,
             subdirs : h,
             version : meson.project_version(),
             name : 'libpmtf',
             filebase : 'pmtf',
             install_dir : 'lib/pkgconfig',
             description : 'PMTF - Polymorphic Types with Flatbuffers')