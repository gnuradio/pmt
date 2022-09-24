pmtf_pybind_sources = files([
    'map_python.cc',
    'string_python.cc',
    'wrap_python.cc',
    'pmtf_python.cc'
 ] )


numpy_include = include_directories(
  run_command(
    py3, '-c', 'import numpy; print(numpy.get_include())',
    check: true
  ).stdout().strip()
)

pmtf_pybind_deps = [python3_dep, pybind11_dep, pmtf_dep]
pmtf_pybind = py3.extension_module('pmtf_python',
    pmtf_pybind_sources,
    include_directories: numpy_include,
    dependencies : pmtf_pybind_deps,
    link_language : 'cpp',
    install : true,
    install_dir : join_paths(py3.get_install_dir( pure : (get_option('libdir') == 'lib')) ,'pmtf')
)

pmtf_pybind_dep = declare_dependency(
					   link_with : pmtf_pybind)
