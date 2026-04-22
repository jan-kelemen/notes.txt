cmake_minimum_required(VERSION 3.31)

project(BoostStringViewTidyModule)

find_package(LLVM 22.1.3 REQUIRED CONFIG)
find_package(Clang 22.1.3 REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${CLANG_CMAKE_DIR}")
include(AddLLVM)
include(AddClang)

add_clang_library(BoostStringViewTidyModule
  ToStringModule.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils
  clangASTMatchers
  clangBasic
  clangAST
)

target_include_directories(BoostStringViewTidyModule PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(BoostStringViewTidyModule PRIVATE
  ${LLVM_DEFINITIONS}
)
