# fe56OS

## Install
### Install llvm
```
sudo apt install llvm clang

# llvm version: 14
# clang -v             
Ubuntu clang version 14.0.0-1ubuntu1
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/bin
Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/11
Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/11
Candidate multilib: .;@m64
Selected multilib: .;@m64
```
### Install edk2
```
git clone --recursive https://github.com/tianocore/edk2.git edk2
git checkout --recurse-submodules e1999b264f1f9d7230edf2448f757c73da567832
make -C edk2/BaseTools/Source/C
```
