# cppmodules
A module system in C++ that loads modules dynamically at run time.

## Building
* boost
	* *OSX*: Install with macports `sudo port install boost`
    * *Windows*: Run the bootstrap script and then run `bjam.exe` with the following args: `-a --with-filesystem runtime-link=static --toolset=msvc-12.0 address-model=64 --build-type=complete install`
* cppmodules
    * *OSX*/*Linux*: Generate the makefile with `cmake -G "Unix Makefiles"` and then run `make`
    * *Windows*: Build the solution linking to the directories you installed the dependencies in.