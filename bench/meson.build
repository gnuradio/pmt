incdir = include_directories('../include')

srcs = ['bm_pmt_serialize_uvec',
        'bm_pmt_dict_ref',
        'bm_pmt_dict_pack_unpack']

deps = [pmt_dep, CLI11_dep, fmt_dep]

if (CLI11_dep.found())
foreach s : srcs
    e = executable(s, 
        s + '.cpp', 
        include_directories : incdir, 
        dependencies: deps, 
        install : false)
    # test(s, e)
endforeach
endif
