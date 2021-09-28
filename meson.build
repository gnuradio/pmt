project('pmt', 'cpp', 
  version : '0.0.0',
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
flatc = find_program('flatc', version : '>=1.12')
gtest_dep = dependency('gtest', main : true, version : '>=1.10', required : get_option('enable_testing'))
# Boost is only used for program_options: FIXME - use something else
boost_dep = dependency('boost', modules : ['program_options'], version : '>=1.65')

subdir('include/pmtf')
subdir('lib')
subdir('test')
subdir('bench')