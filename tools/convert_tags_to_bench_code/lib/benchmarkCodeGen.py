import sys
import os
import shutil
import errno

from codeParser import CodeParser
from string import strip

def generate_cmake_text(src_dir, sourcefiles_list):
    binary_name = "reprompibench"
    
    common_cmake_text = \
"""
cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

project(gen_code_reproMPIbench)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(INCLUDE_PLATFORM_CONFIG_FILE "${CMAKE_SOURCE_DIR}/platform_files/default.cmake" 
    CACHE STRING "Configure project to use a specific platform file")
include(${INCLUDE_PLATFORM_CONFIG_FILE})

set(REPRO_MPI_BENCHMARK_DIR "" CACHE STRING "Path to the reproMPIbench directory")
if(NOT EXISTS ${REPRO_MPI_BENCHMARK_DIR})
    message(FATAL_ERROR "Please specify the path to the ReproMPI benchmark directory")
endif(NOT EXISTS ${REPRO_MPI_BENCHMARK_DIR})

find_package(GSL REQUIRED)

if (GSL_INCLUDE_DIR)
message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
else(GSL_INCLUDE_DIR)
message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)


set(REPRO_MPI_BENCHMARK_LIB ${REPRO_MPI_BENCHMARK_DIR}/lib)
set(REPRO_MPI_BENCHMARK_INCLUDE ${REPRO_MPI_BENCHMARK_DIR}/include)

SET(SRC_DIR %s)
add_executable(%s
%s
)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${REPRO_MPI_BENCHMARK_INCLUDE})
TARGET_LINK_LIBRARIES(%s ${REPRO_MPI_BENCHMARK_LIB}/libreproMPIbench.${LIBRARY_SUFFIX} ${MPI_LIBRARIES} ${GSL_LIBRARIES})


""" % (src_dir, 
       binary_name,
       "\n".join([ "${SRC_DIR}/%s" %f for f in sourcefiles_list]), 
       binary_name)

    return common_cmake_text



class BenchmarkCodeGen:

    def __init__(self, input_dir, output_dir, sources_file, cmake_helper_files_dir):
        self.input_dir = input_dir
        self.output_dir = os.path.join(output_dir, os.path.basename(input_dir))
        self.cmake_helper_files_dir = cmake_helper_files_dir
        self.output_basedir =  output_dir

        with open(sources_file, "r") as f:
            sfiles_list = f.readlines()
        sfiles_list = map(strip, sfiles_list)
        
        self.cfiles = []
        self.cheaders = []
        for dirpath, dirs, files in os.walk(self.input_dir):
            for f in files:
                rel_file_path = os.path.relpath(os.path.join(dirpath, f), self.input_dir)
                if f.endswith(".c") and rel_file_path in sfiles_list:
                    self.cfiles.append(rel_file_path)
                if f.endswith(".h") and rel_file_path in sfiles_list:
                    self.cheaders.append(rel_file_path)
        

    def generate_cmakelists(self):

        filepath = os.path.join(os.path.dirname(self.output_dir), "CMakeLists.txt")

        with open(filepath, "w") as f:
            f.write(generate_cmake_text(os.path.basename(self.output_dir), self.cfiles))


    def copy_platform_files(self):
        platform_files_dir = os.path.join(self.cmake_helper_files_dir, "platform_files")
        output_platform_dir = os.path.join(self.output_basedir,"platform_files")
        try:
            os.makedirs(output_platform_dir)
        except OSError as e:
            if e.errno == errno.EEXIST and os.path.isdir(output_platform_dir):
                pass
            else:
                print("Error: cannot create platform files directory in %s" % (output_platform_dir))
                print(e)
                exit(1)  
        try:
            for f in os.listdir(platform_files_dir):
                if os.path.isfile(os.path.join(platform_files_dir, f)):
                    shutil.copy(os.path.join(platform_files_dir, f), output_platform_dir)
        except Exception as e:
            print("Error: cannot copy platform files from %s to %s" % (platform_files_dir, self.output_basedir))
            print (e)
            exit(1)


    def copy_cmake_modules(self):
        cmake_modules_dir = os.path.join(self.cmake_helper_files_dir, "cmake_modules")
        output_cmake_dir = os.path.join(self.output_basedir,"cmake_modules")
        try:
            os.makedirs(output_cmake_dir)
        except OSError as e:
            if e.errno == errno.EEXIST and os.path.isdir(output_cmake_dir):
                pass
            else:
                print("Error: cannot create cmake module directory in %s" % (output_cmake_dir))
                print(e)
                exit(1)      
        try:
            for f in os.listdir(cmake_modules_dir):
                if os.path.isfile(os.path.join(cmake_modules_dir, f)):
                    shutil.copy(os.path.join(cmake_modules_dir, f), output_cmake_dir)
        except Exception as e:
            print("Error: cannot copy cmake module files from %s to %s" % (cmake_modules_dir, self.output_basedir))
            print(e)
            exit(1)          


    def copy_headers(self):
        for f in self.cheaders:
            final_file_dir = os.path.join(self.output_dir, os.path.dirname(f))
            try:
                os.makedirs(final_file_dir)
            except OSError as e:
                if e.errno == errno.EEXIST and os.path.isdir(final_file_dir):
                    pass
                else:
                    raise
            shutil.copy(os.path.join(self.input_dir, f), final_file_dir)


    def generate_benchmarking_code(self):

        self.generate_cmakelists()
        self.copy_headers()
        self.copy_platform_files()
        self.copy_cmake_modules()
        
        print "Parsing files..."
        for f in self.cfiles:
            print f
            final_file_dir = os.path.join(self.output_dir, os.path.dirname(f))
            parser = CodeParser(os.path.join(self.input_dir, f), final_file_dir)
            parser.parse_file()
            parser.generate_output_file()
        print "Done."






