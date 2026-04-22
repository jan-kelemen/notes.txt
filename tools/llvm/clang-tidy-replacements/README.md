# clang-tidy replacements

Build: 
```
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/lib/llvm-22 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_SHARED_LIBS=ON -Wno-dev --fresh
cmake --build build
```

Run:
```
clang-tidy-22 --load ./libBoostStringViewTidyModule.so --checks=custom-to-string ../test/test_aliased.cpp
```

Result:
```
../test/test_aliased.cpp:11:5: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   11 |     sv.to_string();
      |     ^~~~~~~~~~~~~~
      |     std::string{ sv }
../test/test_aliased.cpp:13:5: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   13 |     temporary().to_string();
      |     ^~~~~~~~~~~~~~~~~~~~~~~
      |     std::string{ temporary() }
../test/test_aliased.cpp:15:13: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   15 |     auto a{ sv.to_string() };
      |             ^~~~~~~~~~~~~~
      |             std::string{ sv }
../test/test_aliased.cpp:16:12: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   16 |     auto b{temporary().to_string()};
      |            ^~~~~~~~~~~~~~~~~~~~~~~
      |            std::string{ temporary() }
../test/test_aliased.cpp:20:12: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   20 |     auto c{sv_ptr->to_string()};
      |            ^~~~~~~~~~~~~~~~~~~
      |            std::string{ *sv_ptr }
../test/test_aliased.cpp:22:13: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   22 |     auto d {(*sv_ptr).to_string()};
      |             ^~~~~~~~~~~~~~~~~~~~~
      |             std::string{ *sv_ptr }
../test/test_aliased.cpp:24:13: warning: replace boost::string_view::to_string() with std::string{...} [custom-to-string]
   24 |     auto e{ sv_ptr->substr(2).to_string()};
      |             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      |             std::string{ sv_ptr->substr(2) }
```
