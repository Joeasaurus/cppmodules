# cppmodules
A module system in C++ that loads modules dynamically at run time.

## Building

Requirements:
* CMake  (>= 2.8)
* Visual Studio 12.0 (2013)
* Boost  (>= 1.58)
* libzmq (>= 3.2)

### Windows
1. Install Boost. I do this by compiling the source manually as we need binaries for boost_system and boost_filesystem.
    * Grab the zip from [here](http://www.boost.org/users/history/version_1_60_0.html)
    * Unzip and open the dir in a command line. Run `.\bootstrap.bat`
    * Run `bjam.exe -a --with-filesystem runtime-link=static --toolset=msvc-12.0 address-model=64 --build-type=complete -j 4 install`
2. Install [libzmq](http://zeromq.org/distro:microsoft-windows)
3. Install [CMake](https://cmake.org/download/)
4. Next you need to compile `dlfcn-win32` in Release mode. Do this by opening the solution from it's directory in `submodules`, changing the target and hitting "Build".
5. Now you should be ready to generate the `cppmodules` solution and build it! Run `cmake . -G "Visual Studio 12 2013 Win64"`
6. Open the solution and click "Build All"
7. You should find `cppmodules.exe` and a `modules` directory under `build`.
