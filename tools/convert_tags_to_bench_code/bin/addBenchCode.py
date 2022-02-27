#! /usr/bin/env python

import sys
import os
import subprocess

# in bin
base_path = os.path.dirname( os.path.realpath( sys.argv[0] ) )
#cd ..
base_path = os.path.dirname( base_path )
lib_path = os.path.join( base_path, "lib" )
sys.path.append(lib_path)

from optparse import OptionParser
from benchmarkCodeGen import BenchmarkCodeGen


if __name__ == "__main__":

    parser = OptionParser(usage="usage: %prog [options]")

    parser.add_option("-d", "--expdir",
                       action="store",
                       dest="input_dir",
                       type="string",
                       help="directory of input files")
    parser.add_option("-l", "--sources_file",
                       action="store",
                       dest="sources_file",
                       type="string",
                       help="path to a file containing the list of source files (one file per line, with each path relative to the input directory)")
    parser.add_option("-o", "--outputdir",
                       action="store",
                       dest="output_dir",
                       type="string",
                       help="output directory")
    
    (options, args) = parser.parse_args()

    if options.input_dir == None:
        input_dir = os.path.abspath(base_path)
        print("Warning: Experiment directory not specified. Using current directory %s\n" %  base_path)
    else:
        input_dir = os.path.abspath(options.input_dir)
        
    if options.sources_file == None:
        sys.exit("List of source files not specified.\n")
    else:
        sources_file = os.path.abspath(options.sources_file)
        if not os.path.exists(sources_file):
            sys.exit("List of source files does not exist in %s.\n" % sources_file)
            
        
    if options.output_dir == None:
        sys.exit("Output directory not specified or does not exist.\n")
        
    if not (os.path.exists(options.output_dir) and os.path.isdir(options.output_dir)):
        print("Creating output directory: %s" % (options.output_dir))
        os.makedirs(options.output_dir)
        
    cmake_helper_files_dir = os.path.join(lib_path, "gen_code_config_files")
    codegen = BenchmarkCodeGen(input_dir, options.output_dir, sources_file, cmake_helper_files_dir)
    codegen.generate_benchmarking_code()
    
    print("Done.\nGenerated code can be found here: %s" % options.output_dir)
