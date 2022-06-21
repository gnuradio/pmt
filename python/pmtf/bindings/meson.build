pmtf_pybind_sources = files([
    'map_python.cc',
    'string_python.cc',
    'wrap_python.cc',
    'pmtf_python.cc'
 ] )

pmtf_pybind_deps = [python3_dep, pybind11_dep, pmtf_dep]
pmtf_pybind = py3_inst.extension_module('pmtf_python',
    pmtf_pybind_sources, 
    dependencies : pmtf_pybind_deps,
    link_language : 'cpp',
    install : true,
    install_dir : join_paths(py3_inst.get_install_dir(),'pmtf')
)

pmtf_pybind_dep = declare_dependency(
					   link_with : pmtf_pybind)
