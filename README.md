# cppmodules
A module system in C++ that loads modules dynamically at run time.

## Building
* boost
    * *Windows*: bjam.exe -a --with-filesystem runtime-link=static --toolset=msvc-12.0 address-model=64 --build-type=complete install
* cppmodules
    * *OSX*: Run `make macosx modules=yes`
    * *Windows*: Build the solution linking to the directories you installed the dependencies in. 