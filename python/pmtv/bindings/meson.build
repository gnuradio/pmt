pmtv_pybind_sources = files([
    'pmt_python.cc',
    'pmtv_python.cc'
 ] )


numpy_include = include_directories(
  run_command(
    py3_inst, '-c', 'import numpy; print(numpy.get_include())',
    check: true
  ).stdout().strip()
)

pmtv_pybind_deps = [python3_dep, pybind11_dep, pmt_dep]
pmtv_pybind = py3_inst.extension_module('pmtv_python',
    pmtv_pybind_sources,
    include_directories: [numpy_include, '../../../include'],
    dependencies : pmtv_pybind_deps,
    link_language : 'cpp',
    install : true,
    install_dir : join_paths(py3_inst.get_install_dir(),'pmtv')
)

pmtv_pybind_dep = declare_dependency(
					   link_with : pmtv_pybind)
