project('pmt', 'cpp',
  version : '0.0.2',
  meson_version: '>=0.63.0',
  license : 'GPLv3',
  default_options : ['cpp_std=c++23', 'warning_level=3'])

cc = meson.get_compiler('cpp')
warnings_as_errors = get_option('warnings_as_errors') # Define this option in your meson_options.txt

msvc_warnings = [
    '/W4',  # Baseline reasonable warnings
    '/w14242',  # 'identifier': conversion from 'type1' to 'type1', possible loss of data
    '/w14254',  # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
    '/w14263',  # 'function': member function does not override any base class virtual member function
    '/w14265',  # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
    '/w14287',  # 'operator': unsigned/negative constant mismatch
    '/we4289',  # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
    '/w14296',  # 'operator': expression is always 'boolean_value'
    '/w14311',  # 'variable': pointer truncation from 'type1' to 'type2'
    '/w14545',  # expression before comma evaluates to a function which is missing an argument list
    '/w14546',  # function call before comma missing argument list
    '/w14547',  # 'operator': operator before comma has no effect; expected operator with side-effect
    '/w14549',  # 'operator': operator before comma has no effect; did you intend 'operator'?
    '/w14555',  # expression has no effect; expected expression with side- effect
    '/w14619',  # pragma warning: there is no warning number 'number'
    '/w14640',  # Enable warning on thread un-safe static member initialization
    '/w14826',  # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
    '/w14905',  # wide string literal cast to 'LPSTR'
    '/w14906',  # string literal cast to 'LPWSTR'
    '/w14928',  # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    '/permissive-',  # standards conformance mode for MSVC compiler.
]

clang_warnings = [
    '-Wall',  # reasonable and standard
    '-Wextra',  # extra warnings
    '-Wshadow',  # warn the user if a variable declaration shadows one from a parent context
    '-Wnon-virtual-dtor',  # warn if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
    '-Wold-style-cast',  # warn for c-style casts
    '-Wcast-align',  # warn for potential performance problem casts
    '-Wunused',  # warn on anything being unused
    '-Woverloaded-virtual',  # warn if you overload (not override) a virtual function
    '-Wpedantic',  # warn if non-standard C++ is used
    '-Wconversion',  # warn on type conversions that may lose data
    '-Wsign-conversion',  # warn on sign conversions
    '-Wnull-dereference',  # warn if a null dereference is detected
    '-Wdouble-promotion',  # warn if float is implicit promoted to double
    '-Wformat=2',  # warn on security issues around functions that format output (ie printf)
    '-Wno-unknown-pragmas',  # ignore IDE, GCC/CLANG specific pragmas
    '-Wimplicit-fallthrough',  # Warns when case statements fall-through.
]

gcc_warnings = clang_warnings + [
    '-Wmisleading-indentation',  # warn if indentation implies blocks where blocks do not exist
    '-Wduplicated-cond',  # warn if if / else chain has duplicated conditions
    '-Wduplicated-branches',  # warn if if / else branches have duplicated code
    '-Wlogical-op',  # warn about logical operations being used where bitwise were probably wanted
    '-Wuseless-cast',  # warn if you perform a cast to the same type
    '-Wno-interference-size',  # suppress ABI compatibility warnings for hardware inferred size
    '-Wno-maybe-uninitialized',  # false positives if asan is enabled: https://gcc.gnu.org/bugzilla//show_bug.cgi?id=1056h6
    '-fconcepts-diagnostics-depth=3',  # for deeper diagnostics on concept-related errors
]

if cc.get_id() == 'msvc'
    if warnings_as_errors
        msvc_warnings += ['/WX']
    endif
    add_project_arguments(msvc_warnings, language : 'cpp')
elif cc.get_id() == 'clang'
    if warnings_as_errors
        clang_warnings += ['-Werror']
    endif
    add_project_arguments(clang_warnings, language : 'cpp')
elif cc.get_id() == 'gcc'
    if warnings_as_errors
        gcc_warnings += ['-Werror']
    endif
    add_project_arguments(gcc_warnings, language : 'cpp')
endif

rt_dep = cc.find_library('rt', required : false)
c_available = add_languages('c', required : true)

if (get_option('enable_python'))
# Import python3 meson module which is used to find the Python dependencies.
py3 = import('python').find_installation()
python3_dep = py3.dependency(required : true)
pybind11_dep = dependency('pybind11', required : true)

# For pybind11, if version < 2.4.4 then we need to add -fsized-deallocation flag
if pybind11_dep.found() and meson.get_compiler('cpp').get_id() == 'clang'
  if pybind11_dep.version().version_compare('<2.4.4')
     add_global_arguments('-fsized-deallocation', language: 'cpp')
  endif
endif
endif

gtest_dep = dependency('gtest', main : true, version : '>=1.10', required : get_option('enable_testing'))
gtest_main_dep = dependency('gtest_main', version : '>=1.10', required : get_option('enable_testing'))
CLI11_dep = dependency('CLI11', fallback : [ 'cli11' , 'CLI11_dep' ])
fmt_dep = dependency('fmt', version:'>=10.0.0')

cmake = import('cmake')

# Configure the CMake project
refl_cpp = cmake.subproject('refl-cpp')

# Fetch the dependency object
refl_cpp_dep = refl_cpp.dependency('refl-cpp')

incdir = include_directories('include')
pmt_dep = declare_dependency(include_directories : incdir, dependencies : refl_cpp_dep)

subdir('include/pmtv')
if get_option('enable_testing')
  subdir('test')
endif
subdir('bench')
if (get_option('enable_python'))
subdir('python/pmtv')
endif

pkg = import('pkgconfig')
# libs = [pmt_lib]     # the library/libraries users need to link against
h = ['.'] # subdirectories of ${prefix}/${includedir} to add to header path
pkg.generate(
             subdirs : h,
             version : meson.project_version(),
             name : 'libpmt',
             filebase : 'pmt',
             install_dir : 'lib/pkgconfig',
             description : 'PMT - Polymorphic Types')
