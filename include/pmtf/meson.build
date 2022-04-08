

pmt_gen_h = custom_target('flatc-output',
                        input : ['pmtf.fbs'],
                        output : ['pmtf_generated.h'],
                        install : true,
                        install_dir : 'include/pmtf',
                        command : [flatc, '-c', '--reflect-types','--gen-mutable','--reflect-names','--cpp-std','c++17', '-o', '@OUTDIR@', '@INPUT@'])

pmt_gen_h_dep = declare_dependency(sources : [pmt_gen_h])

files = [
    'alloc.hpp',
    'base.hpp',
    'scalar.hpp',
    'vector.hpp',
    'string.hpp',
    'map.hpp',
    'tag.hpp',
    'wrap.hpp',
    'type_helpers.hpp'
]
install_headers(files, subdir : 'pmtf')
