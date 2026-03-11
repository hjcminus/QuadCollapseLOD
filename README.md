# Build

## Build on Windows

Enter make directory and execute generate_vs2019.bat or generate_vs2022.bat <br>
to generate the sln file located in relative sub directory. <br>
then use msvc to build the project.

## Build on Ubuntu

Might need to install those packages: <br>
$ sudo apt install gcc-multilib g++-multilib <br>

Enter make directory and execute generate_gmake2.sh shell to generate the Makefile <br>
which located in gmake2 sub directory. <br>
then enter gmake2 sub directory and execute: <br>
$ make config=release_x64 <br>
This will generate the binary executable file located in bin/gmake2/x64/release directory.
