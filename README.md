# PMT Library

https://wiki.gnuradio.org/index.php/Polymorphic_Types_(PMTs)

PMT Objects represent a serializable container of data that can be passed across various interfaced.  In GNU Radio, these are used to carry data on stream tags or through message passing, and could be used to pass data to external consumers of GNU Radio.  This PMT library restructures the API beyond what has previously been included in the GNU Radio codebase to more closely align with the functionality and usability in the C++ Standard Template Library, and uses std::variant for the underlying data storage and serialization 

## Dependencies

- meson
- ninja
- C++20

## Installation

PMTlib uses meson and ninja to manage the build process, which can be installed via pip and your package manager

```bash
pip install meson
cd pmt
meson setup build --buildtype=debugoptimized --prefix=[PREFIX] --libdir=lib
cd build
ninja
ninja install
```


## Build with emscripten
The path to `emsdk` must be provided. It can be done in 2 different ways.

### Option 1
Create file `emscripten-toolchain.ini` in the root directory with the following content:
```text
[constants]
toolchain = '/path/to/emsdk/upstream/emscripten/'
```

And then execute the following command:
```bash
meson setup build_wasm --cross-file emscripten-toolchain.ini --cross-file emscripten-build.ini -Denable_python=false -Denable_testing=true
```

### Option 2
Set `toolchain` constant directly in `emscripten-build.ini` file in `[constants]` section:
```text
[constants]  
...
toolchain = '/home/slebedev/_Soft/emsdk/upstream/emscripten/'
```
And then execute the following command:
```bash
meson setup build_wasm --cross-file emscripten-build.ini -Denable_python=false -Denable_testing=true
```


The next steps are:
```bash
cd build_wasm
ninja
ninja test
```

### Potential problems
If error `Dependency gtest found: NO` occurred, do the following command:
```bash
cd [pmt_project_root_dir]
meson wrap install gtest
```

It creates `subprojects/gtest.wrap` wrap file for `gtest`.
