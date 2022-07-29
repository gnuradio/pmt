

TEST_ENV = environment()
TEST_ENV.prepend('LD_LIBRARY_PATH', 
    join_paths( meson.build_root(),'lib'),
)
TEST_ENV.prepend('PYTHONPATH', join_paths(meson.build_root(),'python') )


test_pmtf_sources = ['test_pmtf.cpp']
executable('test_pmtf', test_pmtf_sources, include_directories : incdir, dependencies: [pmtf_dep], install : false)


# GR namespace tests
qa_srcs = ['qa_pmt',
           'qa_scalar',
           'qa_map',
           'qa_null',
           'qa_string',
           'qa_tag',
           'qa_vector_pmt',
           'qa_vector'
          ]
deps = [pmtf_dep,
                gtest_dep]

foreach qa : qa_srcs
    e = executable(qa, 
        qa + '.cpp', 
        include_directories : incdir, 
        link_language : 'cpp',
        dependencies: deps, 
        install : false)
    test(qa, e)
endforeach

test('Numpy Vector', py3, args : files('qa_vector_numpy.py'), env: TEST_ENV)
