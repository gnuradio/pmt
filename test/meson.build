

TEST_ENV = environment()
TEST_ENV.prepend('LD_LIBRARY_PATH', 
    join_paths( meson.project_build_root(),'lib'),
)
TEST_ENV.prepend('PYTHONPATH', join_paths(meson.project_build_root(),'python') )

# GR namespace tests
qa_srcs = ['qa_scalar',
           'qa_uniform_vector',
           'qa_vector_of_pmts',
           'qa_map',
           'qa_string',
           'qa_reflection'
          ]
deps = [pmt_dep, 
            fmt_dep,
            gtest_dep,
            gtest_main_dep]

foreach qa : qa_srcs
    e = executable(qa, 
        qa + '.cpp', 
        include_directories : incdir, 
        link_language : 'cpp',
        dependencies: deps, 
        install : false)
    test(qa, e)
endforeach

if (get_option('enable_python'))
test('Python Bindings', py3, args : files('qa_pybind.py'), env: TEST_ENV)
endif

