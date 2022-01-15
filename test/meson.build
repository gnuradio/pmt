

# test_pmtf_sources = ['test_pmtf.cpp']
# executable('test_pmtf', test_pmtf_sources, include_directories : incdir, dependencies: [pmtf_dep], install : false)

test_pmtf_sources = ['test_pmtf.cpp']
executable('test_pmtf', test_pmtf_sources, include_directories : incdir, dependencies: [pmtf_dep], install : false)

# executable('test_pmt', test_pmt_sources, include_directories : incdir, install : true)



if get_option('enable_testing')
    srcs = ['qa_pmt.cpp']
    e = executable('qa_pmt', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT Basic Tests', e)

    srcs = ['qa_scalar.cpp']
    e = executable('qa_pmt_scalar', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT Scalar Tests', e)

    srcs = ['qa_vector.cpp']
    e = executable('qa_pmt_vector', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT Vector Tests', e)

    srcs = ['qa_vector_pmt.cpp']
    e = executable('qa_pmt_vector_pmts', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT Vector Of Pmts Tests', e)

    srcs = ['qa_string.cpp']
    e = executable('qa_string', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT String Tests', e)

    srcs = ['qa_map.cpp']
    e = executable('qa_pmt_map', 
        srcs, 
        include_directories : incdir, 
        dependencies: [pmtf_dep,
                    gtest_dep], 
        install : true)
    test('PMT Map Tests', e)
endif
