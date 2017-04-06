# - Find the PGMPI library
#
# This module sets the following variables:
#  PGMPI_FOUND - set to true if both PGMPI libraries are found
#  PGMPI_CLI_LIBRARIES   - full path to the CLI library
#  PGMPI_TUNED_LIBRARIES   - full path to the TUNED library
#  PGMPI_INCLUDE_DIR - directory where the PGMPI header files are
#
##########



set(PGMPI_TUNED_LIBRARY_NAME "pgmpituned")
set(PGMPI_CLI_LIBRARY_NAME "pgmpicli")

#find_path (PGMPI_INCLUDE_DIR pgmpi_tune.h
#              PATHS ${PGMPI_LIBRARY_DEFAULT_PATH}
#              PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
#              PATH_SUFFIXES include)

find_library (PGMPI_TUNED_LIBRARY ${PGMPI_TUNED_LIBRARY_NAME}
              PATHS ${PGMPI_LIBRARY_DEFAULT_PATH}
              PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
              PATH_SUFFIXES lib)

find_library (PGMPI_CLI_LIBRARY ${PGMPI_CLI_LIBRARY_NAME}
              PATHS ${PGMPI_LIBRARY_DEFAULT_PATH}
              PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
              PATH_SUFFIXES lib)

if (PGMPI_TUNED_LIBRARY)
    message(STATUS "PGMPI tuned library path: ${PGMPI_TUNED_LIBRARY}" )       
else(PGMPI_TUNED_LIBRARY)
    message(STATUS "PGMPI tuned library path: not found" )
endif()


if (PGMPI_CLI_LIBRARY)
    message(STATUS "PGMPI cli library path: ${PGMPI_CLI_LIBRARY}" )    
else(PGMPI_CLI_LIBRARY)
    message(STATUS "PGMPI cli library path: not found" )
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PGMPI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(PGMPI DEFAULT_MSG
                                  PGMPI_TUNED_LIBRARY PGMPI_CLI_LIBRARY)

mark_as_advanced(PGMPI_INCLUDE_DIR PGMPI_TUNED_LIBRARY  PGMPI_CLI_LIBRARY)
set(PGMPI_INCLUDE_DIRS ${PGMPI_INCLUDE_DIR} )
set(PGMPI_TUNED_LIBRARIES ${PGMPI_TUNED_LIBRARY} )
set(PGMPI_CLI_LIBRARIES ${PGMPI_CLI_LIBRARY} )

