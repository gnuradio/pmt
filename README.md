# PMT Library

https://wiki.gnuradio.org/index.php/Polymorphic_Types_(PMTs)

PMT Objects represent a serializable container of data that can be passed across various interfaced.  In GNU Radio, these are used to carry data on stream tags or through message passing, and could be used to pass data to external consumers of GNU Radio.  This PMT library restructures the API beyond what has previously been included in the GNU Radio codebase to more closely align with the functionality and usability in the C++ Standard Template Library, and uses Flatbuffers for the underlying data storage and serialization 

## Dependencies

- meson
- ninja
- flatbuffers (>=2.0.0)
- C++17

## Installation

PMTlib uses meson and ninja to manage the build process, which can be installed via pip and your package manager

```bash
pip install meson
cd newsched
meson setup build --buildtype=debugoptimized --prefix=[PREFIX] --libdir=lib
cd build
ninja
ninja install
```
