# Fugaku
# using Fujitsu cross-compiler on login node

find_package(MPI REQUIRED)

SET(CMAKE_C_COMPILER mpifccpx)
SET(CMAKE_CXX_COMPILER mpiFCCpx)
SET(CMAKE_C_FLAGS "-Nclang -Ofast -Wall ")

SET(BUILD_SHARED_LIBS 1)

message(STATUS "Using the Fujitsu cross compiler: ${CMAKE_C_COMPILER}")
