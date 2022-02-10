incdir = include_directories('../include')

if (CLI11_dep.found())
srcs = ['bm_pmt_serialize_uvec.cpp']
executable('bm_pmt_serialize_uvec', 
    srcs, 
    include_directories : incdir, 
    dependencies: [pmtf_dep,
                   CLI11_dep], 
    install : true)

srcs = ['bm_pmt_dict_ref.cpp']
executable('bm_pmt_dict_ref', 
    srcs, 
    include_directories : incdir, 
    dependencies: [pmtf_dep,
                   CLI11_dep], 
    install : true)

srcs = ['bm_pmt_dict_pack_unpack.cpp']
executable('bm_pmt_dict_pack_unpack', 
    srcs, 
    include_directories : incdir, 
    dependencies: [pmtf_dep,
                   CLI11_dep], 
    install : true)
endif