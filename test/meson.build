

TEST_ENV = environment()
TEST_ENV.prepend('LD_LIBRARY_PATH', 
    join_paths( meson.build_root(),'lib'),
)
TEST_ENV.prepend('PYTHONPATH', join_paths(meson.project_build_root(),'python') )

# GR namespace tests
qa_srcs = ['qa_scalar',
            'qa_vector',
          ]
deps = [pmt_dep,
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


# e = executable('test_cereal', 'test_cereal.cpp', dependencies: cereal_dep, include_directories : incdir)