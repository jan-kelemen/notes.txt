# vim

## Compiling vim on Windows with Python3 support
- From [vim](https://github.com/vim/vim) repo in src folder, run:
  - `msvc2022.bat x64`
  - `nmake -f Make_mvc.mak FEATURES=huge GUI=yes PYTHON3="C:\Program Files\Python312" DYNAMIC_LINKING=YES PYTHON3_VER=312`
- See also: [INSTALLpc.txt](https://github.com/vim/vim/blob/master/src/INSTALLpc.txt)

## Add `Edit with Vim` to context menu
```
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\*\Shell\Vim]
@="Edit with &Vim"
"Icon"="\"C:\\vim\\vim91\\gvim.exe\""

[HKEY_CLASSES_ROOT\*\Shell\Vim\command]
@="\"C:\\vim\\vim91\\gvim.exe\" \"%1\""
```

## Add vim as an external tool to Visual Studio
- Add vim.exe to Tools > External Tools > Add
  - Arguments `--servername gVimStudio --remote-silent +"execute 'normal! $(CurLine)G$(CurCol)|'" +"simalt ~x" "$(ItemPath)"`
    - This uses a single instance of vim, positions the cursor to the same place as in VS, and opens it full screen
