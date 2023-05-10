project('pmt', 'cpp',
  version : '0.0.2',
  meson_version: '>=0.63.0',
  license : 'GPLv3',
  default_options : ['cpp_std=c++20'])

cc = meson.get_compiler('cpp')
rt_dep = cc.find_library('rt', required : false)
c_available = add_languages('c', required : true)

if (get_option('enable_python'))
python3_dep = dependency('python3', required : true)
# Import python3 meson module which is used to find the Python dependencies.
py3_inst = import('python').find_installation('python3')
py3 = py3_inst
pybind11_dep = dependency('pybind11', required : true)
# For pybind11, if version < 2.4.4 then we need to add -fsized-deallocation flag
if pybind11_dep.found() and meson.get_compiler('cpp').get_id() == 'clang'
  if pybind11_dep.version().version_compare('<2.4.4')
     add_global_arguments('-fsized-deallocation', language: 'cpp')
  endif
endif
endif

gtest_dep = dependency('gtest', main : true, version : '>=1.10', required : get_option('enable_testing'))
gtest_main_dep = dependency('gtest_main', version : '>=1.10', required : get_option('enable_testing'))
CLI11_dep = dependency('CLI11', fallback : [ 'cli11' , 'CLI11_dep' ])
fmt_dep = dependency('fmt', version:'>=8.1.1')

incdir = include_directories('include')
pmt_dep = declare_dependency(include_directories : incdir)

subdir('include/pmtv')
if get_option('enable_testing')
  subdir('test')
endif
subdir('bench')
if (get_option('enable_python'))
subdir('python/pmtv')
endif

pkg = import('pkgconfig')
# libs = [pmt_lib]     # the library/libraries users need to link against
h = ['.'] # subdirectories of ${prefix}/${includedir} to add to header path
pkg.generate(
             subdirs : h,
             version : meson.project_version(),
             name : 'libpmt',
             filebase : 'pmt',
             install_dir : 'lib/pkgconfig',
             description : 'PMT - Polymorphic Types')
