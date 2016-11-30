
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

SET(CMAKE_C_COMPILER mpixlc)
SET(CMAKE_CXX_COMPILER mpixlcxx)
SET(CMAKE_C_FLAGS "-O3 -qarch=qp -qtune=qp")

SET(BUILD_SHARED_LIBS 0)

message(STATUS "Using the IBM compiler (BG/Q): ${CMAKE_C_COMPILER}")