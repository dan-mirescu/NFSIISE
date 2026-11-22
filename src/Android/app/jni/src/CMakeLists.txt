cmake_minimum_required(VERSION 3.22)

project(nfs2se C CXX)

# <repo>/src
get_filename_component(NFS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

if(NOT TARGET SDL3::SDL3)
  find_package(SDL3 CONFIG REQUIRED)
endif()

set(CMAKE_C_STANDARD   11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# The game itself.  Loaded into the low 4 GB by the libmain.so stub below,
# never with System.loadLibrary (see Loader.c for the reason).
# ----------------------------------------------------------------------------
add_library(nfs2se SHARED
  ${NFS_SRC_DIR}/Cpp/NFS2SE.cpp          # unity build of Memory/Methods_*/Entry
  ${NFS_SRC_DIR}/Cpp/MemoryTranslate.cpp # not part of the unity build
  ${NFS_SRC_DIR}/DInput.c
  ${NFS_SRC_DIR}/EAcsnd.c
  ${NFS_SRC_DIR}/FetchTrackRecords.c
  ${NFS_SRC_DIR}/Glide2x.c
  ${NFS_SRC_DIR}/Kernel32.c
  ${NFS_SRC_DIR}/Timer.c
  ${NFS_SRC_DIR}/User32.c
  ${NFS_SRC_DIR}/WinMM.c
  ${NFS_SRC_DIR}/Wrapper.c
  ${NFS_SRC_DIR}/Wsock32.c
)

target_include_directories(nfs2se PRIVATE ${NFS_SRC_DIR})
target_compile_definitions(nfs2se PRIVATE NFS_CPP GLES2 STACK_REALIGN)

target_compile_options(nfs2se PRIVATE
  -Wall -Wextra
  -Wno-int-to-pointer-cast
  -Wno-narrowing
  -Wno-unused-parameter
  -Wno-unused-variable
  -Wno-unused-but-set-variable
  -Wno-sign-compare
  -Wno-implicit-fallthrough
  -Wno-address-of-packed-member
  -Wno-missing-braces
  -fno-strict-aliasing
  $<$<COMPILE_LANGUAGE:C>:-Wno-pointer-to-int-cast>
  $<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>
  $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>
  $<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>
  $<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-function-pointer-types>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-c++11-narrowing>
)

target_link_libraries(nfs2se PRIVATE SDL3::SDL3 GLESv2 EGL log android)

# ----------------------------------------------------------------------------
# libmain.so - the shared object SDL loads and looks SDL_main up in.  It only
# maps libnfs2se.so into the low 4 GB and jumps into it.
# ----------------------------------------------------------------------------
add_library(main SHARED Loader.c)
target_compile_options(main PRIVATE -Wall -Wextra)
target_link_libraries(main PRIVATE log)
add_dependencies(main nfs2se)
