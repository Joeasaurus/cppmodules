# cppmodules
A module system in C++ that loads modules dynamically at run time.

## Building
* libboost (1.57.0)
    * *OSX*- Install with macports: `sudo port install boost`
    * *Windows*- Run the bootstrap script and then run `bjam.exe` with the following args: `-a --with-filesystem runtime-link=static --toolset=msvc-14.0 address-model=64 --build-type=complete -j 4 install`
    I also had to follow this: http://stackoverflow.com/questions/30760889/unknown-compiler-version-while-compiling-boost-with-msvc-14-0-vs-2015
* libzmq
    * *OSX*- I installed the required binaries via macports
    * *Linux*- On Manjaro, I used pacman (just try your package manager!)
    * *Windows*- I downloaded binaries from teh webs
* cppmodules
    * *OSX*/*Linux*- Generate the makefile with `cmake -G "Unix Makefiles"` and then run `make`
    * *Windows*- Build the solution linking to the directories you installed the dependencies in.