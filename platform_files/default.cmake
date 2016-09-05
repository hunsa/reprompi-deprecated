
# default configuration 
find_package(MPI REQUIRED)

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
SET(CMAKE_C_FLAGS "-O3 -Wall ") # -pedantic")

SET(BUILD_SHARED_LIBS 1)

message(STATUS "Using default compiler: ${CMAKE_C_COMPILER}")