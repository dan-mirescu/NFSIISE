cmake_minimum_required(VERSION 3.22)

project(NFSIISE C CXX)

# SDL3 sources are expected in a subdirectory (or symlink) named "SDL"
add_subdirectory(SDL)

# Game library + loader stub
add_subdirectory(src)
