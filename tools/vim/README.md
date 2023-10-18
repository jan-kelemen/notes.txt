# vim

## Compiling vim on Windows with Python3 support
- From [vim](https://github.com/vim/vim) repo in src folder, run:
  - `msvc2022.bat x64`
  - `nmake -f Make_mvc.mak FEATURES=huge GUI=yes PYTHON3="C:\Program Files\Python312" DYNAMIC_LINKING=YES PYTHON3_VER=312`
- See also: [INSTALLpc.txt](https://github.com/vim/vim/blob/master/src/INSTALLpc.txt)
